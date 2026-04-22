/************************************************************************
 * CAN协议定义
 *
 * 定义卫星平台与管理板之间的CAN通信协议
 ************************************************************************/

#ifndef CAN_PROTOCOL_H
#define CAN_PROTOCOL_H

#include "common_types.h"

/*
 * ============================================================================
 * CAN消息格式
 * ============================================================================
 */

/* CAN标准帧ID定义 */
#define CAN_ID_SAT_TO_BRIDGE    0x100  /* 卫星平台 -> 管理板 */
#define CAN_ID_BRIDGE_TO_SAT    0x200  /* 管理板 -> 卫星平台 */

/* CAN消息类型 */
typedef enum
{
    CAN_MSG_TYPE_CMD_REQ        = 0x01,  /* 命令请求 */
    CAN_MSG_TYPE_CMD_RESP       = 0x02,  /* 命令响应 */
    CAN_MSG_TYPE_STATUS_QUERY   = 0x03,  /* 状态查询 */
    CAN_MSG_TYPE_STATUS_REPORT  = 0x04,  /* 状态上报 */
    CAN_MSG_TYPE_HEARTBEAT      = 0x05,  /* 心跳 */
    CAN_MSG_TYPE_ERROR          = 0x06,  /* 错误报告 */
} can_msg_type_t;

/* 命令类型 */
typedef enum
{
    CMD_TYPE_POWER_ON           = 0x10,  /* 载荷上电 */
    CMD_TYPE_POWER_OFF          = 0x11,  /* 载荷下电 */
    CMD_TYPE_RESET              = 0x12,  /* 载荷重启 */
    CMD_TYPE_QUERY_STATUS       = 0x20,  /* 查询状态 */
    CMD_TYPE_QUERY_TEMP         = 0x21,  /* 查询温度 */
    CMD_TYPE_QUERY_VOLTAGE      = 0x22,  /* 查询电压 */
    CMD_TYPE_SET_CONFIG         = 0x30,  /* 设置配置 */
    CMD_TYPE_GET_CONFIG         = 0x31,  /* 获取配置 */
} can_cmd_type_t;

/* 状态码 */
typedef enum
{
    STATUS_OK                   = 0x00,  /* 成功 */
    STATUS_ERROR                = 0x01,  /* 错误 */
    STATUS_TIMEOUT              = 0x02,  /* 超时 */
    STATUS_INVALID_CMD          = 0x03,  /* 无效命令 */
    STATUS_PAYLOAD_OFFLINE      = 0x04,  /* 载荷离线 */
    STATUS_COMM_ERROR           = 0x05,  /* 通信错误 */
} can_status_t;

/*
 * CAN消息结构 (8字节)
 *
 * Byte 0: 消息类型
 * Byte 1: 命令类型/状态码
 * Byte 2-3: 序列号
 * Byte 4-7: 数据/参数
 */
typedef struct
{
    uint8  msg_type;      /* 消息类型 */
    uint8  cmd_type;      /* 命令类型或状态码 */
    uint16 seq_num;       /* 序列号 */
    uint32 data;          /* 数据或参数 */
} __attribute__((packed)) can_msg_t;

/*
 * ============================================================================
 * CAN消息帧
 * ============================================================================
 */

typedef struct
{
    uint32     can_id;    /* CAN ID */
    uint8      dlc;       /* 数据长度 */
    can_msg_t  msg;       /* 消息内容 */
    uint32     timestamp; /* 时间戳(ms) */
} can_frame_t;

/*
 * ============================================================================
 * 辅助函数
 * ============================================================================
 */

/**
 * @brief 构造CAN命令请求
 */
static inline void can_build_cmd_request(can_frame_t *frame,
                                         can_cmd_type_t cmd_type,
                                         uint16 seq_num,
                                         uint32 param)
{
    frame->can_id = CAN_ID_BRIDGE_TO_SAT;
    frame->dlc = 8;
    frame->msg.msg_type = CAN_MSG_TYPE_CMD_REQ;
    frame->msg.cmd_type = cmd_type;
    frame->msg.seq_num = seq_num;
    frame->msg.data = param;
}

/**
 * @brief 构造CAN命令响应
 */
static inline void can_build_cmd_response(can_frame_t *frame,
                                          uint16 seq_num,
                                          can_status_t status,
                                          uint32 result)
{
    frame->can_id = CAN_ID_BRIDGE_TO_SAT;
    frame->dlc = 8;
    frame->msg.msg_type = CAN_MSG_TYPE_CMD_RESP;
    frame->msg.cmd_type = status;
    frame->msg.seq_num = seq_num;
    frame->msg.data = result;
}

/**
 * @brief 构造状态上报
 */
static inline void can_build_status_report(can_frame_t *frame,
                                           uint16 seq_num,
                                           uint32 status_data)
{
    frame->can_id = CAN_ID_BRIDGE_TO_SAT;
    frame->dlc = 8;
    frame->msg.msg_type = CAN_MSG_TYPE_STATUS_REPORT;
    frame->msg.cmd_type = STATUS_OK;
    frame->msg.seq_num = seq_num;
    frame->msg.data = status_data;
}

/**
 * @brief 构造心跳消息
 */
static inline void can_build_heartbeat(can_frame_t *frame, uint32 uptime_sec)
{
    frame->can_id = CAN_ID_BRIDGE_TO_SAT;
    frame->dlc = 8;
    frame->msg.msg_type = CAN_MSG_TYPE_HEARTBEAT;
    frame->msg.cmd_type = STATUS_OK;
    frame->msg.seq_num = 0;
    frame->msg.data = uptime_sec;
}

/**
 * @brief 解析CAN消息类型
 */
static inline const char* can_get_msg_type_name(can_msg_type_t type)
{
    switch (type)
    {
        case CAN_MSG_TYPE_CMD_REQ:       return "CMD_REQ";
        case CAN_MSG_TYPE_CMD_RESP:      return "CMD_RESP";
        case CAN_MSG_TYPE_STATUS_QUERY:  return "STATUS_QUERY";
        case CAN_MSG_TYPE_STATUS_REPORT: return "STATUS_REPORT";
        case CAN_MSG_TYPE_HEARTBEAT:     return "HEARTBEAT";
        case CAN_MSG_TYPE_ERROR:         return "ERROR";
        default:                         return "UNKNOWN";
    }
}

/**
 * @brief 解析命令类型
 */
static inline const char* can_get_cmd_type_name(can_cmd_type_t type)
{
    switch (type)
    {
        case CMD_TYPE_POWER_ON:      return "POWER_ON";
        case CMD_TYPE_POWER_OFF:     return "POWER_OFF";
        case CMD_TYPE_RESET:         return "RESET";
        case CMD_TYPE_QUERY_STATUS:  return "QUERY_STATUS";
        case CMD_TYPE_QUERY_TEMP:    return "QUERY_TEMP";
        case CMD_TYPE_QUERY_VOLTAGE: return "QUERY_VOLTAGE";
        case CMD_TYPE_SET_CONFIG:    return "SET_CONFIG";
        case CMD_TYPE_GET_CONFIG:    return "GET_CONFIG";
        default:                     return "UNKNOWN";
    }
}

#endif /* CAN_PROTOCOL_H */
