/************************************************************************
 * 软件看门狗和故障恢复机制
 *
 * 功能：
 * 1. 任务心跳监控
 * 2. 系统健康检查
 * 3. 自动故障恢复
 * 4. 硬件看门狗喂狗
 ************************************************************************/

#ifndef WATCHDOG_H
#define WATCHDOG_H

#include "common_types.h"

/*
 * 看门狗配置
 */
typedef struct
{
    uint32 check_interval_ms;      /* 检查间隔(ms) */
    uint32 task_timeout_ms;        /* 任务超时时间(ms) */
    bool   enable_hw_watchdog;     /* 启用硬件看门狗 */
    const char *hw_watchdog_dev;   /* 硬件看门狗设备 */
    uint32 hw_watchdog_timeout_s;  /* 硬件看门狗超时(s) */
} watchdog_config_t;

/*
 * 任务健康状态
 */
typedef enum
{
    TASK_HEALTH_UNKNOWN = 0,
    TASK_HEALTH_OK,
    TASK_HEALTH_WARNING,
    TASK_HEALTH_CRITICAL
} task_health_t;

/**
 * @brief 初始化看门狗系统
 *
 * @param[in] config 配置参数
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 失败
 */
int32 Watchdog_Init(const watchdog_config_t *config);

/**
 * @brief 关闭看门狗系统
 *
 * @return OS_SUCCESS 成功
 */
int32 Watchdog_Deinit(void);

/**
 * @brief 注册需要监控的任务
 *
 * @param[in] task_id 任务ID
 * @param[in] task_name 任务名称
 * @param[in] timeout_ms 超时时间(ms)
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 失败
 */
int32 Watchdog_RegisterTask(osal_id_t task_id, const char *task_name, uint32 timeout_ms);

/**
 * @brief 任务心跳上报
 *
 * @param[in] task_id 任务ID
 *
 * @return OS_SUCCESS 成功
 */
int32 Watchdog_Heartbeat(osal_id_t task_id);

/**
 * @brief 获取任务健康状态
 *
 * @param[in] task_id 任务ID
 * @param[out] health 健康状态
 *
 * @return OS_SUCCESS 成功
 */
int32 Watchdog_GetTaskHealth(osal_id_t task_id, task_health_t *health);

/**
 * @brief 获取系统健康状态
 *
 * @return task_health_t 系统健康状态
 */
task_health_t Watchdog_GetSystemHealth(void);

#endif /* WATCHDOG_H */
