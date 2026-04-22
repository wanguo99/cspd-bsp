/************************************************************************
 * 电源管理服务
 *
 * 功能：
 * - 统一的电源控制接口
 * - 支持多种载荷类型（BMC/Linux）
 * - 电源状态监控和记录
 * - 电源策略管理（定时开关机、节能模式）
 * - 故障保护（过流、过温保护）
 ************************************************************************/

#ifndef SERVICE_POWER_H
#define SERVICE_POWER_H

#include "common_types.h"

/*
 * 电源管理服务句柄
 */
typedef void* power_service_handle_t;

/*
 * 载荷类型
 */
typedef enum
{
    PAYLOAD_TYPE_BMC = 0,      /* 带BMC的载荷 */
    PAYLOAD_TYPE_LINUX = 1,    /* 通用Linux载荷 */
    PAYLOAD_TYPE_GENERIC = 2   /* 通用载荷（GPIO控制） */
} payload_type_t;

/*
 * 电源状态
 */
typedef enum
{
    POWER_STATE_OFF = 0,       /* 关机 */
    POWER_STATE_ON = 1,        /* 开机 */
    POWER_STATE_STANDBY = 2,   /* 待机 */
    POWER_STATE_UNKNOWN = 3    /* 未知 */
} power_state_t;

/*
 * 电源事件类型
 */
typedef enum
{
    POWER_EVENT_ON = 0,        /* 开机事件 */
    POWER_EVENT_OFF = 1,       /* 关机事件 */
    POWER_EVENT_RESET = 2,     /* 复位事件 */
    POWER_EVENT_FAULT = 3      /* 故障事件 */
} power_event_t;

/*
 * 电源配置
 */
typedef struct
{
    payload_type_t type;       /* 载荷类型 */
    void *payload_handle;      /* 载荷服务句柄 */

    /* GPIO配置（用于通用载荷） */
    struct {
        uint32 power_gpio;     /* 电源控制GPIO */
        uint32 reset_gpio;     /* 复位GPIO */
        uint32 status_gpio;    /* 状态检测GPIO */
    } gpio;

    /* 保护参数 */
    struct {
        float max_current_a;   /* 最大电流(A) */
        float max_temp_c;      /* 最大温度(℃) */
        uint32 fault_timeout_ms; /* 故障超时(ms) */
    } protection;

    /* 策略参数 */
    bool auto_recovery;        /* 自动恢复 */
    uint32 recovery_delay_ms;  /* 恢复延迟(ms) */
} power_service_config_t;

/*
 * 电源状态信息
 */
typedef struct
{
    power_state_t state;       /* 当前状态 */
    float voltage_v;           /* 电压(V) */
    float current_a;           /* 电流(A) */
    float power_w;             /* 功率(W) */
    float temp_c;              /* 温度(℃) */
    uint32 uptime_sec;         /* 运行时间(秒) */
    bool fault;                /* 故障标志 */
} power_status_t;

/*
 * 电源事件回调函数
 */
typedef void (*power_event_callback_t)(power_event_t event, void *user_data);

/**
 * @brief 初始化电源管理服务
 *
 * @param[in] config 配置参数
 * @param[out] handle 服务句柄
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 失败
 */
int32 PowerService_Init(const power_service_config_t *config,
                        power_service_handle_t *handle);

/**
 * @brief 反初始化电源管理服务
 *
 * @param[in] handle 服务句柄
 *
 * @return OS_SUCCESS 成功
 */
int32 PowerService_Deinit(power_service_handle_t handle);

/**
 * @brief 电源开机
 *
 * @param[in] handle 服务句柄
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR_TIMEOUT 超时
 * @return OS_ERROR 失败
 */
int32 PowerService_PowerOn(power_service_handle_t handle);

/**
 * @brief 电源关机
 *
 * @param[in] handle 服务句柄
 * @param[in] force 是否强制关机
 *
 * @return OS_SUCCESS 成功
 */
int32 PowerService_PowerOff(power_service_handle_t handle, bool force);

/**
 * @brief 电源复位
 *
 * @param[in] handle 服务句柄
 *
 * @return OS_SUCCESS 成功
 */
int32 PowerService_Reset(power_service_handle_t handle);

/**
 * @brief 进入待机模式
 *
 * @param[in] handle 服务句柄
 *
 * @return OS_SUCCESS 成功
 */
int32 PowerService_Standby(power_service_handle_t handle);

/**
 * @brief 获取电源状态
 *
 * @param[in] handle 服务句柄
 * @param[out] status 状态信息
 *
 * @return OS_SUCCESS 成功
 */
int32 PowerService_GetStatus(power_service_handle_t handle,
                             power_status_t *status);

/**
 * @brief 注册电源事件回调
 *
 * @param[in] handle 服务句柄
 * @param[in] callback 回调函数
 * @param[in] user_data 用户数据
 *
 * @return OS_SUCCESS 成功
 */
int32 PowerService_RegisterCallback(power_service_handle_t handle,
                                    power_event_callback_t callback,
                                    void *user_data);

/**
 * @brief 设置定时开机
 *
 * @param[in] handle 服务句柄
 * @param[in] delay_sec 延迟时间(秒)
 *
 * @return OS_SUCCESS 成功
 */
int32 PowerService_SchedulePowerOn(power_service_handle_t handle,
                                   uint32 delay_sec);

/**
 * @brief 设置定时关机
 *
 * @param[in] handle 服务句柄
 * @param[in] delay_sec 延迟时间(秒)
 *
 * @return OS_SUCCESS 成功
 */
int32 PowerService_SchedulePowerOff(power_service_handle_t handle,
                                    uint32 delay_sec);

/**
 * @brief 取消定时任务
 *
 * @param[in] handle 服务句柄
 *
 * @return OS_SUCCESS 成功
 */
int32 PowerService_CancelSchedule(power_service_handle_t handle);

/**
 * @brief 启用/禁用保护功能
 *
 * @param[in] handle 服务句柄
 * @param[in] enable 是否启用
 *
 * @return OS_SUCCESS 成功
 */
int32 PowerService_EnableProtection(power_service_handle_t handle,
                                    bool enable);

/**
 * @brief 清除故障状态
 *
 * @param[in] handle 服务句柄
 *
 * @return OS_SUCCESS 成功
 */
int32 PowerService_ClearFault(power_service_handle_t handle);

/**
 * @brief 获取服务统计信息
 *
 * @param[in] handle 服务句柄
 * @param[out] on_count 开机次数
 * @param[out] off_count 关机次数
 * @param[out] reset_count 复位次数
 * @param[out] fault_count 故障次数
 *
 * @return OS_SUCCESS 成功
 */
int32 PowerService_GetStats(power_service_handle_t handle,
                            uint32 *on_count,
                            uint32 *off_count,
                            uint32 *reset_count,
                            uint32 *fault_count);

#endif /* SERVICE_POWER_H */
