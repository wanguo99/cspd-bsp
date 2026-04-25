/************************************************************************
 * BMC服务实现
 *
 * 职责：
 * - 实现对外业务接口
 * - 管理通信通道（网络/串口）切换
 * - 调度内部IPMI协议和通信模块
 ************************************************************************/

#include "pdl_bmc.h"
#include "pdl_bmc_internal.h"
#include "osal.h"
#include <stdlib.h>
#include <string.h>

/*
 * BMC服务上下文
 */
typedef struct
{
    bmc_config_t config;

    /* 通信句柄 */
    void *net_handle;
    void *serial_handle;

    /* 当前通道 */
    bmc_channel_t current_channel;
    bool connected;

    /* 统计信息 */
    uint32 cmd_count;
    uint32 success_count;
    uint32 fail_count;
    uint32 switch_count;

    /* 互斥锁 */
    osal_id_t mutex;
} bmc_context_t;

/**
 * @brief 初始化BMC服务
 */
int32 PDL_BMC_Init(const bmc_config_t *config,
                        bmc_handle_t *handle)
{
    if (config == NULL || handle == NULL)
    {
        return OS_ERROR;
    }

    /* 分配上下文 */
    bmc_context_t *ctx = (bmc_context_t *)OSAL_Malloc(sizeof(bmc_context_t));
    if (ctx == NULL)
    {
        LOG_ERROR("BMC", "Failed to allocate context");
        return OS_ERROR;
    }

    memset(ctx, 0, sizeof(bmc_context_t));
    memcpy(&ctx->config, config, sizeof(bmc_config_t));
    ctx->current_channel = config->primary_channel;

    /* 创建互斥锁 */
    if (OSAL_MutexCreate(&ctx->mutex, "bmc_mutex", 0) != OS_SUCCESS)
    {
        LOG_ERROR("BMC", "Failed to create mutex");
        OSAL_Free(ctx);
        return OS_ERROR;
    }

    /* 初始化网络通道 */
    if (config->network.enabled)
    {
        int32 ret = bmc_redfish_init(config->network.ip_addr,
                                config->network.port,
                                config->network.timeout_ms,
                                &ctx->net_handle);
        if (ret != OS_SUCCESS)
        {
            LOG_WARN("BMC", "Failed to open network channel");
        }
        else
        {
            LOG_INFO("BMC", "Network channel opened: %s:%d",
                     config->network.ip_addr, config->network.port);
        }
    }

    /* 初始化串口通道 */
    if (config->serial.enabled)
    {
        int32 ret = bmc_serial_init(config->serial.device,
                                   config->serial.baudrate,
                                   config->serial.timeout_ms,
                                   &ctx->serial_handle);
        if (ret != OS_SUCCESS)
        {
            LOG_WARN("BMC", "Failed to open serial channel");
        }
        else
        {
            LOG_INFO("BMC", "Serial channel opened: %s", config->serial.device);
        }
    }

    /* 检查主通道是否可用 */
    if (ctx->current_channel == BMC_CHANNEL_NETWORK && ctx->net_handle != NULL)
    {
        ctx->connected = true;
    }
    else if (ctx->current_channel == BMC_CHANNEL_SERIAL && ctx->serial_handle != NULL)
    {
        ctx->connected = true;
    }

    *handle = (bmc_handle_t)ctx;
    LOG_INFO("BMC", "BMC payload service initialized");

    return OS_SUCCESS;
}

/**
 * @brief 反初始化BMC服务
 */
int32 PDL_BMC_Deinit(bmc_handle_t handle)
{
    if (handle == NULL)
    {
        return OS_ERROR;
    }

    bmc_context_t *ctx = (bmc_context_t *)handle;

    /* 关闭网络 */
    if (ctx->net_handle != NULL)
    {
        bmc_redfish_deinit(ctx->net_handle);
    }

    /* 关闭串口 */
    if (ctx->serial_handle != NULL)
    {
        bmc_serial_deinit(ctx->serial_handle);
    }

    /* 删除互斥锁 */
    OSAL_MutexDelete(ctx->mutex);

    OSAL_Free(ctx);
    LOG_INFO("BMC", "BMC payload service deinitialized");

    return OS_SUCCESS;
}

