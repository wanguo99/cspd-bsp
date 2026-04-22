/************************************************************************
 * BMC载荷通信服务
 *
 * 功能：
 * - 与带BMC的载荷通信（IPMI/Redfish协议）
 * - 支持多通道（网络/串口）自动切换
 * - 电源控制、状态查询、传感器读取
 * - 故障检测和自动恢复
 ************************************************************************/

#ifndef SERVICE_PAYLOAD_BMC_H
#define SERVICE_PAYLOAD_BMC_H

#include "common_types.h"

/*
 * BMC载荷服务句柄
 */
typedef void* bmc_payload_handle_t;

/*
 * 通信通道类型
 */
typedef enum
{
    BMC_CHANNEL_NETWORK = 0,  /* 网络通道（IPMI over LAN） */
    BMC_CHANNEL_SERIAL  = 1   /* 串口通道（IPMI over Serial） */
} bmc_channel_t;

/*
 * BMC协议类型
 */
typedef enum
{
    BMC_PROTOCOL_IPMI = 0,    /* IPMI协议 */
    BMC_PROTOCOL_REDFISH = 1  /* Redfish协议 */
} bmc_protocol_t;

/*
 * 电源状态
 */
typedef enum
{
    BMC_POWER_OFF = 0,
    BMC_POWER_ON  = 1,
    BMC_POWER_UNKNOWN = 2
} bmc_power_state_t;

/*
 * BMC载荷状态
 */
typedef struct
{
    bmc_power_state_t power_state;
    bool bmc_ready;
    uint32 uptime_sec;
    float cpu_temp;
    float inlet_temp;
} bmc_payload_status_t;

/*
 * BMC载荷配置
 */
typedef struct
{
    /* 网络配置 */
    struct {
        bool enabled;             /* 是否启用 */
        const char *ip_addr;      /* IP地址 */
        uint16 port;              /* 端口（默认623） */
        const char *username;     /* 用户名 */
        const char *password;     /* 密码 */
        uint32 timeout_ms;        /* 超时时间 */
    } network;

    /* 串口配置 */
    struct {
        bool enabled;             /* 是否启用 */
        const char *device;       /* 串口设备 */
        uint32 baudrate;          /* 波特率 */
        uint32 timeout_ms;        /* 超时时间 */
    } serial;

    /* 服务配置 */
    bmc_channel_t primary_channel;  /* 主通道 */
    bool auto_switch;             /* 自动切换通道 */
    uint32 retry_count;           /* 重试次数 */
    uint32 health_check_interval; /* 健康检查间隔(ms) */
} bmc_payload_config_t;

/*
 * 传感器类型
 */
typedef enum
{
    BMC_SENSOR_TEMP = 0,      /* 温度 */
    BMC_SENSOR_VOLTAGE = 1,   /* 电压 */
    BMC_SENSOR_CURRENT = 2,   /* 电流 */
    BMC_SENSOR_FAN = 3        /* 风扇转速 */
} bmc_sensor_type_t;

/*
 * 传感器读数
 */
typedef struct
{
    bmc_sensor_type_t type;
    char name[64];
    float value;
    char unit[16];
    bool valid;
} bmc_sensor_reading_t;

/**
 * @brief 初始化BMC载荷服务
 *
 * @param[in] config 配置参数
 * @param[out] handle 服务句柄
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 失败
 */
int32 BMCPayload_Init(const bmc_payload_config_t *config,
                      bmc_payload_handle_t *handle);

/**
 * @brief 反初始化BMC载荷服务
 *
 * @param[in] handle 服务句柄
 *
 * @return OS_SUCCESS 成功
 */
int32 BMCPayload_Deinit(bmc_payload_handle_t handle);

/**
 * @brief 电源开机
 *
 * @param[in] handle 服务句柄
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR_TIMEOUT 超时
 * @return OS_ERROR 失败
 */
int32 BMCPayload_PowerOn(bmc_payload_handle_t handle);

/**
 * @brief 电源关机
 *
 * @param[in] handle 服务句柄
 *
 * @return OS_SUCCESS 成功
 */
int32 BMCPayload_PowerOff(bmc_payload_handle_t handle);

/**
 * @brief 电源复位
 *
 * @param[in] handle 服务句柄
 *
 * @return OS_SUCCESS 成功
 */
int32 BMCPayload_PowerReset(bmc_payload_handle_t handle);

/**
 * @brief 查询电源状态
 *
 * @param[in] handle 服务句柄
 * @param[out] state 电源状态
 *
 * @return OS_SUCCESS 成功
 */
int32 BMCPayload_GetPowerState(bmc_payload_handle_t handle,
                               bmc_power_state_t *state);

/**
 * @brief 读取传感器
 *
 * @param[in] handle 服务句柄
 * @param[in] type 传感器类型
 * @param[out] readings 读数数组
 * @param[in] max_count 数组大小
 * @param[out] actual_count 实际读取数量
 *
 * @return OS_SUCCESS 成功
 */
int32 BMCPayload_ReadSensors(bmc_payload_handle_t handle,
                             bmc_sensor_type_t type,
                             bmc_sensor_reading_t *readings,
                             uint32 max_count,
                             uint32 *actual_count);

/**
 * @brief 执行原始IPMI命令
 *
 * @param[in] handle 服务句柄
 * @param[in] cmd 命令字符串
 * @param[out] response 响应缓冲区
 * @param[in] resp_size 缓冲区大小
 *
 * @return 实际接收字节数
 * @return <0 错误码
 */
int32 BMCPayload_ExecuteCommand(bmc_payload_handle_t handle,
                                const char *cmd,
                                char *response,
                                uint32 resp_size);

/**
 * @brief 切换通信通道
 *
 * @param[in] handle 服务句柄
 * @param[in] channel 目标通道
 *
 * @return OS_SUCCESS 成功
 */
int32 BMCPayload_SwitchChannel(bmc_payload_handle_t handle,
                               bmc_channel_t channel);

/**
 * @brief 获取当前通道
 *
 * @param[in] handle 服务句柄
 *
 * @return bmc_channel_t 当前通道
 */
bmc_channel_t BMCPayload_GetChannel(bmc_payload_handle_t handle);

/**
 * @brief 检查连接状态
 *
 * @param[in] handle 服务句柄
 *
 * @return true 已连接
 * @return false 未连接
 */
bool BMCPayload_IsConnected(bmc_payload_handle_t handle);

/**
 * @brief 获取服务统计信息
 *
 * @param[in] handle 服务句柄
 * @param[out] cmd_count 命令总数
 * @param[out] success_count 成功数
 * @param[out] fail_count 失败数
 * @param[out] switch_count 通道切换次数
 *
 * @return OS_SUCCESS 成功
 */
int32 BMCPayload_GetStats(bmc_payload_handle_t handle,
                         uint32 *cmd_count,
                         uint32 *success_count,
                         uint32 *fail_count,
                         uint32 *switch_count);

#endif /* SERVICE_PAYLOAD_BMC_H */
