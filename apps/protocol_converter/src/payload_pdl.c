/************************************************************************
 * 载荷服务实现（优化版）
 *
 * 优化内容：
 * 1. 添加连接状态管理和心跳检测
 * 2. 改进错误处理和重连机制
 * 3. 添加统计信息和日志
 * 4. 修复资源泄漏问题
 * 5. 添加线程安全保护
 ************************************************************************/

#include "payload_pdl.h"
#include "osal.h"

/*
 * 连接状态
 */
typedef enum
{
    CONN_STATE_DISCONNECTED = 0,
    CONN_STATE_CONNECTING,
    CONN_STATE_CONNECTED,
    CONN_STATE_ERROR
} conn_state_t;

/*
 * 载荷服务上下文
 */
typedef struct
{
    payload_service_config_t config;           /* 配置 */
    payload_channel_t        current_channel;  /* 当前通道 */
    conn_state_t             state;            /* 连接状态 */
    int32                    eth_fd;           /* 以太网socket */
    int32                    uart_fd;          /* UART文件描述符 */
    bool                     connected;        /* 连接状态 */
    uint32                   fail_count;       /* 失败计数 */
    uint32                   reconnect_count;  /* 重连计数 */
    osal_id_t                mutex;            /* 互斥锁 */

    /* 统计信息 */
    uint32                   tx_bytes;
    uint32                   rx_bytes;
    uint32                   tx_errors;
    uint32                   rx_errors;
} payload_service_context_t;

/*
 * 内部函数声明
 */
static int32 ethernet_connect(payload_service_context_t *ctx);
static int32 ethernet_disconnect(payload_service_context_t *ctx);
static int32 ethernet_send(payload_service_context_t *ctx, const void *data, uint32 len);
static int32 ethernet_recv(payload_service_context_t *ctx, void *buf, uint32 buf_size, uint32 timeout_ms);

static int32 uart_open(payload_service_context_t *ctx);
static int32 uart_close(payload_service_context_t *ctx);
static int32 uart_send(payload_service_context_t *ctx, const void *data, uint32 len);
static int32 uart_recv(payload_service_context_t *ctx, void *buf, uint32 buf_size, uint32 timeout_ms);

/*
 * 公共接口实现
 */
int32 PayloadService_Init(const payload_service_config_t *config,
                          payload_service_handle_t *handle)
{
    payload_service_context_t *ctx;
    int32 ret;

    if (!config || !handle)
    {
        return OS_INVALID_POINTER;
    }

    /* 参数验证 */
    if (config->ethernet.ip_addr == NULL || config->uart.device == NULL)
    {
        OSAL_Printf("[PayloadService] 无效的配置参数\n");
        return OS_INVALID_POINTER;
    }

    /* 分配上下文 */
    ctx = (payload_service_context_t *)OSAL_Malloc(sizeof(payload_service_context_t));
    if (!ctx)
    {
        OSAL_Printf("[PayloadService] 内存分配失败\n");
        return OS_ERROR;
    }

    OSAL_Memset(ctx, 0, sizeof(payload_service_context_t));
    OSAL_Memcpy(&ctx->config, config, sizeof(payload_service_config_t));
    ctx->eth_fd = -1;
    ctx->uart_fd = -1;
    ctx->current_channel = PAYLOAD_CHANNEL_ETHERNET;
    ctx->connected = false;
    ctx->state = CONN_STATE_DISCONNECTED;

    /* 创建互斥锁 */
    ret = OSAL_MutexCreate(&ctx->mutex, "PAYLOAD_MTX", 0);
    if (ret != OS_SUCCESS)
    {
        OSAL_Printf("[PayloadService] 创建互斥锁失败\n");
        OSAL_Free(ctx);
        return ret;
    }

    /* 尝试连接以太网 */
    ret = ethernet_connect(ctx);
    if (ret == OS_SUCCESS)
    {
        OSAL_Printf("[PayloadService] 以太网连接成功\n");
        ctx->connected = true;
        ctx->state = CONN_STATE_CONNECTED;
    }
    else
    {
        OSAL_Printf("[PayloadService] 以太网连接失败，尝试UART\n");

        /* 尝试打开UART */
        ret = uart_open(ctx);
        if (ret == OS_SUCCESS)
        {
            OSAL_Printf("[PayloadService] UART打开成功\n");
            ctx->current_channel = PAYLOAD_CHANNEL_UART;
            ctx->connected = true;
            ctx->state = CONN_STATE_CONNECTED;
        }
        else
        {
            OSAL_Printf("[PayloadService] UART打开失败\n");
            ctx->state = CONN_STATE_ERROR;
            OSAL_MutexDelete(ctx->mutex);
            OSAL_Free(ctx);
            return OS_ERROR;
        }
    }

    *handle = (payload_service_handle_t)ctx;
    return OS_SUCCESS;
}

