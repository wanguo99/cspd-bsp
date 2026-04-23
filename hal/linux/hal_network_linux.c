/************************************************************************
 * HAL层 - Linux网络驱动实现
 ************************************************************************/

#include "hal_network.h"
#include "osapi_log.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/time.h>

typedef struct
{
    int                  sockfd;
    hal_net_proto_t      protocol;
    struct sockaddr_in   addr;
    bool                 connected;
} hal_network_context_t;

int32 HAL_Network_Open(const hal_network_config_t *config, hal_network_handle_t *handle)
{
    if (config == NULL || handle == NULL) {
        return OS_INVALID_POINTER;
    }

    hal_network_context_t *ctx = (hal_network_context_t *)malloc(sizeof(hal_network_context_t));
    if (ctx == NULL) {
        LOG_ERROR("HAL_NET", "Failed to allocate network context");
        return OS_ERROR;
    }

    memset(ctx, 0, sizeof(hal_network_context_t));
    ctx->protocol = config->protocol;

    int sock_type = (config->protocol == HAL_NET_PROTO_TCP) ? SOCK_STREAM : SOCK_DGRAM;
    ctx->sockfd = socket(AF_INET, sock_type, 0);
    if (ctx->sockfd < 0) {
        LOG_ERROR("HAL_NET", "Failed to create socket: %s", strerror(errno));
        free(ctx);
        return OS_ERROR;
    }

    ctx->addr.sin_family = AF_INET;
    ctx->addr.sin_port = htons(config->port);
    if (inet_pton(AF_INET, config->ip_addr, &ctx->addr.sin_addr) <= 0) {
        LOG_ERROR("HAL_NET", "Invalid IP address: %s", config->ip_addr);
        close(ctx->sockfd);
        free(ctx);
        return OS_ERROR;
    }

    /* TCP需要连接 */
    if (config->protocol == HAL_NET_PROTO_TCP) {
        if (connect(ctx->sockfd, (struct sockaddr *)&ctx->addr, sizeof(ctx->addr)) < 0) {
            LOG_ERROR("HAL_NET", "Failed to connect to %s:%d: %s",
                     config->ip_addr, config->port, strerror(errno));
            close(ctx->sockfd);
            free(ctx);
            return OS_ERROR;
        }
        ctx->connected = true;
        LOG_INFO("HAL_NET", "Connected to %s:%d via TCP", config->ip_addr, config->port);
    } else {
        ctx->connected = true;
        LOG_INFO("HAL_NET", "UDP socket ready for %s:%d", config->ip_addr, config->port);
    }

    /* 设置非阻塞模式 */
    int flags = fcntl(ctx->sockfd, F_GETFL, 0);
    if (flags == -1) {
        LOG_ERROR("HAL_NET", "Failed to get socket flags: %s", strerror(errno));
        close(ctx->sockfd);
        free(ctx);
        return OS_ERROR;
    }

    if (fcntl(ctx->sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
        LOG_ERROR("HAL_NET", "Failed to set non-blocking mode: %s", strerror(errno));
        close(ctx->sockfd);
        free(ctx);
        return OS_ERROR;
    }

    *handle = (hal_network_handle_t)ctx;
    return OS_SUCCESS;
}

/*
 * 关闭网络连接
 */
int32 HAL_Network_Close(hal_network_handle_t handle)
{
    if (handle == NULL) {
        return OS_ERR_INVALID_ID;
    }

    hal_network_context_t *ctx = (hal_network_context_t *)handle;

    if (ctx->sockfd >= 0) {
        close(ctx->sockfd);
        ctx->sockfd = -1;
    }

    ctx->connected = false;
    free(ctx);

    LOG_INFO("HAL_NET", "Network connection closed");
    return OS_SUCCESS;
}

/*
 * 发送数据
 */
int32 HAL_Network_Send(hal_network_handle_t handle, const void *buffer, uint32 size, int32 timeout)
{
    if (handle == NULL || buffer == NULL) {
        return OS_ERROR;
    }

    hal_network_context_t *ctx = (hal_network_context_t *)handle;

    if (!ctx->connected) {
        return OS_ERROR;
    }

    /* 设置超时 */
    if (timeout > 0) {
        fd_set write_fds;
        struct timeval tv;

        FD_ZERO(&write_fds);
        FD_SET(ctx->sockfd, &write_fds);

        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;

        int ret = select(ctx->sockfd + 1, NULL, &write_fds, NULL, &tv);
        if (ret == 0) {
            return OS_ERROR_TIMEOUT;
        } else if (ret < 0) {
            return OS_ERROR;
        }
    }

    /* 发送数据 */
    ssize_t sent;
    if (ctx->protocol == HAL_NET_PROTO_TCP) {
        sent = send(ctx->sockfd, buffer, size, 0);
    } else {
        sent = sendto(ctx->sockfd, buffer, size, 0,
                     (struct sockaddr *)&ctx->addr, sizeof(ctx->addr));
    }

    if (sent < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return OS_ERROR_TIMEOUT;
        }
        LOG_ERROR("HAL_NET", "Send failed: %s", strerror(errno));
        ctx->connected = false;
        return OS_ERROR;
    }

    return (int32)sent;
}

/*
 * 接收数据
 */
int32 HAL_Network_Recv(hal_network_handle_t handle, void *buffer, uint32 size, int32 timeout)
{
    if (handle == NULL || buffer == NULL) {
        return OS_ERROR;
    }

    hal_network_context_t *ctx = (hal_network_context_t *)handle;

    if (!ctx->connected) {
        return OS_ERROR;
    }

    /* 设置超时 */
    if (timeout > 0) {
        fd_set read_fds;
        struct timeval tv;

        FD_ZERO(&read_fds);
        FD_SET(ctx->sockfd, &read_fds);

        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;

        int ret = select(ctx->sockfd + 1, &read_fds, NULL, NULL, &tv);
        if (ret == 0) {
            return OS_ERROR_TIMEOUT;
        } else if (ret < 0) {
            return OS_ERROR;
        }
    }

    /* 接收数据 */
    ssize_t received;
    if (ctx->protocol == HAL_NET_PROTO_TCP) {
        received = recv(ctx->sockfd, buffer, size, 0);
    } else {
        socklen_t addr_len = sizeof(ctx->addr);
        received = recvfrom(ctx->sockfd, buffer, size, 0,
                           (struct sockaddr *)&ctx->addr, &addr_len);
    }

    if (received < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return OS_ERROR_TIMEOUT;
        }
        LOG_ERROR("HAL_NET", "Recv failed: %s", strerror(errno));
        ctx->connected = false;
        return OS_ERROR;
    } else if (received == 0 && ctx->protocol == HAL_NET_PROTO_TCP) {
        /* TCP连接关闭 */
        ctx->connected = false;
        return OS_ERROR;
    }

    return (int32)received;
}

/*
 * 检查连接状态
 */
bool HAL_Network_IsConnected(hal_network_handle_t handle)
{
    if (handle == NULL) {
        return false;
    }

    hal_network_context_t *ctx = (hal_network_context_t *)handle;
    return ctx->connected;
}