/**
 * @brief 电源开机
 */
int32 PDL_BMC_PowerOn(bmc_handle_t handle)
{
    if (handle == NULL)
    {
        return OS_ERROR;
    }

    bmc_context_t *ctx = (bmc_context_t *)handle;

    OSAL_MutexLock(ctx->mutex);

    ctx->cmd_count++;

    int32 ret;
    if (ctx->current_channel == BMC_CHANNEL_NETWORK)
    {
        ret = bmc_ipmi_power_on(ctx->net_handle, bmc_redfish_send_recv);
    }
    else
    {
        ret = bmc_ipmi_power_on(ctx->serial_handle, bmc_serial_send_recv);
    }

    if (ret == OS_SUCCESS)
    {
        ctx->success_count++;
        LOG_INFO("BMC", "Power on command sent");
    }
    else
    {
        ctx->fail_count++;
        LOG_ERROR("BMC", "Failed to send power on command");
    }

    OSAL_MutexUnlock(ctx->mutex);

    return ret;
}

/**
 * @brief 电源关机
 */
int32 PDL_BMC_PowerOff(bmc_handle_t handle)
{
    if (handle == NULL)
    {
        return OS_ERROR;
    }

    bmc_context_t *ctx = (bmc_context_t *)handle;

    OSAL_MutexLock(ctx->mutex);

    ctx->cmd_count++;

    int32 ret;
    if (ctx->current_channel == BMC_CHANNEL_NETWORK)
    {
        ret = bmc_ipmi_power_off(ctx->net_handle, bmc_redfish_send_recv);
    }
    else
    {
        ret = bmc_ipmi_power_off(ctx->serial_handle, bmc_serial_send_recv);
    }

    if (ret == OS_SUCCESS)
    {
        ctx->success_count++;
        LOG_INFO("BMC", "Power off command sent");
    }
    else
    {
        ctx->fail_count++;
        LOG_ERROR("BMC", "Failed to send power off command");
    }

    OSAL_MutexUnlock(ctx->mutex);

    return ret;
}

/**
 * @brief 电源复位
 */
int32 PDL_BMC_PowerReset(bmc_handle_t handle)
{
    if (handle == NULL)
    {
        return OS_ERROR;
    }

    bmc_context_t *ctx = (bmc_context_t *)handle;

    OSAL_MutexLock(ctx->mutex);

    ctx->cmd_count++;

    int32 ret;
    if (ctx->current_channel == BMC_CHANNEL_NETWORK)
    {
        ret = bmc_ipmi_power_reset(ctx->net_handle, bmc_redfish_send_recv);
    }
    else
    {
        ret = bmc_ipmi_power_reset(ctx->serial_handle, bmc_serial_send_recv);
    }

    if (ret == OS_SUCCESS)
    {
        ctx->success_count++;
        LOG_INFO("BMC", "Power reset command sent");
    }
    else
    {
        ctx->fail_count++;
        LOG_ERROR("BMC", "Failed to send power reset command");
    }

    OSAL_MutexUnlock(ctx->mutex);

    return ret;
}

/**
 * @brief 查询电源状态
 */
int32 PDL_BMC_GetPowerState(bmc_handle_t handle,
                                 bmc_power_state_t *state)
{
    if (handle == NULL || state == NULL)
    {
        return OS_ERROR;
    }

    bmc_context_t *ctx = (bmc_context_t *)handle;

    OSAL_MutexLock(ctx->mutex);

    ctx->cmd_count++;

    int32 ret;
    if (ctx->current_channel == BMC_CHANNEL_NETWORK)
    {
        ret = bmc_ipmi_get_power_state(ctx->net_handle, bmc_redfish_send_recv, state);
    }
    else
    {
        ret = bmc_ipmi_get_power_state(ctx->serial_handle, bmc_serial_send_recv, state);
    }

    if (ret == OS_SUCCESS)
    {
        ctx->success_count++;
    }
    else
    {
        ctx->fail_count++;
    }

    OSAL_MutexUnlock(ctx->mutex);

    return ret;
}

/**
 * @brief 读取传感器
 */
