/************************************************************************
 * MCU CAN通信实现
 *
 * 职责：
 * - 封装CAN总线收发
 * - 实现MCU的CAN协议（帧格式、ID映射）
 * - 超时和重试机制
 ************************************************************************/

#include "pdl_mcu_internal.h"
#include "pdl_mcu.h"
#include "hal_can.h"
#include "osal.h"

/*
 * CAN通信上下文
 */
typedef struct
{
    hal_can_handle_t can_handle;
    uint32 tx_id;
    uint32 rx_id;
    osal_id_t rx_mutex;
} mcu_can_context_t;

/**
 * @brief 初始化CAN通信
 */
int32 mcu_can_init(const void *config, void **handle)
{
    if (config == NULL || handle == NULL)
    {
        return OS_ERROR;
    }

    const mcu_config_t *mcu_cfg = (const mcu_config_t *)config;
    mcu_can_context_t *ctx = (mcu_can_context_t *)OSAL_Malloc(sizeof(mcu_can_context_t));
    if (ctx == NULL)
    {
        return OS_ERROR;
    }

    OSAL_Memset(ctx, 0, sizeof(mcu_can_context_t));
    ctx->tx_id = mcu_cfg->can.tx_id;
    ctx->rx_id = mcu_cfg->can.rx_id;

    /* 打开CAN设备 */
    hal_can_config_t can_config = {
        .interface = mcu_cfg->can.device,
        .baudrate = mcu_cfg->can.bitrate,
        .rx_timeout = 1000,
        .tx_timeout = 1000
    };

    if (HAL_CAN_Init(&can_config, &ctx->can_handle) != OS_SUCCESS)
    {
        OSAL_Free(ctx);
        return OS_ERROR;
    }

    /* 创建接收互斥锁 */
    if (OSAL_MutexCreate(&ctx->rx_mutex, "mcu_can_rx", 0) != OS_SUCCESS)
    {
        HAL_CAN_Deinit(ctx->can_handle);
        OSAL_Free(ctx);
        return OS_ERROR;
    }

    *handle = ctx;
    return OS_SUCCESS;
}

/**
 * @brief 反初始化CAN通信
 */
int32 mcu_can_deinit(void *handle)
{
    if (handle == NULL)
    {
        return OS_ERROR;
    }

    mcu_can_context_t *ctx = (mcu_can_context_t *)handle;

    HAL_CAN_Deinit(ctx->can_handle);
    OSAL_MutexDelete(ctx->rx_mutex);
    OSAL_Free(ctx);

    return OS_SUCCESS;
}

/**
 * @brief 发送命令并接收响应
 */
int32 mcu_can_send_command(void *handle,
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

    mcu_can_context_t *ctx = (mcu_can_context_t *)handle;

    /* 封装CAN帧：[cmd_code][data_len][data...] */
    uint8 tx_frame[8];
    uint32 tx_len = 0;

    tx_frame[tx_len++] = cmd_code;
    tx_frame[tx_len++] = (uint8)data_len;

    if (data != NULL && data_len > 0)
    {
        uint32 copy_len = (data_len > 6) ? 6 : data_len;  /* CAN最多8字节，留2字节给头 */
        OSAL_Memcpy(&tx_frame[tx_len], data, copy_len);
        tx_len += copy_len;
    }

    /* 发送CAN帧 */
    can_frame_t can_frame;
    can_frame.can_id = ctx->tx_id;
    can_frame.dlc = tx_len;
    OSAL_Memcpy(can_frame.data, tx_frame, tx_len);

    if (HAL_CAN_Send(ctx->can_handle, &can_frame) != OS_SUCCESS)
    {
        return OS_ERROR;
    }

    /* 接收响应 */
    OSAL_MutexLock(ctx->rx_mutex);

    can_frame_t rx_frame;
    int32 ret = HAL_CAN_Recv(ctx->can_handle, &rx_frame, timeout_ms);

    if (ret == OS_SUCCESS)
    {
        /* 检查CAN ID */
        if (rx_frame.can_id != ctx->rx_id)
        {
            OSAL_MutexUnlock(ctx->rx_mutex);
            return OS_ERROR;
        }

        /* 解析响应：[status][data_len][data...] */
        if (rx_frame.dlc >= 2)
        {
            uint8 status = rx_frame.data[0];
            uint8 resp_len = rx_frame.data[1];

            if (status != 0)
            {
                OSAL_MutexUnlock(ctx->rx_mutex);
                return OS_ERROR;
            }

            if (response != NULL && resp_len > 0)
            {
                uint32 copy_len = (resp_len < resp_size) ? resp_len : resp_size;
                copy_len = (copy_len < ((uint32)rx_frame.dlc - 2)) ? copy_len : ((uint32)rx_frame.dlc - 2);
                OSAL_Memcpy(response, &rx_frame.data[2], copy_len);

                if (actual_size != NULL)
                {
                    *actual_size = copy_len;
                }
            }
        }
    }

    OSAL_MutexUnlock(ctx->rx_mutex);

    return ret;
}
