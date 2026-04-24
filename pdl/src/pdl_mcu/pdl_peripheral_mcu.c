/************************************************************************
 * MCU外设驱动实现（I2C接口）
 *
 * 支持通过I2C与MCU通信
 ************************************************************************/

#include "pdl_peripherals/pdl_peripheral_mcu.h"
#include "pdl_peripheral_device.h"
#include "osal.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * MCU驱动上下文
 */
typedef struct
{
    mcu_config_t config;
    void *i2c_handle;
    void *spi_handle;
    void *uart_handle;
    bool initialized;
    osal_id_t mutex;
    mcu_version_t version;
    mcu_status_t status;
} mcu_driver_context_t;

/*
 * CRC16计算（预留）
 */
static uint16 calculate_crc16(const uint8 *data, uint32 len) __attribute__((unused));
static uint16 calculate_crc16(const uint8 *data, uint32 len)
{
    uint16 crc = 0xFFFF;
    for (uint32 i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (int j = 0; j < 8; j++)
        {
            if (crc & 0x0001)
            {
                crc = (crc >> 1) ^ 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }
    return crc;
}

/*
 * 通过I2C发送命令（TODO: 需要实现HAL I2C接口）
 */
static int32 mcu_i2c_send_command(mcu_driver_context_t *ctx,
                                  const mcu_cmd_frame_t *cmd,
                                  mcu_resp_frame_t *resp,
                                  uint32 timeout_ms)
{
    /* TODO: 实现I2C通信 */
    (void)ctx;
    (void)cmd;
    (void)timeout_ms;

    /* 模拟响应 */
    resp->status = 0;
    resp->len = 0;

    return OS_SUCCESS;
}

/*
 * 初始化MCU驱动
 */
static int32 mcu_driver_init(void *config, peripheral_handle_t *handle)
{
    if (config == NULL || handle == NULL)
    {
        return OS_ERROR;
    }

    mcu_config_t *cfg = (mcu_config_t *)config;
    mcu_driver_context_t *ctx = (mcu_driver_context_t *)malloc(sizeof(mcu_driver_context_t));
    if (ctx == NULL)
    {
        return OS_ERROR;
    }

    memset(ctx, 0, sizeof(mcu_driver_context_t));
    memcpy(&ctx->config, cfg, sizeof(mcu_config_t));

    /* 创建互斥锁 */
    if (OS_MutexCreate(&ctx->mutex, "mcu_mutex", 0) != OS_SUCCESS)
    {
        free(ctx);
        return OS_ERROR;
    }

    /* 初始化通信接口 */
    int32 ret = OS_SUCCESS;
    switch (cfg->interface)
    {
        case MCU_INTERFACE_I2C:
            /* TODO: 实现I2C接口初始化 */
            ctx->i2c_handle = NULL;
            ret = OS_SUCCESS;
            break;
        case MCU_INTERFACE_SPI:
            /* TODO: 实现SPI接口 */
            ret = OS_ERROR;
            break;
        case MCU_INTERFACE_UART:
            /* TODO: 实现UART接口 */
            ret = OS_ERROR;
            break;
        default:
            ret = OS_ERROR;
            break;
    }

    if (ret != OS_SUCCESS)
    {
        OS_MutexDelete(ctx->mutex);
        free(ctx);
        return ret;
    }

    ctx->initialized = true;
    *handle = (peripheral_handle_t)ctx;

    return OS_SUCCESS;
}

/*
 * 反初始化MCU驱动
 */
static int32 mcu_driver_deinit(peripheral_handle_t handle)
{
    if (handle == NULL)
    {
        return OS_ERROR;
    }

    mcu_driver_context_t *ctx = (mcu_driver_context_t *)handle;

    /* 关闭通信接口 */
    switch (ctx->config.interface)
    {
        case MCU_INTERFACE_I2C:
            /* TODO: 实现I2C关闭 */
            break;
        case MCU_INTERFACE_SPI:
            /* TODO */
            break;
        case MCU_INTERFACE_UART:
            /* TODO */
            break;
    }

    OS_MutexDelete(ctx->mutex);
    free(ctx);

    return OS_SUCCESS;
}

/*
 * MCU上电
 */
static int32 mcu_driver_power_on(peripheral_handle_t handle)
{
    mcu_driver_context_t *ctx = (mcu_driver_context_t *)handle;

    mcu_cmd_frame_t cmd = {
        .cmd = MCU_CMD_POWER_ON,
        .len = 0
    };
    mcu_resp_frame_t resp;

    OS_MutexLock(ctx->mutex);
    int32 ret = mcu_i2c_send_command(ctx, &cmd, &resp, ctx->config.cmd_timeout_ms);
    OS_MutexUnlock(ctx->mutex);

    return ret;
}

/*
 * MCU下电
 */
static int32 mcu_driver_power_off(peripheral_handle_t handle)
{
    mcu_driver_context_t *ctx = (mcu_driver_context_t *)handle;

    mcu_cmd_frame_t cmd = {
        .cmd = MCU_CMD_POWER_OFF,
        .len = 0
    };
    mcu_resp_frame_t resp;

    OS_MutexLock(ctx->mutex);
    int32 ret = mcu_i2c_send_command(ctx, &cmd, &resp, ctx->config.cmd_timeout_ms);
    OS_MutexUnlock(ctx->mutex);

    return ret;
}

/*
 * MCU复位
 */
static int32 mcu_driver_reset(peripheral_handle_t handle)
{
    mcu_driver_context_t *ctx = (mcu_driver_context_t *)handle;

    mcu_cmd_frame_t cmd = {
        .cmd = MCU_CMD_RESET,
        .len = 0
    };
    mcu_resp_frame_t resp;

    OS_MutexLock(ctx->mutex);
    int32 ret = mcu_i2c_send_command(ctx, &cmd, &resp, ctx->config.cmd_timeout_ms);
    OS_MutexUnlock(ctx->mutex);

    return ret;
}

/*
 * 获取MCU状态
 */
static int32 mcu_driver_get_status(peripheral_handle_t handle, peripheral_status_t *status)
{
    mcu_driver_context_t *ctx = (mcu_driver_context_t *)handle;

    mcu_cmd_frame_t cmd = {
        .cmd = MCU_CMD_GET_STATUS,
        .len = 0
    };
    mcu_resp_frame_t resp;

    OS_MutexLock(ctx->mutex);
    int32 ret = mcu_i2c_send_command(ctx, &cmd, &resp, ctx->config.cmd_timeout_ms);
    OS_MutexUnlock(ctx->mutex);

    if (ret == OS_SUCCESS && resp.len >= 8)
    {
        status->state = PERIPHERAL_STATE_ONLINE;
        status->power = PERIPHERAL_POWER_ON;
        status->uptime_sec = (resp.data[0] << 24) | (resp.data[1] << 16) |
                            (resp.data[2] << 8) | resp.data[3];
        status->temperature = (float)resp.data[4];
        status->fault = (resp.data[5] != 0);
        status->error_code = resp.data[6];
    }

    return ret;
}

/*
 * 获取MCU信息
 */
static int32 mcu_driver_get_info(peripheral_handle_t handle, peripheral_info_t *info)
{
    mcu_driver_context_t *ctx = (mcu_driver_context_t *)handle;

    strncpy(info->name, ctx->config.name, sizeof(info->name) - 1);
    info->name[sizeof(info->name) - 1] = '\0';
    info->type = PERIPHERAL_TYPE_MCU;
    info->interface = (ctx->config.interface == MCU_INTERFACE_I2C) ? COMM_INTERFACE_I2C :
                     (ctx->config.interface == MCU_INTERFACE_SPI) ? COMM_INTERFACE_SPI :
                     COMM_INTERFACE_UART;
    info->capabilities = PERIPHERAL_CAP_POWER_CONTROL | PERIPHERAL_CAP_RESET |
                        PERIPHERAL_CAP_STATUS_QUERY | PERIPHERAL_CAP_COMMAND_EXEC;

    /* 获取版本信息 */
    mcu_cmd_frame_t cmd = {
        .cmd = MCU_CMD_GET_VERSION,
        .len = 0
    };
    mcu_resp_frame_t resp;

    OS_MutexLock(ctx->mutex);
    int32 ret = mcu_i2c_send_command(ctx, &cmd, &resp, ctx->config.cmd_timeout_ms);
    OS_MutexUnlock(ctx->mutex);

    if (ret == OS_SUCCESS && resp.len >= 4)
    {
        snprintf(info->firmware_version, sizeof(info->firmware_version),
                "%d.%d.%d.%d", resp.data[0], resp.data[1], resp.data[2], resp.data[3]);
    }

    return OS_SUCCESS;
}

/*
 * 执行MCU命令
 */
static int32 mcu_driver_execute_command(peripheral_handle_t handle,
                                       const void *cmd,
                                       uint32 cmd_len,
                                       void *response,
                                       uint32 resp_size,
                                       uint32 *actual_size)
{
    mcu_driver_context_t *ctx = (mcu_driver_context_t *)handle;

    if (cmd_len < 2)
    {
        return OS_ERROR;
    }

    const mcu_cmd_frame_t *cmd_frame = (const mcu_cmd_frame_t *)cmd;
    mcu_resp_frame_t resp;

    OS_MutexLock(ctx->mutex);
    int32 ret = mcu_i2c_send_command(ctx, cmd_frame, &resp, ctx->config.cmd_timeout_ms);
    OS_MutexUnlock(ctx->mutex);

    if (ret == OS_SUCCESS && response != NULL)
    {
        uint32 copy_len = (resp.len < resp_size) ? resp.len : resp_size;
        memcpy(response, resp.data, copy_len);
        if (actual_size != NULL)
        {
            *actual_size = copy_len;
        }
    }

    return ret;
}

/*
 * MCU外设操作接口
 */
static const peripheral_ops_t mcu_peripheral_ops = {
    .init = mcu_driver_init,
    .deinit = mcu_driver_deinit,
    .power_on = mcu_driver_power_on,
    .power_off = mcu_driver_power_off,
    .reset = mcu_driver_reset,
    .get_status = mcu_driver_get_status,
    .get_info = mcu_driver_get_info,
    .execute_command = mcu_driver_execute_command,
    .read_data = NULL,
    .write_data = NULL,
    .register_callback = NULL,
    .get_stats = NULL,
    .reset_stats = NULL
};

/**
 * @brief 注册MCU外设驱动
 */
int32 MCU_RegisterDriver(void)
{
    return Peripheral_RegisterDriver(PERIPHERAL_TYPE_MCU, &mcu_peripheral_ops);
}

/**
 * @brief 初始化MCU外设
 */
int32 MCU_Init(const mcu_config_t *config, peripheral_handle_t *handle)
{
    return Peripheral_Create(PERIPHERAL_TYPE_MCU, (void *)config, handle);
}

/**
 * @brief 反初始化MCU外设
 */
int32 MCU_Deinit(peripheral_handle_t handle)
{
    return Peripheral_Destroy(handle);
}

/**
 * @brief 获取MCU版本
 */
int32 MCU_GetVersion(peripheral_handle_t handle, mcu_version_t *version)
{
    mcu_cmd_frame_t cmd = {
        .cmd = MCU_CMD_GET_VERSION,
        .len = 0
    };
    mcu_resp_frame_t resp;
    uint32 actual_size;

    int32 ret = Peripheral_ExecuteCommand(handle, &cmd, sizeof(cmd),
                                         &resp, sizeof(resp), &actual_size);

    if (ret == OS_SUCCESS && resp.len >= 4)
    {
        version->major = resp.data[0];
        version->minor = resp.data[1];
        version->patch = resp.data[2];
        version->build = resp.data[3];
        snprintf(version->version_string, sizeof(version->version_string),
                "%d.%d.%d.%d", version->major, version->minor,
                version->patch, version->build);
    }

    return ret;
}

/**
 * @brief 获取MCU状态
 */
int32 MCU_GetStatus(peripheral_handle_t handle, mcu_status_t *status)
{
    peripheral_status_t peri_status;
    int32 ret = Peripheral_GetStatus(handle, &peri_status);

    if (ret == OS_SUCCESS)
    {
        status->online = (peri_status.state == PERIPHERAL_STATE_ONLINE);
        status->uptime_sec = peri_status.uptime_sec;
        status->error_code = peri_status.error_code;
        status->temperature = peri_status.temperature;
    }

    return ret;
}

/**
 * @brief MCU复位
 */
int32 MCU_Reset(peripheral_handle_t handle)
{
    return Peripheral_Reset(handle);
}

/**
 * @brief 发送命令到MCU
 */
int32 MCU_SendCommand(peripheral_handle_t handle,
                     const mcu_cmd_frame_t *cmd,
                     mcu_resp_frame_t *resp,
                     uint32 timeout_ms)
{
    (void)timeout_ms;
    uint32 actual_size;
    return Peripheral_ExecuteCommand(handle, cmd, sizeof(*cmd),
                                    resp, sizeof(*resp), &actual_size);
}

/**
 * @brief 读取MCU寄存器
 */
int32 MCU_ReadRegister(peripheral_handle_t handle, uint8 reg_addr, uint8 *value)
{
    mcu_cmd_frame_t cmd = {
        .cmd = MCU_CMD_CUSTOM,
        .len = 1,
        .data = {reg_addr}
    };
    mcu_resp_frame_t resp;
    uint32 actual_size;

    int32 ret = Peripheral_ExecuteCommand(handle, &cmd, sizeof(cmd),
                                         &resp, sizeof(resp), &actual_size);

    if (ret == OS_SUCCESS && resp.len >= 1)
    {
        *value = resp.data[0];
    }

    return ret;
}

/**
 * @brief 写入MCU寄存器
 */
int32 MCU_WriteRegister(peripheral_handle_t handle, uint8 reg_addr, uint8 value)
{
    mcu_cmd_frame_t cmd = {
        .cmd = MCU_CMD_CUSTOM,
        .len = 2,
        .data = {reg_addr, value}
    };
    mcu_resp_frame_t resp;
    uint32 actual_size;

    return Peripheral_ExecuteCommand(handle, &cmd, sizeof(cmd),
                                    &resp, sizeof(resp), &actual_size);
}

/**
 * @brief MCU固件升级
 */
int32 MCU_FirmwareUpdate(peripheral_handle_t handle,
                        const char *firmware_path,
                        void (*progress_callback)(uint32 percent))
{
    (void)handle;
    (void)firmware_path;
    (void)progress_callback;
    /* TODO: 实现固件升级逻辑 */
    return OS_ERROR;
}
