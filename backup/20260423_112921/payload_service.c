/************************************************************************
 * 载荷服务实现
 *
 * 提供以太网和UART双通道通信能力，支持自动切换
 ************************************************************************/

#include "payload_service.h"
#include "osal.h"
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

/*
 * ============================================================================
 * 内部结构定义
 * ============================================================================
 */

/* 载荷服务上下文 */
typedef struct
{
    payload_service_config_t config;           /* 配置 */
    payload_channel_t        current_channel;  /* 当前通道 */
    int32                    eth_fd;           /* 以太网socket */
    int32                    uart_fd;          /* UART文件描述符 */
    bool                     connected;        /* 连接状态 */
    uint32                   fail_count;       /* 失败计数 */
} payload_service_context_t;

/*
 * ============================================================================
 * 内部函数声明
 * ============================================================================
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
 * ============================================================================
 * 公共接口实现
 * ============================================================================
 */

int32 PayloadService_Init(const payload_service_config_t *config,
                          payload_service_handle_t *handle)
{
    payload_service_context_t *ctx;
    int32 ret;

    if (!config || !handle)
    {
        return OS_ERROR;
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

    /* 尝试连接以太网 */
    ret = ethernet_connect(ctx);
    if (ret == OS_SUCCESS)
    {
        OS_printf("[PayloadService] 以太网连接成功\n");
        ctx->connected = true;
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
        }
        else
        {
            OS_printf("[PayloadService] UART打开失败\n");
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
        return OS_ERROR;
    }

    ethernet_disconnect(ctx);
    uart_close(ctx);
    free(ctx);

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

    /* 自动切换逻辑 */
    if (ret < 0 && ctx->config.auto_switch)
    {
        ctx->fail_count++;

        if (ctx->fail_count >= ctx->config.retry_count)
        {
            OS_printf("[PayloadService] 通道失败，尝试切换\n");

            if (ctx->current_channel == PAYLOAD_CHANNEL_ETHERNET)
            {
                PayloadService_SwitchChannel(handle, PAYLOAD_CHANNEL_UART);
            }
            else
            {
                PayloadService_SwitchChannel(handle, PAYLOAD_CHANNEL_ETHERNET);
            }

            ctx->fail_count = 0;
        }
    }
    else if (ret >= 0)
    {
        ctx->fail_count = 0;
    }

    return ret;
}

int32 PayloadService_Recv(payload_service_handle_t handle,
                          void *buf,
                          uint32 buf_size,
                          uint32 timeout_ms)
{
    payload_service_context_t *ctx = (payload_service_context_t *)handle;

    if (!ctx || !buf || buf_size == 0)
    {
        return OS_ERROR;
    }

    if (ctx->current_channel == PAYLOAD_CHANNEL_ETHERNET)
    {
        return ethernet_recv(ctx, buf, buf_size, timeout_ms);
    }
    else
    {
        return uart_recv(ctx, buf, buf_size, timeout_ms);
    }
}

bool PayloadService_IsConnected(payload_service_handle_t handle)
{
    payload_service_context_t *ctx = (payload_service_context_t *)handle;

    if (!ctx)
    {
        return false;
    }

    return ctx->connected;
}

int32 PayloadService_SwitchChannel(payload_service_handle_t handle,
                                   payload_channel_t channel)
{
    payload_service_context_t *ctx = (payload_service_context_t *)handle;
    int32 ret;

    if (!ctx)
    {
        return OS_ERROR;
    }

    if (ctx->current_channel == channel)
    {
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
            return OS_SUCCESS;
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
            return OS_SUCCESS;
        }
    }

    ctx->connected = false;
    return OS_ERROR;
}

payload_channel_t PayloadService_GetChannel(payload_service_handle_t handle)
{
    payload_service_context_t *ctx = (payload_service_context_t *)handle;

    if (!ctx)
    {
        return PAYLOAD_CHANNEL_ETHERNET;
    }

    return ctx->current_channel;
}

/*
 * ============================================================================
 * 以太网实现
 * ============================================================================
 */

static int32 ethernet_connect(payload_service_context_t *ctx)
{
    struct sockaddr_in server_addr;
    int32 flags;

    /* 创建socket */
    ctx->eth_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (ctx->eth_fd < 0)
    {
        OS_printf("[PayloadService] 创建socket失败: %s\n", strerror(errno));
        return OS_ERROR;
    }

    /* 设置非阻塞 */
    flags = fcntl(ctx->eth_fd, F_GETFL, 0);
    fcntl(ctx->eth_fd, F_SETFL, flags | O_NONBLOCK);

    /* 配置服务器地址 */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(ctx->config.ethernet.port);
    inet_pton(AF_INET, ctx->config.ethernet.ip_addr, &server_addr.sin_addr);

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
    }

    /* 恢复阻塞模式 */
    fcntl(ctx->eth_fd, F_SETFL, flags);

    return OS_SUCCESS;
}

static int32 ethernet_disconnect(payload_service_context_t *ctx)
{
    if (ctx->eth_fd >= 0)
    {
        close(ctx->eth_fd);
        ctx->eth_fd = -1;
    }
    return OS_SUCCESS;
}

static int32 ethernet_send(payload_service_context_t *ctx, const void *data, uint32 len)
{
    int32 ret;

    if (ctx->eth_fd < 0)
    {
        return OS_ERROR;
    }

    ret = send(ctx->eth_fd, data, len, 0);
    if (ret < 0)
    {
        OS_printf("[PayloadService] 以太网发送失败: %s\n", strerror(errno));
        return OS_ERROR;
    }

    return ret;
}

static int32 ethernet_recv(payload_service_context_t *ctx, void *buf, uint32 buf_size, uint32 timeout_ms)
{
    int32 ret;
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

    return ret;
}

/*
 * ============================================================================
 * UART实现
 * ============================================================================
 */

static int32 uart_open(payload_service_context_t *ctx)
{
    struct termios tty;

    /* 打开串口 */
    ctx->uart_fd = open(ctx->config.uart.device, O_RDWR | O_NOCTTY);
    if (ctx->uart_fd < 0)
    {
        OS_printf("[PayloadService] 打开UART失败: %s\n", strerror(errno));
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
    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);

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
    int32 ret;

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

    return ret;
}

static int32 uart_recv(payload_service_context_t *ctx, void *buf, uint32 buf_size, uint32 timeout_ms)
{
    int32 ret;
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

    return ret;
}
