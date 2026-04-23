/************************************************************************
 * 电源管理服务 - Linux实现
 ************************************************************************/

#include "service_power.h"
#include "service_payload_bmc.h"
#include "service_payload_linux.h"
#include "osal.h"
#include "system_config.h"
#include <stdlib.h>
#include <string.h>

/*
 * 电源管理服务上下文
 */
typedef struct
{
    power_service_config_t config;

    /* 载荷句柄 */
    void *payload_handle;

    /* 当前状态 */
    power_state_t current_state;
    bool fault;

    /* 事件回调 */
    power_event_callback_t callback;
    void *user_data;

    /* 统计信息 */
    uint32 on_count;
    uint32 off_count;
    uint32 reset_count;
    uint32 fault_count;

    /* 定时任务 */
    osal_id_t timer_task_id;
    uint32 scheduled_action;  /* 0=无, 1=开机, 2=关机 */
    uint32 scheduled_delay;

    /* 互斥锁 */
    osal_id_t mutex;
} power_service_context_t;

/*
 * 触发电源事件
 */
static void trigger_event(power_service_context_t *ctx, power_event_t event)
{
    if (ctx->callback != NULL) {
        ctx->callback(event, ctx->user_data);
    }
}

/*
 * 定时任务
 */
static void timer_task(void *arg)
{
    power_service_context_t *ctx = (power_service_context_t *)arg;
    bool running = true;

    while (running) {
        OS_MutexLock(ctx->mutex);
        running = (ctx->scheduled_action != 0);
        OS_MutexUnlock(ctx->mutex);

        if (!running)
            break;
        OS_TaskDelay(1000);  /* 1秒 */

        OS_MutexLock(ctx->mutex);

        if (ctx->scheduled_delay > 0) {
            ctx->scheduled_delay--;
        }

        if (ctx->scheduled_delay == 0 && ctx->scheduled_action != 0) {
            /* 执行定时动作 */
            if (ctx->scheduled_action == 1) {
                LOG_INFO("SVC_PWR", "Scheduled power on");
                PowerService_PowerOn((power_service_handle_t)ctx);
            } else if (ctx->scheduled_action == 2) {
                LOG_INFO("SVC_PWR", "Scheduled power off");
                PowerService_PowerOff((power_service_handle_t)ctx, false);
            }

            ctx->scheduled_action = 0;
        }

        OS_MutexUnlock(ctx->mutex);
    }
}

/*
 * 初始化电源管理服务
 */
int32 PowerService_Init(const power_service_config_t *config,
                        power_service_handle_t *handle)
{
    if (config == NULL || handle == NULL) {
        return OS_INVALID_POINTER;
    }

    /* 分配上下文 */
    power_service_context_t *ctx = (power_service_context_t *)malloc(sizeof(power_service_context_t));
    if (ctx == NULL) {
        LOG_ERROR("SVC_PWR", "Failed to allocate power service context");
        return OS_ERROR;
    }

    memset(ctx, 0, sizeof(power_service_context_t));
    memcpy(&ctx->config, config, sizeof(power_service_config_t));
    ctx->payload_handle = config->payload_handle;
    ctx->current_state = POWER_STATE_UNKNOWN;

    /* 创建互斥锁 */
    int32 ret = OS_MutexCreate(&ctx->mutex, "PWR_MTX", 0);
    if (ret != OS_SUCCESS) {
        LOG_ERROR("SVC_PWR", "Failed to create mutex");
        free(ctx);
        return ret;
    }

    *handle = (power_service_handle_t)ctx;
    LOG_INFO("SVC_PWR", "Power service initialized");

    return OS_SUCCESS;
}

/*
 * 反初始化电源管理服务
 */
int32 PowerService_Deinit(power_service_handle_t handle)
{
    if (handle == NULL) {
        return OS_INVALID_POINTER;
    }

    power_service_context_t *ctx = (power_service_context_t *)handle;

    /* 取消定时任务 */
    PowerService_CancelSchedule(handle);

    OS_MutexDelete(ctx->mutex);
    free(ctx);

    LOG_INFO("SVC_PWR", "Power service deinitialized");

    return OS_SUCCESS;
}

/*
 * 电源开机
 */
