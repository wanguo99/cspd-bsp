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

#include "osa_types.h"

typedef void* power_service_handle_t;

typedef enum
{
    PAYLOAD_TYPE_BMC = 0,
    PAYLOAD_TYPE_LINUX = 1,
    PAYLOAD_TYPE_GENERIC = 2   /* GPIO控制 */
} payload_type_t;

typedef enum
{
    POWER_STATE_OFF = 0,
    POWER_STATE_ON = 1,
    POWER_STATE_STANDBY = 2,
    POWER_STATE_UNKNOWN = 3
} power_state_t;

typedef enum
{
    POWER_EVENT_ON = 0,
    POWER_EVENT_OFF = 1,
    POWER_EVENT_RESET = 2,
    POWER_EVENT_FAULT = 3
} power_event_t;

typedef struct
{
    payload_type_t type;
    void *payload_handle;

    struct {
        uint32 power_gpio;
        uint32 reset_gpio;
        uint32 status_gpio;
    } gpio;

    struct {
        float max_current_a;
        float max_temp_c;
        uint32 fault_timeout_ms;
    } protection;

    bool auto_recovery;
    uint32 recovery_delay_ms;
} power_service_config_t;

typedef struct
{
    power_state_t state;
    float voltage_v;
    float current_a;
    float power_w;
    float temp_c;
    uint32 uptime_sec;
    bool fault;
} power_status_t;

typedef void (*power_event_callback_t)(power_event_t event, void *user_data);

/**
 * @brief 初始化电源管理服务
 */
int32 PowerService_Init(const power_service_config_t *config,
                        power_service_handle_t *handle);

/**
 * @brief 反初始化电源管理服务
 */
int32 PowerService_Deinit(power_service_handle_t handle);

/**
 * @brief 电源开机
 */
int32 PowerService_PowerOn(power_service_handle_t handle);

/**
 * @brief 电源关机
 *
 * @param[in] force 是否强制关机
 */
int32 PowerService_PowerOff(power_service_handle_t handle, bool force);

/**
 * @brief 电源复位
 */
int32 PowerService_Reset(power_service_handle_t handle);

/**
 * @brief 进入待机模式
 */
int32 PowerService_Standby(power_service_handle_t handle);

/**
 * @brief 获取电源状态
 */
int32 PowerService_GetStatus(power_service_handle_t handle,
                             power_status_t *status);

/**
 * @brief 注册电源事件回调
 */
int32 PowerService_RegisterCallback(power_service_handle_t handle,
                                    power_event_callback_t callback,
                                    void *user_data);

/**
 * @brief 设置定时开机
 *
 * @param[in] delay_sec 延迟时间(秒)
 */
int32 PowerService_SchedulePowerOn(power_service_handle_t handle,
                                   uint32 delay_sec);

/**
 * @brief 设置定时关机
 *
 * @param[in] delay_sec 延迟时间(秒)
 */
int32 PowerService_SchedulePowerOff(power_service_handle_t handle,
                                    uint32 delay_sec);

/**
 * @brief 取消定时任务
 */
int32 PowerService_CancelSchedule(power_service_handle_t handle);

/**
 * @brief 启用/禁用保护功能
 */
int32 PowerService_EnableProtection(power_service_handle_t handle,
                                    bool enable);

/**
 * @brief 清除故障状态
 */
int32 PowerService_ClearFault(power_service_handle_t handle);

/**
 * @brief 获取服务统计信息
 */
int32 PowerService_GetStats(power_service_handle_t handle,
                            uint32 *on_count,
                            uint32 *off_count,
                            uint32 *reset_count,
                            uint32 *fault_count);

#endif /* SERVICE_POWER_H */