int32 PDL_BMC_ReadSensors(bmc_handle_t handle,
                               bmc_sensor_type_t type,
                               bmc_sensor_reading_t *readings,
                               uint32 max_count,
                               uint32 *actual_count)
{
    if (handle == NULL)
    {
        return OS_ERROR;
    }

    bmc_context_t *ctx = (bmc_context_t *)handle;

    OSAL_MutexLock(ctx->mutex);

    ctx->cmd_count++;

    int32 ret;
    if (ctx->current_channel == BMC_CHANNEL_NETWORK)
    {
        ret = bmc_ipmi_read_sensors(ctx->net_handle, bmc_redfish_send_recv,
                                   type, readings, max_count, actual_count);
    }
    else
    {
        ret = bmc_ipmi_read_sensors(ctx->serial_handle, bmc_serial_send_recv,
                                   type, readings, max_count, actual_count);
    }

    if (ret == OS_SUCCESS)
    {
        ctx->success_count++;
    }
    else
    {
        ctx->fail_count++;
    }

    OSAL_MutexUnlock(ctx->mutex);

    return ret;
}

/**
 * @brief 执行原始IPMI命令
 */
int32 PDL_BMC_ExecuteCommand(bmc_handle_t handle,
                                  const char *cmd,
                                  char *response,
                                  uint32 resp_size)
{
    (void)handle;
    (void)cmd;
    (void)response;
    (void)resp_size;
    /* TODO: 实现原始命令执行 */
    return OS_ERROR;
}

/**
 * @brief 切换通信通道
 */
int32 PDL_BMC_SwitchChannel(bmc_handle_t handle,
                                 bmc_channel_t channel)
{
    if (handle == NULL)
    {
        return OS_ERROR;
    }

    bmc_context_t *ctx = (bmc_context_t *)handle;

    OSAL_MutexLock(ctx->mutex);

    /* 检查通道是否可用 */
    if (channel == BMC_CHANNEL_NETWORK && ctx->net_handle == NULL)
    {
        LOG_ERROR("BMC", "Network channel not available");
        OSAL_MutexUnlock(ctx->mutex);
        return OS_ERROR;
    }

    if (channel == BMC_CHANNEL_SERIAL && ctx->serial_handle == NULL)
    {
        LOG_ERROR("BMC", "Serial channel not available");
        OSAL_MutexUnlock(ctx->mutex);
        return OS_ERROR;
    }

    /* 切换通道 */
    ctx->current_channel = channel;
    ctx->connected = true;
    ctx->switch_count++;

    LOG_INFO("BMC", "Switched to %s channel",
             (channel == BMC_CHANNEL_NETWORK) ? "network" : "serial");

    OSAL_MutexUnlock(ctx->mutex);

    return OS_SUCCESS;
}

/**
 * @brief 获取当前通道
 */
bmc_channel_t PDL_BMC_GetChannel(bmc_handle_t handle)
{
    if (handle == NULL)
    {
        return BMC_CHANNEL_NETWORK;
    }

    bmc_context_t *ctx = (bmc_context_t *)handle;
    return ctx->current_channel;
}

/**
 * @brief 检查连接状态
 */
bool PDL_BMC_IsConnected(bmc_handle_t handle)
{
    if (handle == NULL)
    {
        return false;
    }

    bmc_context_t *ctx = (bmc_context_t *)handle;
    return ctx->connected;
}

/**
 * @brief 获取服务统计信息
 */
int32 PDL_BMC_GetStats(bmc_handle_t handle,
                            uint32 *cmd_count,
                            uint32 *success_count,
                            uint32 *fail_count,
                            uint32 *switch_count)
{
    if (handle == NULL)
    {
        return OS_ERROR;
    }

    bmc_context_t *ctx = (bmc_context_t *)handle;

    OSAL_MutexLock(ctx->mutex);

    if (cmd_count != NULL) *cmd_count = ctx->cmd_count;
    if (success_count != NULL) *success_count = ctx->success_count;
    if (fail_count != NULL) *fail_count = ctx->fail_count;
    if (switch_count != NULL) *switch_count = ctx->switch_count;

    OSAL_MutexUnlock(ctx->mutex);

    return OS_SUCCESS;
}
