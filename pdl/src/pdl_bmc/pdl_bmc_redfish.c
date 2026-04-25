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

/*
 * 网络通信上下文
 */
typedef struct
{
    osal_id_t sock_id;
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

    memset(ctx, 0, sizeof(bmc_redfish_context_t));
    ctx->timeout_ms = timeout_ms;

    /* 创建TCP Socket */
    if (OSAL_SocketOpen(&ctx->sock_id, 0, OS_SOCK_STREAM) != OS_SUCCESS)
    {
        OSAL_Free(ctx);
        return OS_ERROR;
    }

    /* 连接到远程地址 */
    if (OSAL_SocketConnect(ctx->sock_id, ip_addr, port, timeout_ms) != OS_SUCCESS)
    {
        OSAL_SocketClose(ctx->sock_id);
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

    OSAL_SocketClose(ctx->sock_id);
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
    int32 sent = OSAL_SocketSend(ctx->sock_id, request, req_size, ctx->timeout_ms);
    if (sent != (int32)req_size)
    {
        return OS_ERROR;
    }

    /* 接收响应 */
    if (response != NULL && resp_size > 0)
    {
        int32 recv_len = OSAL_SocketRecv(ctx->sock_id, response, resp_size, ctx->timeout_ms);
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

    bmc_serial_context_t *ctx = (bmc_serial_context_t *)OSAL_Malloc(sizeof(bmc_serial_context_t));
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
