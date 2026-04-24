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
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

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
        OS_printf("[PayloadService] 无效的配置参数\n");
        return OS_INVALID_POINTER;
    }

    /* 分配上下文 */
    ctx = (payload_service_context_t *)malloc(sizeof(payload_service_context_t));
    if (!ctx)
    {
        OS_printf("[PayloadService] 内存分配失败\n");
        return OS_ERROR;
    }

    memset(ctx, 0, sizeof(payload_service_context_t));
    memcpy(&ctx->config, config, sizeof(payload_service_config_t));
    ctx->eth_fd = -1;
    ctx->uart_fd = -1;
    ctx->current_channel = PAYLOAD_CHANNEL_ETHERNET;
    ctx->connected = false;
    ctx->state = CONN_STATE_DISCONNECTED;

    /* 创建互斥锁 */
    ret = OS_MutexCreate(&ctx->mutex, "PAYLOAD_MTX", 0);
    if (ret != OS_SUCCESS)
    {
        OS_printf("[PayloadService] 创建互斥锁失败\n");
        free(ctx);
        return ret;
    }

    /* 尝试连接以太网 */
    ret = ethernet_connect(ctx);
    if (ret == OS_SUCCESS)
    {
        OS_printf("[PayloadService] 以太网连接成功\n");
        ctx->connected = true;
        ctx->state = CONN_STATE_CONNECTED;
    }
    else
    {
        OS_printf("[PayloadService] 以太网连接失败，尝试UART\n");

        /* 尝试打开UART */
        ret = uart_open(ctx);
        if (ret == OS_SUCCESS)
        {
            OS_printf("[PayloadService] UART打开成功\n");
            ctx->current_channel = PAYLOAD_CHANNEL_UART;
            ctx->connected = true;
            ctx->state = CONN_STATE_CONNECTED;
        }
        else
        {
            OS_printf("[PayloadService] UART打开失败\n");
            ctx->state = CONN_STATE_ERROR;
            OS_MutexDelete(ctx->mutex);
            free(ctx);
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

    OS_MutexLock(ctx->mutex);

    ethernet_disconnect(ctx);
    uart_close(ctx);

    ctx->connected = false;
    ctx->state = CONN_STATE_DISCONNECTED;

    OS_MutexUnlock(ctx->mutex);
    OS_MutexDelete(ctx->mutex);

    free(ctx);

    OS_printf("[PayloadService] 服务已关闭\n");
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

    OS_MutexLock(ctx->mutex);

    if (!ctx->connected)
    {
        OS_MutexUnlock(ctx->mutex);
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
            OS_printf("[PayloadService] 通道失败次数过多(%u)，尝试切换\n",
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
                    OS_printf("[PayloadService] 已切换到以太网\n");
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
                    OS_printf("[PayloadService] 已切换到UART\n");
                }
            }
        }
    }

    OS_MutexUnlock(ctx->mutex);
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

    OS_MutexLock(ctx->mutex);

    if (!ctx->connected)
    {
        OS_MutexUnlock(ctx->mutex);
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

    OS_MutexUnlock(ctx->mutex);
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
    OS_MutexLock(ctx->mutex);
    connected = ctx->connected && (ctx->state == CONN_STATE_CONNECTED);
    OS_MutexUnlock(ctx->mutex);

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

    OS_MutexLock(ctx->mutex);

    if (ctx->current_channel == channel)
    {
        OS_MutexUnlock(ctx->mutex);
        return OS_SUCCESS;
    }

    OS_printf("[PayloadService] 切换通道: %s -> %s\n",
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

    OS_MutexUnlock(ctx->mutex);
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
    OS_MutexLock(ctx->mutex);
    channel = ctx->current_channel;
    OS_MutexUnlock(ctx->mutex);

    return channel;
}

/*
 * 以太网实现
 */
static int32 ethernet_connect(payload_service_context_t *ctx)
{
    struct sockaddr_in server_addr;
    int32 flags;
    int enable = 1;

    /* 创建socket */
    ctx->eth_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (ctx->eth_fd < 0)
    {
        OS_printf("[PayloadService] 创建socket失败: %s\n", strerror(errno));
        return OS_ERROR;
    }

    /* 设置socket选项 */
    setsockopt(ctx->eth_fd, SOL_SOCKET, SO_KEEPALIVE, &enable, sizeof(enable));
    setsockopt(ctx->eth_fd, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));

    /* 设置非阻塞 */
    flags = fcntl(ctx->eth_fd, F_GETFL, 0);
    fcntl(ctx->eth_fd, F_SETFL, flags | O_NONBLOCK);

    /* 配置服务器地址 */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(ctx->config.ethernet.port);

    if (inet_pton(AF_INET, ctx->config.ethernet.ip_addr, &server_addr.sin_addr) <= 0)
    {
        OS_printf("[PayloadService] 无效的IP地址: %s\n", ctx->config.ethernet.ip_addr);
        close(ctx->eth_fd);
        ctx->eth_fd = -1;
        return OS_ERROR;
    }

    /* 连接 */
    if (connect(ctx->eth_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        if (errno != EINPROGRESS)
        {
            OS_printf("[PayloadService] 连接失败: %s\n", strerror(errno));
            close(ctx->eth_fd);
            ctx->eth_fd = -1;
            return OS_ERROR;
        }

        /* 等待连接完成 */
        fd_set writefds;
        struct timeval tv;
        FD_ZERO(&writefds);
        FD_SET(ctx->eth_fd, &writefds);
        tv.tv_sec = 5;  /* 5秒超时 */
        tv.tv_usec = 0;

        int ret = select(ctx->eth_fd + 1, NULL, &writefds, NULL, &tv);
        if (ret <= 0)
        {
            OS_printf("[PayloadService] 连接超时\n");
            close(ctx->eth_fd);
            ctx->eth_fd = -1;
            return OS_ERROR;
        }

        /* 检查连接是否成功 */
        int error = 0;
        socklen_t len = sizeof(error);
        getsockopt(ctx->eth_fd, SOL_SOCKET, SO_ERROR, &error, &len);
        if (error != 0)
        {
            OS_printf("[PayloadService] 连接失败: %s\n", strerror(error));
            close(ctx->eth_fd);
            ctx->eth_fd = -1;
            return OS_ERROR;
        }
    }

    /* 恢复阻塞模式 */
    fcntl(ctx->eth_fd, F_SETFL, flags);

    return OS_SUCCESS;
}

static int32 ethernet_disconnect(payload_service_context_t *ctx)
{
    if (ctx->eth_fd >= 0)
    {
        shutdown(ctx->eth_fd, SHUT_RDWR);
        close(ctx->eth_fd);
        ctx->eth_fd = -1;
    }
    return OS_SUCCESS;
}

static int32 ethernet_send(payload_service_context_t *ctx, const void *data, uint32 len)
{
    ssize_t ret;

    if (ctx->eth_fd < 0)
    {
        return OS_ERROR;
    }

    ret = send(ctx->eth_fd, data, len, MSG_NOSIGNAL);
    if (ret < 0)
    {
        OS_printf("[PayloadService] 以太网发送失败: %s\n", strerror(errno));
        return OS_ERROR;
    }

    return (int32)ret;
}

static int32 ethernet_recv(payload_service_context_t *ctx, void *buf, uint32 buf_size, uint32 timeout_ms)
{
    ssize_t ret;
    fd_set readfds;
    struct timeval tv;

    if (ctx->eth_fd < 0)
    {
        return OS_ERROR;
    }

    /* 设置超时 */
    FD_ZERO(&readfds);
    FD_SET(ctx->eth_fd, &readfds);
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    ret = select(ctx->eth_fd + 1, &readfds, NULL, NULL, &tv);
    if (ret == 0)
    {
        return OS_ERROR_TIMEOUT;
    }
    else if (ret < 0)
    {
        OS_printf("[PayloadService] select失败: %s\n", strerror(errno));
        return OS_ERROR;
    }

    ret = recv(ctx->eth_fd, buf, buf_size, 0);
    if (ret < 0)
    {
        OS_printf("[PayloadService] 以太网接收失败: %s\n", strerror(errno));
        return OS_ERROR;
    }
    else if (ret == 0)
    {
        OS_printf("[PayloadService] 连接已关闭\n");
        return OS_ERROR;
    }

    return (int32)ret;
}

/*
 * UART实现
 */
static int32 uart_open(payload_service_context_t *ctx)
{
    struct termios tty;

    /* 打开串口 */
    ctx->uart_fd = open(ctx->config.uart.device, O_RDWR | O_NOCTTY);
    if (ctx->uart_fd < 0)
    {
        OS_printf("[PayloadService] 打开UART失败: %s (%s)\n",
                 strerror(errno), ctx->config.uart.device);
        return OS_ERROR;
    }

    /* 配置串口 */
    memset(&tty, 0, sizeof(tty));
    if (tcgetattr(ctx->uart_fd, &tty) != 0)
    {
        OS_printf("[PayloadService] 获取UART属性失败: %s\n", strerror(errno));
        close(ctx->uart_fd);
        ctx->uart_fd = -1;
        return OS_ERROR;
    }

    /* 设置波特率 */
    speed_t speed = B115200;
    switch (ctx->config.uart.baudrate)
    {
        case 9600:   speed = B9600; break;
        case 19200:  speed = B19200; break;
        case 38400:  speed = B38400; break;
        case 57600:  speed = B57600; break;
        case 115200: speed = B115200; break;
        default:
            OS_printf("[PayloadService] 不支持的波特率: %u\n", ctx->config.uart.baudrate);
            speed = B115200;
    }

    cfsetospeed(&tty, speed);
    cfsetispeed(&tty, speed);

    /* 8N1 */
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
#ifdef CRTSCTS
    tty.c_cflag &= ~CRTSCTS;
#endif
    tty.c_cflag |= CREAD | CLOCAL;

    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO;
    tty.c_lflag &= ~ISIG;

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

    tty.c_oflag &= ~OPOST;
    tty.c_oflag &= ~ONLCR;

    tty.c_cc[VTIME] = 10;
    tty.c_cc[VMIN] = 0;

    if (tcsetattr(ctx->uart_fd, TCSANOW, &tty) != 0)
    {
        OS_printf("[PayloadService] 设置UART属性失败: %s\n", strerror(errno));
        close(ctx->uart_fd);
        ctx->uart_fd = -1;
        return OS_ERROR;
    }

    /* 清空缓冲区 */
    tcflush(ctx->uart_fd, TCIOFLUSH);

    return OS_SUCCESS;
}

static int32 uart_close(payload_service_context_t *ctx)
{
    if (ctx->uart_fd >= 0)
    {
        close(ctx->uart_fd);
        ctx->uart_fd = -1;
    }
    return OS_SUCCESS;
}

static int32 uart_send(payload_service_context_t *ctx, const void *data, uint32 len)
{
    ssize_t ret;

    if (ctx->uart_fd < 0)
    {
        return OS_ERROR;
    }

    ret = write(ctx->uart_fd, data, len);
    if (ret < 0)
    {
        OS_printf("[PayloadService] UART发送失败: %s\n", strerror(errno));
        return OS_ERROR;
    }

    /* 等待数据发送完成 */
    tcdrain(ctx->uart_fd);

    return (int32)ret;
}

static int32 uart_recv(payload_service_context_t *ctx, void *buf, uint32 buf_size, uint32 timeout_ms)
{
    ssize_t ret;
    fd_set readfds;
    struct timeval tv;

    if (ctx->uart_fd < 0)
    {
        return OS_ERROR;
    }

    /* 设置超时 */
    FD_ZERO(&readfds);
    FD_SET(ctx->uart_fd, &readfds);
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    ret = select(ctx->uart_fd + 1, &readfds, NULL, NULL, &tv);
    if (ret == 0)
    {
        return OS_ERROR_TIMEOUT;
    }
    else if (ret < 0)
    {
        OS_printf("[PayloadService] select失败: %s\n", strerror(errno));
        return OS_ERROR;
    }

    ret = read(ctx->uart_fd, buf, buf_size);
    if (ret < 0)
    {
        OS_printf("[PayloadService] UART接收失败: %s\n", strerror(errno));
        return OS_ERROR;
    }

    return (int32)ret;
}