int32 PayloadService_Deinit(payload_service_handle_t handle)
{
    payload_service_context_t *ctx = (payload_service_context_t *)handle;

    if (!ctx)
    {
        return OS_INVALID_POINTER;
    }

    OSAL_MutexLock(ctx->mutex);

    ethernet_disconnect(ctx);
    uart_close(ctx);

    ctx->connected = false;
    ctx->state = CONN_STATE_DISCONNECTED;

    OSAL_MutexUnlock(ctx->mutex);
    OSAL_MutexDelete(ctx->mutex);

    OSAL_Free(ctx);

    OSAL_Printf("[PayloadService] 服务已关闭\n");
    return OS_SUCCESS;
}

int32 PayloadService_Send(payload_service_handle_t handle,
                          const void *data,
                          uint32 len)
{
    payload_service_context_t *ctx = (payload_service_context_t *)handle;
    int32 ret;

    if (!ctx || !data || len == 0)
    {
        return OS_INVALID_POINTER;
    }

    OSAL_MutexLock(ctx->mutex);

    if (!ctx->connected)
    {
        OSAL_MutexUnlock(ctx->mutex);
        return OS_ERROR;
    }

    if (ctx->current_channel == PAYLOAD_CHANNEL_ETHERNET)
    {
        ret = ethernet_send(ctx, data, len);
    }
    else
    {
        ret = uart_send(ctx, data, len);
    }

    /* 更新统计 */
    if (ret > 0)
    {
        ctx->tx_bytes += ret;
        ctx->fail_count = 0;  /* 重置失败计数 */
    }
    else
    {
        ctx->tx_errors++;
        ctx->fail_count++;
    }

    /* 自动切换逻辑 */
    if (ret < 0 && ctx->config.auto_switch)
    {
        if (ctx->fail_count >= ctx->config.retry_count)
        {
            OSAL_Printf("[PayloadService] 通道失败次数过多(%u)，尝试切换\n",
                     ctx->fail_count);

            payload_channel_t new_channel = (ctx->current_channel == PAYLOAD_CHANNEL_ETHERNET) ?
                                            PAYLOAD_CHANNEL_UART : PAYLOAD_CHANNEL_ETHERNET;

            /* 在锁内切换通道 */
            if (new_channel == PAYLOAD_CHANNEL_ETHERNET)
            {
                uart_close(ctx);
                if (ethernet_connect(ctx) == OS_SUCCESS)
                {
                    ctx->current_channel = PAYLOAD_CHANNEL_ETHERNET;
                    ctx->connected = true;
                    ctx->fail_count = 0;
                    OSAL_Printf("[PayloadService] 已切换到以太网\n");
                }
            }
            else
            {
                ethernet_disconnect(ctx);
                if (uart_open(ctx) == OS_SUCCESS)
                {
                    ctx->current_channel = PAYLOAD_CHANNEL_UART;
                    ctx->connected = true;
                    ctx->fail_count = 0;
                    OSAL_Printf("[PayloadService] 已切换到UART\n");
                }
            }
        }
    }

    OSAL_MutexUnlock(ctx->mutex);
    return ret;
}

int32 PayloadService_Recv(payload_service_handle_t handle,
                          void *buf,
                          uint32 buf_size,
                          uint32 timeout_ms)
{
    payload_service_context_t *ctx = (payload_service_context_t *)handle;
    int32 ret;

    if (!ctx || !buf || buf_size == 0)
    {
        return OS_INVALID_POINTER;
    }

    OSAL_MutexLock(ctx->mutex);

    if (!ctx->connected)
    {
        OSAL_MutexUnlock(ctx->mutex);
        return OS_ERROR;
    }

    if (ctx->current_channel == PAYLOAD_CHANNEL_ETHERNET)
    {
        ret = ethernet_recv(ctx, buf, buf_size, timeout_ms);
    }
    else
    {
        ret = uart_recv(ctx, buf, buf_size, timeout_ms);
    }

    /* 更新统计 */
    if (ret > 0)
    {
        ctx->rx_bytes += ret;
    }
    else if (ret < 0 && ret != OS_ERROR_TIMEOUT)
    {
        ctx->rx_errors++;
    }

    OSAL_MutexUnlock(ctx->mutex);
    return ret;
}

