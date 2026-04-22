/************************************************************************
 * BMC载荷通信服务 - Linux实现
 ************************************************************************/

#include "hal_can.h"
#include "hal_serial.h"
#include "hal_network.h"
#include "service_payload_bmc.h"
#include "osal.h"
#include "system_config.h"
#include <stdlib.h>
#include <string.h>

/*
 * BMC载荷服务上下文
 */
typedef struct
{
    bmc_payload_config_t config;

    /* 通信句柄 */
    hal_network_handle_t net_handle;
    hal_serial_handle_t serial_handle;

    /* 当前通道 */
    bmc_channel_t current_channel;
    bool connected;

    /* 统计信息 */
    uint32 cmd_count;
    uint32 success_count;
    uint32 fail_count;

    /* 互斥锁 */
    osal_id_t mutex;
} bmc_payload_context_t;

/*
 * 初始化BMC载荷服务
 */
int32 PayloadBMC_Init(const bmc_payload_config_t *config,
                      bmc_payload_handle_t *handle)
{
    if (config == NULL || handle == NULL) {
        return OS_INVALID_POINTER;
    }

    /* 分配上下文 */
    bmc_payload_context_t *ctx = (bmc_payload_context_t *)malloc(sizeof(bmc_payload_context_t));
    if (ctx == NULL) {
        LOG_ERROR("SVC_BMC", "Failed to allocate BMC payload context");
        return OS_ERROR;
    }

    memset(ctx, 0, sizeof(bmc_payload_context_t));
    memcpy(&ctx->config, config, sizeof(bmc_payload_config_t));
    ctx->current_channel = config->primary_channel;

    /* 创建互斥锁 */
    int32 ret = OS_MutexCreate(&ctx->mutex, "BMC_MTX", 0);
    if (ret != OS_SUCCESS) {
        LOG_ERROR("SVC_BMC", "Failed to create mutex");
        free(ctx);
        return ret;
    }

    /* 初始化网络通道 */
    if (config->network.enabled) {
        hal_network_config_t net_cfg = {
            .ip_addr = config->network.ip_addr,
            .port = config->network.port,
            .timeout_ms = config->network.timeout_ms
        };

        ret = HAL_Network_Open(&net_cfg, &ctx->net_handle);
        if (ret != OS_SUCCESS) {
            LOG_WARN("SVC_BMC", "Failed to open network channel");
        } else {
            LOG_INFO("SVC_BMC", "Network channel opened: %s:%d",
                     config->network.ip_addr, config->network.port);
        }
    }

    /* 初始化串口通道 */
    if (config->serial.enabled) {
        hal_serial_config_t serial_cfg = {
            .baudrate = config->serial.baudrate,
            .databits = 8,
            .stopbits = 1,
            .parity = HAL_SERIAL_PARITY_NONE,
            .timeout_ms = config->serial.timeout_ms
        };

        ret = HAL_Serial_Open(config->serial.device, &serial_cfg, &ctx->serial_handle);
        if (ret != OS_SUCCESS) {
            LOG_WARN("SVC_BMC", "Failed to open serial channel");
        } else {
            LOG_INFO("SVC_BMC", "Serial channel opened: %s", config->serial.device);
        }
    }

    /* 尝试连接主通道 */
    if (ctx->current_channel == BMC_CHANNEL_NETWORK && ctx->net_handle != NULL) {
        ctx->connected = true;  /* 网络通道打开即可用 */
    } else if (ctx->current_channel == BMC_CHANNEL_SERIAL && ctx->serial_handle != NULL) {
        ctx->connected = true;  /* 串口打开即连接 */
    }

    *handle = (bmc_payload_handle_t)ctx;
    LOG_INFO("SVC_BMC", "BMC payload service initialized");

    return OS_SUCCESS;
}

/*
 * 反初始化BMC载荷服务
 */
int32 PayloadBMC_Deinit(bmc_payload_handle_t handle)
{
    if (handle == NULL) {
        return OS_INVALID_POINTER;
    }

    bmc_payload_context_t *ctx = (bmc_payload_context_t *)handle;

    /* 关闭网络 */
    if (ctx->net_handle != NULL) {
        HAL_Network_Close(ctx->net_handle);
    }

    /* 关闭串口 */
    if (ctx->serial_handle != NULL) {
        HAL_Serial_Close(ctx->serial_handle);
    }

    /* 删除互斥锁 */
    OS_MutexDelete(ctx->mutex);

    free(ctx);
    LOG_INFO("SVC_BMC", "BMC payload service deinitialized");

    return OS_SUCCESS;
}

/*
 * 发送IPMI/Redfish命令
 */
