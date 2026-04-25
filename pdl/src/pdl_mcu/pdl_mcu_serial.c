/************************************************************************
 * MCU串口通信实现
 *
 * 职责：
 * - 封装串口收发
 * - 实现MCU的串口协议（帧格式、起止符、CRC校验）
 * - 超时和重试机制
 ************************************************************************/

#include "pdl_mcu_internal.h"
#include "pdl_mcu.h"
#include "hal_serial.h"
#include "osal.h"

/*
 * 串口协议帧格式：
 * [0xAA][0x55][cmd][len][data...][crc16_h][crc16_l]
 */
#define FRAME_HEADER_0    0xAA
#define FRAME_HEADER_1    0x55
#define FRAME_HEADER_SIZE 2
#define FRAME_CRC_SIZE    2
#define FRAME_OVERHEAD    (FRAME_HEADER_SIZE + 2 + FRAME_CRC_SIZE)  /* 头+cmd+len+CRC */

/*
 * 串口通信上下文
 */
typedef struct
{
    hal_serial_handle_t serial_handle;
    bool enable_crc;
    osal_id_t rx_mutex;
} mcu_serial_context_t;

/**
 * @brief 初始化串口通信
 */
int32 mcu_serial_init(const void *config, void **handle)
{
    if (config == NULL || handle == NULL)
    {
        return OS_ERROR;
    }

    const mcu_config_t *mcu_cfg = (const mcu_config_t *)config;
    mcu_serial_context_t *ctx = (mcu_serial_context_t *)OSAL_Malloc(sizeof(mcu_serial_context_t));
    if (ctx == NULL)
    {
        return OS_ERROR;
    }

    OSAL_Memset(ctx, 0, sizeof(mcu_serial_context_t));
    ctx->enable_crc = mcu_cfg->enable_crc;

    /* 打开串口设备 */
    hal_serial_config_t serial_config = {
        .baud_rate = mcu_cfg->serial.baudrate,
        .data_bits = mcu_cfg->serial.data_bits,
        .stop_bits = mcu_cfg->serial.stop_bits,
        .parity = mcu_cfg->serial.parity,
        .flow_control = 0  /* NONE */
    };

    if (HAL_Serial_Open(mcu_cfg->serial.device, &serial_config, &ctx->serial_handle) != OS_SUCCESS)
    {
        OSAL_Free(ctx);
        return OS_ERROR;
    }

    /* 创建接收互斥锁 */
    if (OSAL_MutexCreate(&ctx->rx_mutex, "mcu_serial_rx", 0) != OS_SUCCESS)
    {
        HAL_Serial_Close(ctx->serial_handle);
        OSAL_Free(ctx);
        return OS_ERROR;
    }

    *handle = ctx;
    return OS_SUCCESS;
}

/**
 * @brief 反初始化串口通信
 */
int32 mcu_serial_deinit(void *handle)
{
    if (handle == NULL)
    {
        return OS_ERROR;
    }

    mcu_serial_context_t *ctx = (mcu_serial_context_t *)handle;

    HAL_Serial_Close(ctx->serial_handle);
    OSAL_MutexDelete(ctx->rx_mutex);
    OSAL_Free(ctx);

    return OS_SUCCESS;
}

/**
 * @brief 封装串口帧
 */
static int32 mcu_serial_pack_frame(uint8 cmd_code,
                                   const uint8 *data,
                                   uint32 data_len,
                                   bool enable_crc,
                                   uint8 *frame,
                                   uint32 frame_size,
                                   uint32 *actual_size)
{
    uint32 required_size = FRAME_OVERHEAD + data_len;
    if (frame_size < required_size)
    {
        return OS_ERROR;
    }

    uint32 pos = 0;

    /* 帧头 */
    frame[pos++] = FRAME_HEADER_0;
    frame[pos++] = FRAME_HEADER_1;

    /* 命令和长度 */
    frame[pos++] = cmd_code;
    frame[pos++] = (uint8)data_len;

    /* 数据 */
    if (data != NULL && data_len > 0)
    {
        OSAL_Memcpy(&frame[pos], data, data_len);
        pos += data_len;
    }

    /* CRC校验 */
    if (enable_crc)
    {
        uint16 crc = mcu_protocol_calc_crc16(&frame[FRAME_HEADER_SIZE], pos - FRAME_HEADER_SIZE);
        frame[pos++] = (uint8)(crc >> 8);
        frame[pos++] = (uint8)(crc & 0xFF);
    }
    else
    {
        frame[pos++] = 0;
        frame[pos++] = 0;
    }

    *actual_size = pos;
    return OS_SUCCESS;
}