bool PayloadService_IsConnected(payload_service_handle_t handle)
{
    payload_service_context_t *ctx = (payload_service_context_t *)handle;

    if (!ctx)
    {
        return false;
    }

    bool connected;
    OSAL_MutexLock(ctx->mutex);
    connected = ctx->connected && (ctx->state == CONN_STATE_CONNECTED);
    OSAL_MutexUnlock(ctx->mutex);

    return connected;
}

int32 PayloadService_SwitchChannel(payload_service_handle_t handle,
                                   payload_channel_t channel)
{
    payload_service_context_t *ctx = (payload_service_context_t *)handle;
    int32 ret;

    if (!ctx)
    {
        return OS_INVALID_POINTER;
    }

    OSAL_MutexLock(ctx->mutex);

    if (ctx->current_channel == channel)
    {
        OSAL_MutexUnlock(ctx->mutex);
        return OS_SUCCESS;
    }

    OSAL_Printf("[PayloadService] 切换通道: %s -> %s\n",
             ctx->current_channel == PAYLOAD_CHANNEL_ETHERNET ? "以太网" : "UART",
             channel == PAYLOAD_CHANNEL_ETHERNET ? "以太网" : "UART");

    if (channel == PAYLOAD_CHANNEL_ETHERNET)
    {
        uart_close(ctx);
        ret = ethernet_connect(ctx);
        if (ret == OS_SUCCESS)
        {
            ctx->current_channel = PAYLOAD_CHANNEL_ETHERNET;
            ctx->connected = true;
            ctx->state = CONN_STATE_CONNECTED;
            ctx->reconnect_count++;
        }
        else
        {
            ctx->connected = false;
            ctx->state = CONN_STATE_ERROR;
        }
    }
    else
    {
        ethernet_disconnect(ctx);
        ret = uart_open(ctx);
        if (ret == OS_SUCCESS)
        {
            ctx->current_channel = PAYLOAD_CHANNEL_UART;
            ctx->connected = true;
            ctx->state = CONN_STATE_CONNECTED;
            ctx->reconnect_count++;
        }
        else
        {
            ctx->connected = false;
            ctx->state = CONN_STATE_ERROR;
        }
    }

    OSAL_MutexUnlock(ctx->mutex);
    return ret;
}

payload_channel_t PayloadService_GetChannel(payload_service_handle_t handle)
{
    payload_service_context_t *ctx = (payload_service_context_t *)handle;

    if (!ctx)
    {
        return PAYLOAD_CHANNEL_ETHERNET;
    }

    payload_channel_t channel;
    OSAL_MutexLock(ctx->mutex);
    channel = ctx->current_channel;
    OSAL_MutexUnlock(ctx->mutex);

    return channel;
}

/*
 * 以太网实现
 */
static int32 ethernet_connect(payload_service_context_t *ctx)
{
    osal_sockaddr_in_t server_addr;
    int32 flags;
    int32 enable = 1;

    /* 创建socket */
    ctx->eth_fd = OSAL_socket(OSAL_AF_INET, OSAL_SOCK_STREAM, 0);
    if (ctx->eth_fd < 0)
    {
        OSAL_Printf("[PayloadService] 创建socket失败: %s\n", OSAL_StrError(OSAL_GetErrno()));
        return OS_ERROR;
    }

    /* 设置socket选项 */
    OSAL_setsockopt(ctx->eth_fd, OSAL_SOL_SOCKET, OSAL_SO_KEEPALIVE, &enable, sizeof(enable));
    OSAL_setsockopt(ctx->eth_fd, OSAL_IPPROTO_TCP, OSAL_TCP_NODELAY, &enable, sizeof(enable));

    /* 设置非阻塞 */
    flags = OSAL_fcntl(ctx->eth_fd, OSAL_F_GETFL, 0);
    OSAL_fcntl(ctx->eth_fd, OSAL_F_SETFL, flags | OSAL_O_NONBLOCK);

    /* 配置服务器地址 */
    OSAL_Memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = OSAL_AF_INET;
    server_addr.sin_port = OSAL_htons(ctx->config.ethernet.port);

    if (OSAL_inet_pton(OSAL_AF_INET, ctx->config.ethernet.ip_addr, &server_addr.sin_addr) <= 0)
    {
        OSAL_Printf("[PayloadService] 无效的IP地址: %s\n", ctx->config.ethernet.ip_addr);
        OSAL_close(ctx->eth_fd);
        ctx->eth_fd = -1;
        return OS_ERROR;
    }

    /* 连接 */
    if (OSAL_connect(ctx->eth_fd, (osal_sockaddr_t *)&server_addr, sizeof(server_addr)) < 0)
    {
        int32 err = OSAL_GetErrno();
        if (err != OSAL_EINPROGRESS)
        {
            OSAL_Printf("[PayloadService] 连接失败: %s\n", OSAL_StrError(err));
            OSAL_close(ctx->eth_fd);
            ctx->eth_fd = -1;
            return OS_ERROR;
        }

        /* 等待连接完成 */
        osal_fd_set_t writefds;
        osal_timeval_t tv;
        OSAL_FD_ZERO(&writefds);
        OSAL_FD_SET(ctx->eth_fd, &writefds);
        tv.tv_sec = 5;  /* 5秒超时 */
        tv.tv_usec = 0;

        int ret = OSAL_select(ctx->eth_fd + 1, NULL, &writefds, NULL, &tv);
        if (ret <= 0)
        {
            OSAL_Printf("[PayloadService] 连接超时\n");
            OSAL_close(ctx->eth_fd);
            ctx->eth_fd = -1;
            return OS_ERROR;
        }

        /* 检查连接是否成功 */
        int error = 0;
        osal_size_t len = sizeof(error);
        OSAL_getsockopt(ctx->eth_fd, OSAL_SOL_SOCKET, OSAL_SO_ERROR, &error, &len);
        if (error != 0)
        {
            OSAL_Printf("[PayloadService] 连接失败: %s\n", OSAL_StrError(error));
            OSAL_close(ctx->eth_fd);
            ctx->eth_fd = -1;
            return OS_ERROR;
        }
    }

    /* 恢复阻塞模式 */
    OSAL_fcntl(ctx->eth_fd, OSAL_F_SETFL, flags);

    return OS_SUCCESS;
}

