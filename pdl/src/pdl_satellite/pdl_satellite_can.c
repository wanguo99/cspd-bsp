/************************************************************************
 * 卫星平台CAN通信实现
 *
 * 职责：
 * - 封装卫星CAN协议（帧格式、消息类型）
 * - CAN帧的收发
 * - 协议解析和封装
 ************************************************************************/

#include "pdl_satellite_internal.h"
#include "hal_can.h"
#include "osal.h"
#include <string.h>

/**
 * @brief 初始化卫星CAN通信
 */
int32 satellite_can_init(const char *device, uint32 bitrate, void **handle)
{
    if (device == NULL || handle == NULL)
    {
        return OS_ERROR;
    }

    /* 打开CAN设备 */
    hal_can_config_t can_config = {
        .interface = device,
        .baudrate = bitrate,
        .rx_timeout = 1000,
        .tx_timeout = 1000
    };

    hal_can_handle_t can_handle;
    if (HAL_CanInit(&can_config, &can_handle) != OS_SUCCESS)
    {
        return OS_ERROR;
    }

    *handle = can_handle;
    return OS_SUCCESS;
}

/**
 * @brief 反初始化卫星CAN通信
 */
int32 satellite_can_deinit(void *handle)
{
    if (handle == NULL)
    {
        return OS_ERROR;
    }

    hal_can_handle_t can_handle = (hal_can_handle_t)handle;
    return HAL_CanDeinit(can_handle);
}

/**
 * @brief 接收卫星CAN消息
 */
int32 satellite_can_recv(void *handle, satellite_can_msg_t *msg, uint32 timeout_ms)
{
    if (handle == NULL || msg == NULL)
    {
        return OS_ERROR;
    }

    hal_can_handle_t can_handle = (hal_can_handle_t)handle;
    can_frame_t frame;

    int32 ret = HAL_CanRecv(can_handle, &frame, timeout_ms);
    if (ret != OS_SUCCESS)
    {
        return ret;
    }

    /* 解析CAN帧：[msg_type][seq_num][cmd_type][data] */
    if (frame.dlc >= 8)
    {
        msg->msg_type = frame.data[0];
        msg->seq_num = frame.data[1];
        msg->cmd_type = frame.data[2];
        msg->data = (frame.data[4] << 24) | (frame.data[5] << 16) |
                    (frame.data[6] << 8) | frame.data[7];
    }
    else
    {
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

/**
 * @brief 发送卫星CAN消息
 */
int32 satellite_can_send(void *handle, const satellite_can_msg_t *msg)
{
    if (handle == NULL || msg == NULL)
    {
        return OS_ERROR;
    }

    hal_can_handle_t can_handle = (hal_can_handle_t)handle;

    /* 封装CAN帧：[msg_type][seq_num][cmd_type][reserved][data] */
    can_frame_t frame;
    frame.can_id = SATELLITE_CAN_TX_ID;
    frame.dlc = 8;

    frame.data[0] = msg->msg_type;
    frame.data[1] = msg->seq_num;
    frame.data[2] = msg->cmd_type;
    frame.data[3] = 0;  /* 预留 */
    frame.data[4] = (uint8)(msg->data >> 24);
    frame.data[5] = (uint8)(msg->data >> 16);
    frame.data[6] = (uint8)(msg->data >> 8);
    frame.data[7] = (uint8)(msg->data & 0xFF);

    return HAL_CanSend(can_handle, &frame);
}

/**
 * @brief 发送心跳消息
 */
int32 satellite_can_send_heartbeat(void *handle, uint8 status)
{
    satellite_can_msg_t msg = {
        .msg_type = CAN_MSG_TYPE_HEARTBEAT,
        .seq_num = 0,
        .cmd_type = status,
        .data = 0
    };

    return satellite_can_send(handle, &msg);
}

/**
 * @brief 发送响应消息
 */
int32 satellite_can_send_response(void *handle, uint8 seq_num, uint8 status, uint32 result)
{
    satellite_can_msg_t msg = {
        .msg_type = CAN_MSG_TYPE_CMD_RESP,
        .seq_num = seq_num,
        .cmd_type = status,
        .data = result
    };

    return satellite_can_send(handle, &msg);
}