/**
 * @brief 解析串口帧
 */
static int32 mcu_serial_unpack_frame(const uint8 *frame,
                                     uint32 frame_len,
                                     bool enable_crc,
                                     uint8 *status,
                                     uint8 *data,
                                     uint32 data_size,
                                     uint32 *actual_size)
{
    /* 最小帧长度检查 */
    if (frame_len < FRAME_OVERHEAD)
    {
        return OS_ERROR;
    }

    /* 帧头检查 */
    if (frame[0] != FRAME_HEADER_0 || frame[1] != FRAME_HEADER_1)
    {
        return OS_ERROR;
    }

    /* CRC校验 */
    if (enable_crc)
    {
        uint16 crc_recv = (frame[frame_len - 2] << 8) | frame[frame_len - 1];
        uint16 crc_calc = mcu_protocol_calc_crc16(&frame[FRAME_HEADER_SIZE], frame_len - FRAME_OVERHEAD);
        if (crc_recv != crc_calc)
        {
            return OS_ERROR;
        }
    }

    /* 解析状态和数据 */
    *status = frame[2];
    uint8 data_len = frame[3];

    if (data != NULL && data_len > 0)
    {
        uint32 copy_len = (data_len < data_size) ? data_len : data_size;
        OSAL_Memcpy(data, &frame[4], copy_len);
        if (actual_size != NULL)
        {
            *actual_size = copy_len;
        }
    }

    return OS_SUCCESS;
}

/**
 * @brief 发送命令并接收响应
 */
int32 mcu_serial_send_command(void *handle,
                             uint8 cmd_code,
                             const uint8 *data,
                             uint32 data_len,
                             uint8 *response,
                             uint32 resp_size,
                             uint32 *actual_size,
                             uint32 timeout_ms)
{
    if (handle == NULL)
    {
        return OS_ERROR;
    }

    mcu_serial_context_t *ctx = (mcu_serial_context_t *)handle;

    /* 封装发送帧 */
    uint8 tx_frame[256];
    uint32 tx_len;

    if (mcu_serial_pack_frame(cmd_code, data, data_len, ctx->enable_crc,
                              tx_frame, sizeof(tx_frame), &tx_len) != OS_SUCCESS)
    {
        return OS_ERROR;
    }

    /* 发送 */
    if (HAL_Serial_Write(ctx->serial_handle, tx_frame, tx_len, timeout_ms) != (int32)tx_len)
    {
        return OS_ERROR;
    }

    /* 接收响应 */
    OSAL_MutexLock(ctx->rx_mutex);

    uint8 rx_frame[256];
    int32 rx_len = HAL_Serial_Read(ctx->serial_handle, rx_frame, sizeof(rx_frame), timeout_ms);

    if (rx_len > 0)
    {
        uint8 status;
        int32 ret = mcu_serial_unpack_frame(rx_frame, rx_len, ctx->enable_crc,
                                            &status, response, resp_size, actual_size);

        OSAL_MutexUnlock(ctx->rx_mutex);

        if (ret != OS_SUCCESS || status != 0)
        {
            return OS_ERROR;
        }

        return OS_SUCCESS;
    }

    OSAL_MutexUnlock(ctx->rx_mutex);

    return (rx_len == 0) ? OS_ERROR_TIMEOUT : OS_ERROR;
}