int32 PowerService_PowerOn(power_service_handle_t handle)
{
    if (handle == NULL) {
        return OS_INVALID_POINTER;
    }

    power_service_context_t *ctx = (power_service_context_t *)handle;
    int32 ret = OS_ERROR;

    OS_MutexLock(ctx->mutex);

    LOG_INFO("SVC_PWR", "Power on requested");

    /* 根据载荷类型调用相应接口 */
    if (ctx->config.type == PAYLOAD_TYPE_BMC) {
        ret = BMCPayload_PowerOn((bmc_payload_handle_t)ctx->payload_handle);
    } else if (ctx->config.type == PAYLOAD_TYPE_LINUX) {
        /* Linux载荷通常没有独立电源控制 */
        LOG_WARN("SVC_PWR", "Linux payload does not support power control");
        ret = OS_ERR_NOT_IMPLEMENTED;
    } else {
        /* TODO: GPIO控制 */
        LOG_WARN("SVC_PWR", "Generic payload power control not implemented");
        ret = OS_ERR_NOT_IMPLEMENTED;
    }

    if (ret == OS_SUCCESS) {
        ctx->current_state = POWER_STATE_ON;
        ctx->on_count++;
        trigger_event(ctx, POWER_EVENT_ON);
    }

    OS_MutexUnlock(ctx->mutex);

    return ret;
}

/*
 * 电源关机
 */
int32 PowerService_PowerOff(power_service_handle_t handle, bool force)
{
    if (handle == NULL) {
        return OS_INVALID_POINTER;
    }

    power_service_context_t *ctx = (power_service_context_t *)handle;
    int32 ret = OS_ERROR;

    OS_MutexLock(ctx->mutex);

    LOG_INFO("SVC_PWR", "Power off requested (force=%d)", force);

    /* 根据载荷类型调用相应接口 */
    if (ctx->config.type == PAYLOAD_TYPE_BMC) {
        ret = BMCPayload_PowerOff((bmc_payload_handle_t)ctx->payload_handle);
    } else if (ctx->config.type == PAYLOAD_TYPE_LINUX) {
        if (force) {
            /* 强制关机 */
            ret = LinuxPayload_Shutdown((linux_payload_handle_t)ctx->payload_handle);
        } else {
            /* 优雅关机 */
            ret = LinuxPayload_Shutdown((linux_payload_handle_t)ctx->payload_handle);
        }
    } else {
        /* TODO: GPIO控制 */
        ret = OS_ERR_NOT_IMPLEMENTED;
    }

    if (ret == OS_SUCCESS) {
        ctx->current_state = POWER_STATE_OFF;
        ctx->off_count++;
        trigger_event(ctx, POWER_EVENT_OFF);
    }

    OS_MutexUnlock(ctx->mutex);

    return ret;
}

/*
 * 电源复位
 */
int32 PowerService_Reset(power_service_handle_t handle)
{
    if (handle == NULL) {
        return OS_INVALID_POINTER;
    }

    power_service_context_t *ctx = (power_service_context_t *)handle;
    int32 ret = OS_ERROR;

    OS_MutexLock(ctx->mutex);

    LOG_INFO("SVC_PWR", "Power reset requested");

    /* 根据载荷类型调用相应接口 */
    if (ctx->config.type == PAYLOAD_TYPE_BMC) {
        ret = BMCPayload_PowerReset((bmc_payload_handle_t)ctx->payload_handle);
    } else if (ctx->config.type == PAYLOAD_TYPE_LINUX) {
        ret = LinuxPayload_Reboot((linux_payload_handle_t)ctx->payload_handle);
    } else {
        /* TODO: GPIO控制 */
        ret = OS_ERR_NOT_IMPLEMENTED;
    }

    if (ret == OS_SUCCESS) {
        ctx->reset_count++;
        trigger_event(ctx, POWER_EVENT_RESET);
    }

    OS_MutexUnlock(ctx->mutex);

    return ret;
}

/*
 * 进入待机模式
 */
int32 PowerService_Standby(power_service_handle_t handle)
{
    if (handle == NULL) {
        return OS_INVALID_POINTER;
    }

    /* TODO: 实现待机模式 */
    LOG_WARN("SVC_PWR", "PowerService_Standby not implemented yet");

    return OS_ERR_NOT_IMPLEMENTED;
}

/*
 * 获取电源状态
 */
int32 PowerService_GetStatus(power_service_handle_t handle,
                             power_status_t *status)
{
    if (handle == NULL || status == NULL) {
        return OS_INVALID_POINTER;
    }

    power_service_context_t *ctx = (power_service_context_t *)handle;

    OS_MutexLock(ctx->mutex);

    status->state = ctx->current_state;
    status->fault = ctx->fault;

    /* TODO: 读取电压、电流、温度等传感器数据 */
    status->voltage_v = 0.0f;
    status->current_a = 0.0f;
    status->power_w = 0.0f;
    status->temp_c = 0.0f;
    status->uptime_sec = 0;

    OS_MutexUnlock(ctx->mutex);

    return OS_SUCCESS;
}

/*
 * 注册电源事件回调
 */
int32 PowerService_RegisterCallback(power_service_handle_t handle,
                                    power_event_callback_t callback,
                                    void *user_data)
{
    if (handle == NULL) {
        return OS_INVALID_POINTER;
    }

    power_service_context_t *ctx = (power_service_context_t *)handle;

    OS_MutexLock(ctx->mutex);
    ctx->callback = callback;
    ctx->user_data = user_data;
    OS_MutexUnlock(ctx->mutex);

    return OS_SUCCESS;
}