static int32 ethernet_disconnect(payload_service_context_t *ctx)
{
    if (ctx->eth_fd >= 0)
    {
        OSAL_shutdown(ctx->eth_fd, OSAL_SHUT_RDWR);
        OSAL_close(ctx->eth_fd);
        ctx->eth_fd = -1;
    }
    return OS_SUCCESS;
}

static int32 ethernet_send(payload_service_context_t *ctx, const void *data, uint32 len)
{
    osal_ssize_t ret;

    if (ctx->eth_fd < 0)
    {
        return OS_ERROR;
    }

    ret = OSAL_send(ctx->eth_fd, data, len, OSAL_MSG_NOSIGNAL);
    if (ret < 0)
    {
        OSAL_Printf("[PayloadService] 以太网发送失败: %s\n", OSAL_StrError(OSAL_GetErrno()));
        return OS_ERROR;
    }

    return (int32)ret;
}

static int32 ethernet_recv(payload_service_context_t *ctx, void *buf, uint32 buf_size, uint32 timeout_ms)
{
    osal_ssize_t ret;
    osal_fd_set_t readfds;
    osal_timeval_t tv;

    if (ctx->eth_fd < 0)
    {
        return OS_ERROR;
    }

    /* 设置超时 */
    OSAL_FD_ZERO(&readfds);
    OSAL_FD_SET(ctx->eth_fd, &readfds);
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    ret = OSAL_select(ctx->eth_fd + 1, &readfds, NULL, NULL, &tv);
    if (ret == 0)
    {
        return OS_ERROR_TIMEOUT;
    }
    else if (ret < 0)
    {
        OSAL_Printf("[PayloadService] select失败: %s\n", OSAL_StrError(OSAL_GetErrno()));
        return OS_ERROR;
    }

    ret = OSAL_recv(ctx->eth_fd, buf, buf_size, 0);
    if (ret < 0)
    {
        OSAL_Printf("[PayloadService] 以太网接收失败: %s\n", OSAL_StrError(OSAL_GetErrno()));
        return OS_ERROR;
    }
    else if (ret == 0)
    {
        OSAL_Printf("[PayloadService] 连接已关闭\n");
        return OS_ERROR;
    }

    return (int32)ret;
}

/*
 * UART实现
 */
