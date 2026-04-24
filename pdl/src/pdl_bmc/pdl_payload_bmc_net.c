/************************************************************************
 * BMC载荷网络通信实现
 *
 * 职责：
 * - 封装以太网和串口通信
 * - 实现IPMI over LAN和IPMI over Serial
 ************************************************************************/

#include "pdl_payload_bmc_internal.h"
#include "hal_network.h"
#include "hal_serial.h"
#include "osal.h"
#include <stdlib.h>
#include <string.h>

/*
 * 网络通信上下文
 */
typedef struct
{
    hal_network_handle_t net_handle;
    uint32 timeout_ms;
} bmc_net_context_t;

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
int32 bmc_net_init(const char *ip_addr, uint16 port, uint32 timeout_ms, void **handle)
{
    if (ip_addr == NULL || handle == NULL)
    {
        return OS_ERROR;
    }

    bmc_net_context_t *ctx = (bmc_net_context_t *)malloc(sizeof(bmc_net_context_t));
    if (ctx == NULL)
    {
        return OS_ERROR;
    }

    memset(ctx, 0, sizeof(bmc_net_context_t));
    ctx->timeout_ms = timeout_ms;

    /* 打开网络连接 */
    hal_network_config_t net_config = {
        .ip_addr = ip_addr,
        .port = port,
        .timeout_ms = timeout_ms
    };

    if (HAL_NetworkOpen(&net_config, &ctx->net_handle) != OS_SUCCESS)
    {
        free(ctx);
        return OS_ERROR;
    }

    *handle = ctx;
    return OS_SUCCESS;
}

/**
 * @brief 反初始化网络通信
 */
int32 bmc_net_deinit(void *handle)
{
    if (handle == NULL)
    {
        return OS_ERROR;
    }

    bmc_net_context_t *ctx = (bmc_net_context_t *)handle;

    HAL_NetworkClose(ctx->net_handle);
    free(ctx);

    return OS_SUCCESS;
}

/**
 * @brief 网络发送并接收
 */
int32 bmc_net_send_recv(void *handle,
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

    bmc_net_context_t *ctx = (bmc_net_context_t *)handle;

    /* 发送请求 */
    if (HAL_NetworkSend(ctx->net_handle, request, req_size, ctx->timeout_ms) != OS_SUCCESS)
    {
        return OS_ERROR;
    }

    /* 接收响应 */
    if (response != NULL && resp_size > 0)
    {
        int32 recv_len = HAL_NetworkRecv(ctx->net_handle, response, resp_size, ctx->timeout_ms);
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

/**
 * @brief 初始化串口通信
 */
int32 bmc_serial_init(const char *device, uint32 baudrate, uint32 timeout_ms, void **handle)
{
    if (device == NULL || handle == NULL)
    {
        return OS_ERROR;
    }

    bmc_serial_context_t *ctx = (bmc_serial_context_t *)malloc(sizeof(bmc_serial_context_t));
    if (ctx == NULL)
    {
        return OS_ERROR;
    }

    memset(ctx, 0, sizeof(bmc_serial_context_t));
    ctx->timeout_ms = timeout_ms;

    /* 打开串口 */
    hal_serial_config_t serial_config = {
        .baud_rate = baudrate,
        .data_bits = 8,
        .stop_bits = 1,
        .parity = 'N'
    };

    if (HAL_SerialOpen(device, &serial_config, &ctx->serial_handle) != OS_SUCCESS)
    {
        free(ctx);
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

    HAL_SerialClose(ctx->serial_handle);
    free(ctx);

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
    if (HAL_SerialWrite(ctx->serial_handle, request, req_size, ctx->timeout_ms) != (int32)req_size)
    {
        return OS_ERROR;
    }

    /* 接收响应 */
    if (response != NULL && resp_size > 0)
    {
        int32 recv_len = HAL_SerialRead(ctx->serial_handle, response, resp_size, ctx->timeout_ms);
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
