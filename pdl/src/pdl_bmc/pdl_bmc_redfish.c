/************************************************************************
 * BMC Redfish通信实现
 *
 * 职责：
 * - 封装以太网和串口通信
 * - 实现IPMI over LAN和IPMI over Serial
 ************************************************************************/

#include "pdl_bmc_internal.h"
#include "osal.h"
#include "hal_serial.h"
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*
 * 网络通信上下文
 */
typedef struct
{
    int32 sockfd;
    uint32 timeout_ms;
} bmc_redfish_context_t;

/*
 * 串口通信上下文
 */
typedef struct
{
    hal_serial_handle_t serial_handle;
    uint32 timeout_ms;
} bmc_serial_context_t;

/**
 * @brief 初始化网络通信
 */
int32 bmc_redfish_init(const char *ip_addr, uint16 port, uint32 timeout_ms, void **handle)
{
    if (ip_addr == NULL || handle == NULL)
    {
        return OS_ERROR;
    }

    bmc_redfish_context_t *ctx = (bmc_redfish_context_t *)OSAL_Malloc(sizeof(bmc_redfish_context_t));
    if (ctx == NULL)
    {
        return OS_ERROR;
    }

    OSAL_Memset(ctx, 0, sizeof(bmc_redfish_context_t));
    ctx->timeout_ms = timeout_ms;

    /* 创建TCP Socket */
    ctx->sockfd = OSAL_socket(OSAL_AF_INET, OSAL_SOCK_STREAM, 0);
    if (ctx->sockfd < 0)
    {
        OSAL_Free(ctx);
        return OS_ERROR;
    }

    /* 设置超时 */
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    OSAL_setsockopt(ctx->sockfd, OSAL_SOL_SOCKET, OSAL_SO_RCVTIMEO, &tv, sizeof(tv));
    OSAL_setsockopt(ctx->sockfd, OSAL_SOL_SOCKET, OSAL_SO_SNDTIMEO, &tv, sizeof(tv));

    /* 连接到远程地址 */
    struct sockaddr_in server_addr;
    OSAL_Memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip_addr, &server_addr.sin_addr) <= 0)
    {
        OSAL_close(ctx->sockfd);
        OSAL_Free(ctx);
        return OS_ERROR;
    }

    if (OSAL_connect(ctx->sockfd, (osal_sockaddr_t *)&server_addr, sizeof(server_addr)) < 0)
    {
        OSAL_close(ctx->sockfd);
        OSAL_Free(ctx);
        return OS_ERROR;
    }

    *handle = ctx;
    return OS_SUCCESS;
}

/**
 * @brief 反初始化网络通信
 */
int32 bmc_redfish_deinit(void *handle)
{
    if (handle == NULL)
    {
        return OS_ERROR;
    }

    bmc_redfish_context_t *ctx = (bmc_redfish_context_t *)handle;

    OSAL_close(ctx->sockfd);
    OSAL_Free(ctx);

    return OS_SUCCESS;
}

/**
 * @brief 网络发送并接收
 */
int32 bmc_redfish_send_recv(void *handle,
                       const uint8 *request,
                       uint32 req_size,
                       uint8 *response,
                       uint32 resp_size,
                       uint32 *actual_size)
{
    if (handle == NULL || request == NULL)
    {
        return OS_ERROR;
    }

    bmc_redfish_context_t *ctx = (bmc_redfish_context_t *)handle;

    /* 发送请求 */
    osal_ssize_t sent = OSAL_send(ctx->sockfd, request, req_size, 0);
    if (sent != (osal_ssize_t)req_size)
    {
        return OS_ERROR;
    }

    /* 接收响应 */
    if (response != NULL && resp_size > 0)
    {
        osal_ssize_t recv_len = OSAL_recv(ctx->sockfd, response, resp_size, 0);
        if (recv_len < 0)
        {
            return OS_ERROR;
        }

        if (actual_size != NULL)
        {
            *actual_size = (uint32)recv_len;
        }
    }

    return OS_SUCCESS;
}

/**
 * @brief 初始化串口通信
 */
int32 bmc_serial_init(const char *device, uint32 baudrate, uint32 timeout_ms, void **handle)
{
    if (device == NULL || handle == NULL)
    {
        return OS_ERROR;
    }

    bmc_serial_context_t *ctx = (bmc_serial_context_t *)OSAL_Malloc(sizeof(bmc_serial_context_t));
    if (ctx == NULL)
    {
        return OS_ERROR;
    }

    OSAL_Memset(ctx, 0, sizeof(bmc_serial_context_t));
    ctx->timeout_ms = timeout_ms;

    /* 打开串口 */
    hal_serial_config_t serial_config = {
        .baud_rate = baudrate,
        .data_bits = 8,
        .stop_bits = 1,
        .parity = 'N'
    };

    if (HAL_Serial_Open(device, &serial_config, &ctx->serial_handle) != OS_SUCCESS)
    {
        OSAL_Free(ctx);
        return OS_ERROR;
    }

    *handle = ctx;
    return OS_SUCCESS;
}

/**
 * @brief 反初始化串口通信
 */
int32 bmc_serial_deinit(void *handle)
{
    if (handle == NULL)
    {
        return OS_ERROR;
    }

    bmc_serial_context_t *ctx = (bmc_serial_context_t *)handle;

    HAL_Serial_Close(ctx->serial_handle);
    OSAL_Free(ctx);

    return OS_SUCCESS;
}

/**
 * @brief 串口发送并接收
 */
int32 bmc_serial_send_recv(void *handle,
                          const uint8 *request,
                          uint32 req_size,
                          uint8 *response,
                          uint32 resp_size,
                          uint32 *actual_size)
{
    if (handle == NULL || request == NULL)
    {
        return OS_ERROR;
    }

    bmc_serial_context_t *ctx = (bmc_serial_context_t *)handle;

    /* 发送请求 */
    if (HAL_Serial_Write(ctx->serial_handle, request, req_size, ctx->timeout_ms) != (int32)req_size)
    {
        return OS_ERROR;
    }

    /* 接收响应 */
    if (response != NULL && resp_size > 0)
    {
        int32 recv_len = HAL_Serial_Read(ctx->serial_handle, response, resp_size, ctx->timeout_ms);
        if (recv_len < 0)
        {
            return OS_ERROR;
        }

        if (actual_size != NULL)
        {
            *actual_size = recv_len;
        }
    }

    return OS_SUCCESS;
}
