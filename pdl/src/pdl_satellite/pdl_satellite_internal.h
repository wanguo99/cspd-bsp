/************************************************************************
 * 卫星平台服务内部接口
 *
 * 仅供pdl_satellite模块内部使用，不对外暴露
 ************************************************************************/

#ifndef PDL_SATELLITE_INTERNAL_H
#define PDL_SATELLITE_INTERNAL_H

#include "osal_types.h"

/*
 * 卫星CAN ID定义
 */
#define SATELLITE_CAN_RX_ID   0x100   /* 接收ID（卫星→管理板） */
#define SATELLITE_CAN_TX_ID   0x101   /* 发送ID（管理板→卫星） */

/*
 * CAN消息类型
 */
#define CAN_MSG_TYPE_CMD_REQ      0x01   /* 命令请求 */
#define CAN_MSG_TYPE_CMD_RESP     0x02   /* 命令响应 */
#define CAN_MSG_TYPE_HEARTBEAT    0x03   /* 心跳 */

/*
 * 卫星CAN消息结构
 */
typedef struct
{
    uint8 msg_type;      /* 消息类型 */
    uint8 seq_num;       /* 序列号 */
    uint8 cmd_type;      /* 命令类型/状态码 */
    uint32 data;         /* 数据 */
} satellite_can_msg_t;

/*
 * CAN通信接口（pdl_satellite_can.c实现）
 */
int32 satellite_can_init(const char *device, uint32 bitrate, void **handle);
int32 satellite_can_deinit(void *handle);
int32 satellite_can_recv(void *handle, satellite_can_msg_t *msg, uint32 timeout_ms);
int32 satellite_can_send(void *handle, const satellite_can_msg_t *msg);
int32 satellite_can_send_heartbeat(void *handle, uint8 status);
int32 satellite_can_send_response(void *handle, uint8 seq_num, uint8 status, uint32 result);

#endif /* PDL_SATELLITE_INTERNAL_H */