/*
 * 设置定时开机
 */
int32 PowerService_SchedulePowerOn(power_service_handle_t handle,
                                   uint32 delay_sec)
{
    if (handle == NULL) {
        return OS_INVALID_POINTER;
    }

    power_service_context_t *ctx = (power_service_context_t *)handle;

    OS_MutexLock(ctx->mutex);

    ctx->scheduled_action = 1;
    ctx->scheduled_delay = delay_sec;

    /* 创建定时任务 */
    if (ctx->timer_task_id == 0) {
        int32 ret = OS_TaskCreate(&ctx->timer_task_id, "PWR_TIMER",
                                  timer_task, (uint32*)ctx,
                                  TASK_STACK_SIZE_SMALL,
                                  PRIORITY_LOW, 0);
        if (ret != OS_SUCCESS) {
            LOG_ERROR("SVC_PWR", "Failed to create timer task");
            OS_MutexUnlock(ctx->mutex);
            return ret;
        }
    }

    LOG_INFO("SVC_PWR", "Scheduled power on in %u seconds", delay_sec);

    OS_MutexUnlock(ctx->mutex);

    return OS_SUCCESS;
}

/*
 * 设置定时关机
 */
int32 PowerService_SchedulePowerOff(power_service_handle_t handle,
                                    uint32 delay_sec)
{
    if (handle == NULL) {
        return OS_INVALID_POINTER;
    }

    power_service_context_t *ctx = (power_service_context_t *)handle;

    OS_MutexLock(ctx->mutex);

    ctx->scheduled_action = 2;
    ctx->scheduled_delay = delay_sec;

    /* 创建定时任务 */
    if (ctx->timer_task_id == 0) {
        int32 ret = OS_TaskCreate(&ctx->timer_task_id, "PWR_TIMER",
                                  timer_task, (uint32*)ctx,
                                  TASK_STACK_SIZE_SMALL,
                                  PRIORITY_LOW, 0);
        if (ret != OS_SUCCESS) {
            LOG_ERROR("SVC_PWR", "Failed to create timer task");
            OS_MutexUnlock(ctx->mutex);
            return ret;
        }
    }

    LOG_INFO("SVC_PWR", "Scheduled power off in %u seconds", delay_sec);

    OS_MutexUnlock(ctx->mutex);

    return OS_SUCCESS;
}

/*
 * 取消定时任务
 */
int32 PowerService_CancelSchedule(power_service_handle_t handle)
{
    if (handle == NULL) {
        return OS_INVALID_POINTER;
    }

    power_service_context_t *ctx = (power_service_context_t *)handle;

    OS_MutexLock(ctx->mutex);

    ctx->scheduled_action = 0;
    ctx->scheduled_delay = 0;

    LOG_INFO("SVC_PWR", "Scheduled action cancelled");

    OS_MutexUnlock(ctx->mutex);

    return OS_SUCCESS;
}

/*
 * 启用/禁用保护功能
 */
int32 PowerService_EnableProtection(power_service_handle_t handle,
                                    bool enable)
{
    if (handle == NULL) {
        return OS_INVALID_POINTER;
    }

    /* TODO: 实现保护功能 */
    LOG_INFO("SVC_PWR", "Power protection %s", enable ? "enabled" : "disabled");

    return OS_SUCCESS;
}

/*
 * 清除故障状态
 */
int32 PowerService_ClearFault(power_service_handle_t handle)
{
    if (handle == NULL) {
        return OS_INVALID_POINTER;
    }

    power_service_context_t *ctx = (power_service_context_t *)handle;

    OS_MutexLock(ctx->mutex);
    ctx->fault = false;
    LOG_INFO("SVC_PWR", "Fault cleared");
    OS_MutexUnlock(ctx->mutex);

    return OS_SUCCESS;
}

/*
 * 获取服务统计信息
 */
int32 PowerService_GetStats(power_service_handle_t handle,
                            uint32 *on_count,
                            uint32 *off_count,
                            uint32 *reset_count,
                            uint32 *fault_count)
{
    if (handle == NULL) {
        return OS_INVALID_POINTER;
    }

    power_service_context_t *ctx = (power_service_context_t *)handle;

    OS_MutexLock(ctx->mutex);

    if (on_count != NULL) *on_count = ctx->on_count;
    if (off_count != NULL) *off_count = ctx->off_count;
    if (reset_count != NULL) *reset_count = ctx->reset_count;
    if (fault_count != NULL) *fault_count = ctx->fault_count;

    OS_MutexUnlock(ctx->mutex);

    return OS_SUCCESS;
}