int32 PayloadBMC_SendCommand(bmc_payload_handle_t handle,
                             bmc_protocol_t protocol,
                             const void *request,
                             uint32 req_size,
                             void *response,
                             uint32 resp_size,
                             uint32 *actual_size)
{
    (void)protocol;  /* 当前未使用，预留用于协议选择 */

    if (handle == NULL || request == NULL) {
        return OS_INVALID_POINTER;
    }

    bmc_payload_context_t *ctx = (bmc_payload_context_t *)handle;
    int32 ret;

    OS_MutexLock(ctx->mutex);

    ctx->cmd_count++;

    /* 检查连接 */
    if (!ctx->connected) {
        LOG_ERROR("SVC_BMC", "BMC not connected");
        ctx->fail_count++;
        OS_MutexUnlock(ctx->mutex);
        return OS_ERROR;
    }

    /* 根据当前通道发送 */
    if (ctx->current_channel == BMC_CHANNEL_NETWORK) {
        ret = HAL_Network_Send(ctx->net_handle, request, req_size, ETHERNET_TIMEOUT_MS);
        if (ret == OS_SUCCESS && response != NULL) {
            ret = HAL_Network_Recv(ctx->net_handle, response, resp_size, ETHERNET_TIMEOUT_MS);
            if (actual_size != NULL) {
                *actual_size = (ret > 0) ? ret : 0;
            }
        }
    } else {
        ret = HAL_Serial_Write(ctx->serial_handle, request, req_size, UART_TIMEOUT_MS);
        if (ret == OS_SUCCESS && response != NULL) {
            ret = HAL_Serial_Read(ctx->serial_handle, response, resp_size, UART_TIMEOUT_MS);
            if (actual_size != NULL) {
                *actual_size = (ret > 0) ? ret : 0;
            }
        }
    }

    if (ret == OS_SUCCESS) {
        ctx->success_count++;
    } else {
        ctx->fail_count++;
        LOG_ERROR("SVC_BMC", "Failed to send BMC command");
    }

    OS_MutexUnlock(ctx->mutex);

    return ret;
}

/*
 * 切换通信通道
 */
int32 PayloadBMC_SwitchChannel(bmc_payload_handle_t handle,
                               bmc_channel_t channel)
{
    if (handle == NULL) {
        return OS_INVALID_POINTER;
    }

    bmc_payload_context_t *ctx = (bmc_payload_context_t *)handle;

    OS_MutexLock(ctx->mutex);

    /* 检查通道是否可用 */
    if (channel == BMC_CHANNEL_NETWORK && ctx->net_handle == NULL) {
        LOG_ERROR("SVC_BMC", "Network channel not available");
        OS_MutexUnlock(ctx->mutex);
        return OS_ERROR;
    }

    if (channel == BMC_CHANNEL_SERIAL && ctx->serial_handle == NULL) {
        LOG_ERROR("SVC_BMC", "Serial channel not available");
        OS_MutexUnlock(ctx->mutex);
        return OS_ERROR;
    }

    /* 切换通道 */
    ctx->current_channel = channel;
    ctx->connected = false;

    /* 尝试连接新通道 */
    if (channel == BMC_CHANNEL_NETWORK) {
        ctx->connected = true;  /* 网络通道已打开即可用 */
        LOG_INFO("SVC_BMC", "Switched to network channel");
    } else {
        ctx->connected = true;
        LOG_INFO("SVC_BMC", "Switched to serial channel");
    }

    OS_MutexUnlock(ctx->mutex);

    return ctx->connected ? OS_SUCCESS : OS_ERROR;
}

/*
 * 获取载荷状态
 */
int32 PayloadBMC_GetStatus(bmc_payload_handle_t handle,
                           bmc_payload_status_t *status)
{
    if (handle == NULL || status == NULL) {
        return OS_INVALID_POINTER;
    }

    bmc_payload_context_t *ctx = (bmc_payload_context_t *)handle;

    OS_MutexLock(ctx->mutex);

    /* 填充状态信息 */
    status->power_state = BMC_POWER_UNKNOWN;  /* 需要查询BMC */
    status->bmc_ready = ctx->connected;
    status->uptime_sec = 0;  /* 需要查询BMC */
    status->cpu_temp = 0.0f;
    status->inlet_temp = 0.0f;

    OS_MutexUnlock(ctx->mutex);

    return OS_SUCCESS;
}

/*
 * 获取服务统计信息
 */
int32 PayloadBMC_GetStats(bmc_payload_handle_t handle,
                          uint32 *cmd_count,
                          uint32 *success_count,
                          uint32 *fail_count)
{
    if (handle == NULL) {
        return OS_INVALID_POINTER;
    }

    bmc_payload_context_t *ctx = (bmc_payload_context_t *)handle;

    OS_MutexLock(ctx->mutex);

    if (cmd_count != NULL) *cmd_count = ctx->cmd_count;
    if (success_count != NULL) *success_count = ctx->success_count;
    if (fail_count != NULL) *fail_count = ctx->fail_count;

    OS_MutexUnlock(ctx->mutex);

    return OS_SUCCESS;
}