static int32 uart_open(payload_service_context_t *ctx)
{
    osal_termios_t tty;

    /* 打开串口 */
    ctx->uart_fd = OSAL_open(ctx->config.uart.device, OSAL_O_RDWR | OSAL_O_NOCTTY, 0);
    if (ctx->uart_fd < 0)
    {
        OSAL_Printf("[PayloadService] 打开UART失败: %s (%s)\n",
                 OSAL_StrError(OSAL_GetErrno()), ctx->config.uart.device);
        return OS_ERROR;
    }

    /* 配置串口 */
    OSAL_Memset(&tty, 0, sizeof(tty));
    if (OSAL_tcgetattr(ctx->uart_fd, &tty) != 0)
    {
        OSAL_Printf("[PayloadService] 获取UART属性失败: %s\n", OSAL_StrError(OSAL_GetErrno()));
        OSAL_close(ctx->uart_fd);
        ctx->uart_fd = -1;
        return OS_ERROR;
    }

    /* 设置波特率 */
    uint32 speed = OSAL_B115200;
    switch (ctx->config.uart.baudrate)
    {
        case 9600:   speed = OSAL_B9600; break;
        case 19200:  speed = OSAL_B19200; break;
        case 38400:  speed = OSAL_B38400; break;
        case 57600:  speed = OSAL_B57600; break;
        case 115200: speed = OSAL_B115200; break;
        default:
            OSAL_Printf("[PayloadService] 不支持的波特率: %u\n", ctx->config.uart.baudrate);
            speed = OSAL_B115200;
    }

    OSAL_cfsetospeed(&tty, speed);
    OSAL_cfsetispeed(&tty, speed);

    /* 8N1 */
    tty.c_cflag &= ~OSAL_PARENB;
    tty.c_cflag &= ~OSAL_CSTOPB;
    tty.c_cflag &= ~OSAL_CSIZE;
    tty.c_cflag |= OSAL_CS8;
    tty.c_cflag &= ~OSAL_CRTSCTS;
    tty.c_cflag |= OSAL_CREAD | OSAL_CLOCAL;

    tty.c_lflag &= ~OSAL_ICANON;
    tty.c_lflag &= ~OSAL_ECHO;
    tty.c_lflag &= ~OSAL_ISIG;

    tty.c_iflag &= ~(OSAL_IXON | OSAL_IXOFF | OSAL_IXANY);
    tty.c_iflag &= ~(OSAL_IGNBRK | OSAL_BRKINT | OSAL_PARMRK | OSAL_ISTRIP | OSAL_INLCR | OSAL_IGNCR | OSAL_ICRNL);

    tty.c_oflag &= ~OSAL_OPOST;
    tty.c_oflag &= ~OSAL_ONLCR;

    tty.c_cc[OSAL_VTIME] = 10;
    tty.c_cc[OSAL_VMIN] = 0;

    if (OSAL_tcsetattr(ctx->uart_fd, OSAL_TCSANOW, &tty) != 0)
    {
        OSAL_Printf("[PayloadService] 设置UART属性失败: %s\n", OSAL_StrError(OSAL_GetErrno()));
        OSAL_close(ctx->uart_fd);
        ctx->uart_fd = -1;
        return OS_ERROR;
    }

    /* 清空缓冲区 */
    OSAL_tcflush(ctx->uart_fd, OSAL_TCIOFLUSH);

    return OS_SUCCESS;
}

static int32 uart_close(payload_service_context_t *ctx)
{
    if (ctx->uart_fd >= 0)
    {
        OSAL_close(ctx->uart_fd);
        ctx->uart_fd = -1;
    }
    return OS_SUCCESS;
}

static int32 uart_send(payload_service_context_t *ctx, const void *data, uint32 len)
{
    osal_ssize_t ret;

    if (ctx->uart_fd < 0)
    {
        return OS_ERROR;
    }

    ret = OSAL_write(ctx->uart_fd, data, len);
    if (ret < 0)
    {
        OSAL_Printf("[PayloadService] UART发送失败: %s\n", OSAL_StrError(OSAL_GetErrno()));
        return OS_ERROR;
    }

    /* 等待数据发送完成 */
    OSAL_tcdrain(ctx->uart_fd);

    return (int32)ret;
}

static int32 uart_recv(payload_service_context_t *ctx, void *buf, uint32 buf_size, uint32 timeout_ms)
{
    osal_ssize_t ret;
    osal_fd_set_t readfds;
    osal_timeval_t tv;

    if (ctx->uart_fd < 0)
    {
        return OS_ERROR;
    }

    /* 设置超时 */
    OSAL_FD_ZERO(&readfds);
    OSAL_FD_SET(ctx->uart_fd, &readfds);
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    ret = OSAL_select(ctx->uart_fd + 1, &readfds, NULL, NULL, &tv);
    if (ret == 0)
    {
        return OS_ERROR_TIMEOUT;
    }
    else if (ret < 0)
    {
        OSAL_Printf("[PayloadService] select失败: %s\n", OSAL_StrError(OSAL_GetErrno()));
        return OS_ERROR;
    }

    ret = OSAL_read(ctx->uart_fd, buf, buf_size);
    if (ret < 0)
    {
        OSAL_Printf("[PayloadService] UART接收失败: %s\n", OSAL_StrError(OSAL_GetErrno()));
        return OS_ERROR;
    }

    return (int32)ret;
}
