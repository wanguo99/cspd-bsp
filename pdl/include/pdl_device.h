/************************************************************************
 * 外设设备抽象接口
 *
 * 设计理念：
 * - 管理板为核心，所有外部设备（卫星平台、载荷、BMC、MCU）都是外设
 * - 提供统一的外设接口抽象，便于扩展新的外设类型
 * - 支持多种通信协议（CAN、Ethernet、UART、I2C、SPI）
 * - 统一的电源管理、状态监控、命令执行接口
 ************************************************************************/

#ifndef PERIPHERAL_DEVICE_H
#define PERIPHERAL_DEVICE_H

#include "osal_types.h"

/*
 * 外设类型
 */
typedef enum
{
    PERIPHERAL_TYPE_SATELLITE = 0,    /* 卫星平台（CAN总线） */
    PERIPHERAL_TYPE_PAYLOAD_BMC = 1,  /* BMC载荷（IPMI/Redfish） */
    PERIPHERAL_TYPE_PAYLOAD_OS = 2,/* OS载荷（SSH/网络） */
    PERIPHERAL_TYPE_MCU = 3,          /* MCU外设（I2C/SPI/UART） */
    PERIPHERAL_TYPE_GENERIC = 4       /* 通用外设（GPIO控制） */
} peripheral_type_t;

/*
 * 通信接口类型
 */
typedef enum
{
    COMM_INTERFACE_CAN = 0,
    COMM_INTERFACE_ETHERNET = 1,
    COMM_INTERFACE_UART = 2,
    COMM_INTERFACE_I2C = 3,
    COMM_INTERFACE_SPI = 4,
    COMM_INTERFACE_GPIO = 5
} comm_interface_t;

/*
 * 外设状态
 */
typedef enum
{
    PERIPHERAL_STATE_OFFLINE = 0,     /* 离线 */
    PERIPHERAL_STATE_ONLINE = 1,      /* 在线 */
    PERIPHERAL_STATE_STANDBY = 2,     /* 待机 */
    PERIPHERAL_STATE_FAULT = 3,       /* 故障 */
    PERIPHERAL_STATE_UNKNOWN = 4      /* 未知 */
} peripheral_state_t;

/*
 * 电源状态
 */
typedef enum
{
    PERIPHERAL_POWER_OFF = 0,
    PERIPHERAL_POWER_ON = 1,
    PERIPHERAL_POWER_STANDBY = 2,
    PERIPHERAL_POWER_UNKNOWN = 3
} peripheral_power_state_t;

/*
 * 外设能力标志
 */
typedef enum
{
    PERIPHERAL_CAP_POWER_CONTROL = (1 << 0),  /* 支持电源控制 */
    PERIPHERAL_CAP_RESET = (1 << 1),          /* 支持复位 */
    PERIPHERAL_CAP_STATUS_QUERY = (1 << 2),   /* 支持状态查询 */
    PERIPHERAL_CAP_SENSOR_READ = (1 << 3),    /* 支持传感器读取 */
    PERIPHERAL_CAP_COMMAND_EXEC = (1 << 4),   /* 支持命令执行 */
    PERIPHERAL_CAP_FIRMWARE_UPDATE = (1 << 5),/* 支持固件升级 */
    PERIPHERAL_CAP_HEARTBEAT = (1 << 6)       /* 支持心跳机制 */
} peripheral_capability_t;

/*
 * 外设信息
 */
typedef struct
{
    peripheral_type_t type;           /* 外设类型 */
    char name[64];                    /* 外设名称 */
    char description[128];            /* 描述信息 */
    comm_interface_t interface;       /* 通信接口 */
    uint32 capabilities;              /* 能力标志（位掩码） */
    char firmware_version[32];        /* 固件版本 */
    char hardware_version[32];        /* 硬件版本 */
} peripheral_info_t;

/*
 * 外设状态信息
 */
typedef struct
{
    peripheral_state_t state;         /* 外设状态 */
    peripheral_power_state_t power;   /* 电源状态 */
    uint32 uptime_sec;                /* 运行时间(秒) */
    float temperature;                /* 温度(℃) */
    bool fault;                       /* 故障标志 */
    uint32 error_code;                /* 错误码 */
    char error_msg[128];              /* 错误信息 */
} peripheral_status_t;

/*
 * 外设统计信息
 */
typedef struct
{
    uint32 cmd_count;                 /* 命令总数 */
    uint32 success_count;             /* 成功数 */
    uint32 fail_count;                /* 失败数 */
    uint32 timeout_count;             /* 超时数 */
    uint32 reset_count;               /* 复位次数 */
    uint32 last_error_code;           /* 最后错误码 */
} peripheral_stats_t;

/*
 * 外设句柄
 */
typedef void* peripheral_handle_t;

/*
 * 外设事件类型
 */
typedef enum
{
    PERIPHERAL_EVENT_ONLINE = 0,      /* 上线 */
    PERIPHERAL_EVENT_OFFLINE = 1,     /* 离线 */
    PERIPHERAL_EVENT_FAULT = 2,       /* 故障 */
    PERIPHERAL_EVENT_RECOVERED = 3,   /* 恢复 */
    PERIPHERAL_EVENT_POWER_ON = 4,    /* 上电 */
    PERIPHERAL_EVENT_POWER_OFF = 5,   /* 下电 */
    PERIPHERAL_EVENT_RESET = 6        /* 复位 */
} peripheral_event_t;

/*
 * 外设事件回调函数
 */
typedef void (*peripheral_event_callback_t)(peripheral_handle_t handle,
                                           peripheral_event_t event,
                                           void *user_data);

/*
 * 外设操作接口（虚函数表）
 */
typedef struct
{
    /* 初始化/反初始化 */
    int32 (*init)(void *config, peripheral_handle_t *handle);
    int32 (*deinit)(peripheral_handle_t handle);

    /* 电源控制 */
    int32 (*power_on)(peripheral_handle_t handle);
    int32 (*power_off)(peripheral_handle_t handle);
    int32 (*reset)(peripheral_handle_t handle);

    /* 状态查询 */
    int32 (*get_status)(peripheral_handle_t handle, peripheral_status_t *status);
    int32 (*get_info)(peripheral_handle_t handle, peripheral_info_t *info);

    /* 命令执行 */
    int32 (*execute_command)(peripheral_handle_t handle,
                            const void *cmd,
                            uint32 cmd_len,
                            void *response,
                            uint32 resp_size,
                            uint32 *actual_size);

    /* 数据读写 */
    int32 (*read_data)(peripheral_handle_t handle,
                      uint32 offset,
                      void *buffer,
                      uint32 size,
                      uint32 *actual_size);
    int32 (*write_data)(peripheral_handle_t handle,
                       uint32 offset,
                       const void *buffer,
                       uint32 size);

    /* 事件回调 */
    int32 (*register_callback)(peripheral_handle_t handle,
                              peripheral_event_callback_t callback,
                              void *user_data);

    /* 统计信息 */
    int32 (*get_stats)(peripheral_handle_t handle, peripheral_stats_t *stats);
    int32 (*reset_stats)(peripheral_handle_t handle);
} peripheral_ops_t;

/**
 * @brief 注册外设驱动
 *
 * @param[in] type 外设类型
 * @param[in] ops 操作接口
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 失败
 */
int32 Peripheral_RegisterDriver(peripheral_type_t type, const peripheral_ops_t *ops);

/**
 * @brief 创建外设实例
 *
 * @param[in] type 外设类型
 * @param[in] config 配置参数
 * @param[out] handle 外设句柄
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 失败
 */
int32 Peripheral_Create(peripheral_type_t type, void *config, peripheral_handle_t *handle);

/**
 * @brief 销毁外设实例
 *
 * @param[in] handle 外设句柄
 *
 * @return OS_SUCCESS 成功
 */
int32 Peripheral_Destroy(peripheral_handle_t handle);

/**
 * @brief 外设上电
 *
 * @param[in] handle 外设句柄
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 失败
 */
int32 Peripheral_PowerOn(peripheral_handle_t handle);

/**
 * @brief 外设下电
 *
 * @param[in] handle 外设句柄
 *
 * @return OS_SUCCESS 成功
 */
int32 Peripheral_PowerOff(peripheral_handle_t handle);

/**
 * @brief 外设复位
 *
 * @param[in] handle 外设句柄
 *
 * @return OS_SUCCESS 成功
 */
int32 Peripheral_Reset(peripheral_handle_t handle);

/**
 * @brief 获取外设状态
 *
 * @param[in] handle 外设句柄
 * @param[out] status 状态信息
 *
 * @return OS_SUCCESS 成功
 */
int32 Peripheral_GetStatus(peripheral_handle_t handle, peripheral_status_t *status);

/**
 * @brief 获取外设信息
 *
 * @param[in] handle 外设句柄
 * @param[out] info 外设信息
 *
 * @return OS_SUCCESS 成功
 */
int32 Peripheral_GetInfo(peripheral_handle_t handle, peripheral_info_t *info);

/**
 * @brief 执行外设命令
 *
 * @param[in] handle 外设句柄
 * @param[in] cmd 命令数据
 * @param[in] cmd_len 命令长度
 * @param[out] response 响应缓冲区
 * @param[in] resp_size 缓冲区大小
 * @param[out] actual_size 实际响应长度
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR_TIMEOUT 超时
 * @return OS_ERROR 失败
 */
int32 Peripheral_ExecuteCommand(peripheral_handle_t handle,
                               const void *cmd,
                               uint32 cmd_len,
                               void *response,
                               uint32 resp_size,
                               uint32 *actual_size);

/**
 * @brief 读取外设数据
 *
 * @param[in] handle 外设句柄
 * @param[in] offset 偏移地址
 * @param[out] buffer 数据缓冲区
 * @param[in] size 读取大小
 * @param[out] actual_size 实际读取大小
 *
 * @return OS_SUCCESS 成功
 */
int32 Peripheral_ReadData(peripheral_handle_t handle,
                         uint32 offset,
                         void *buffer,
                         uint32 size,
                         uint32 *actual_size);

/**
 * @brief 写入外设数据
 *
 * @param[in] handle 外设句柄
 * @param[in] offset 偏移地址
 * @param[in] buffer 数据缓冲区
 * @param[in] size 写入大小
 *
 * @return OS_SUCCESS 成功
 */
int32 Peripheral_WriteData(peripheral_handle_t handle,
                          uint32 offset,
                          const void *buffer,
                          uint32 size);

/**
 * @brief 注册事件回调
 *
 * @param[in] handle 外设句柄
 * @param[in] callback 回调函数
 * @param[in] user_data 用户数据
 *
 * @return OS_SUCCESS 成功
 */
int32 Peripheral_RegisterCallback(peripheral_handle_t handle,
                                 peripheral_event_callback_t callback,
                                 void *user_data);

/**
 * @brief 获取统计信息
 *
 * @param[in] handle 外设句柄
 * @param[out] stats 统计信息
 *
 * @return OS_SUCCESS 成功
 */
int32 Peripheral_GetStats(peripheral_handle_t handle, peripheral_stats_t *stats);

/**
 * @brief 重置统计信息
 *
 * @param[in] handle 外设句柄
 *
 * @return OS_SUCCESS 成功
 */
int32 Peripheral_ResetStats(peripheral_handle_t handle);

#endif /* PERIPHERAL_DEVICE_H */
