/************************************************************************
 * 软件看门狗和故障恢复机制实现
 ************************************************************************/

#include "watchdog.h"
#include "osal.h"
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/watchdog.h>

#define MAX_MONITORED_TASKS  32

/*
 * 监控任务信息
 */
typedef struct
{
    bool        in_use;
    osal_id_t   task_id;
    char        task_name[OS_MAX_API_NAME];
    uint32      timeout_ms;
    uint32      last_heartbeat;
    uint32      miss_count;
    task_health_t health;
} monitored_task_t;

/*
 * 看门狗上下文
 */
typedef struct
{
    watchdog_config_t config;
    monitored_task_t  tasks[MAX_MONITORED_TASKS];
    osal_id_t         mutex;
    osal_id_t         watchdog_task_id;
    int32             hw_watchdog_fd;
    bool              running;
    uint32            system_uptime;
} watchdog_context_t;

static watchdog_context_t *g_watchdog_ctx = NULL;

/*
 * 硬件看门狗操作
 */
static int32 hw_watchdog_open(watchdog_context_t *ctx)
{
    if (!ctx->config.enable_hw_watchdog)
        return OS_SUCCESS;

    ctx->hw_watchdog_fd = open(ctx->config.hw_watchdog_dev, O_WRONLY);
    if (ctx->hw_watchdog_fd < 0)
    {
        OS_printf("[Watchdog] 打开硬件看门狗失败: %s\n", ctx->config.hw_watchdog_dev);
        return OS_ERROR;
    }

    /* 设置超时时间 */
    int timeout = ctx->config.hw_watchdog_timeout_s;
    if (ioctl(ctx->hw_watchdog_fd, WDIOC_SETTIMEOUT, &timeout) < 0)
    {
        OS_printf("[Watchdog] 设置硬件看门狗超时失败\n");
        close(ctx->hw_watchdog_fd);
        ctx->hw_watchdog_fd = -1;
        return OS_ERROR;
    }

    OS_printf("[Watchdog] 硬件看门狗已启用，超时: %d秒\n", timeout);
    return OS_SUCCESS;
}

static void hw_watchdog_feed(watchdog_context_t *ctx)
{
    if (ctx->hw_watchdog_fd >= 0)
    {
        /* 喂狗 */
        ioctl(ctx->hw_watchdog_fd, WDIOC_KEEPALIVE, 0);
    }
}

static void hw_watchdog_close(watchdog_context_t *ctx)
{
    if (ctx->hw_watchdog_fd >= 0)
    {
        /* 写入魔术字符关闭看门狗 */
        ssize_t ret = write(ctx->hw_watchdog_fd, "V", 1);
        if (ret < 0)
        {
            OS_printf("[Watchdog] 关闭硬件看门狗失败\n");
        }
        close(ctx->hw_watchdog_fd);
        ctx->hw_watchdog_fd = -1;
    }
}

/*
 * 检查任务健康状态
 */
static void check_task_health(watchdog_context_t *ctx)
{
    uint32 current_time = OS_GetTickCount();

    OS_MutexLock(ctx->mutex);

    for (uint32 i = 0; i < MAX_MONITORED_TASKS; i++)
    {
        if (!ctx->tasks[i].in_use)
            continue;

        monitored_task_t *task = &ctx->tasks[i];
        uint32 elapsed = current_time - task->last_heartbeat;

        if (elapsed > task->timeout_ms)
        {
            task->miss_count++;

            if (task->miss_count >= 3)
            {
                task->health = TASK_HEALTH_CRITICAL;
                OS_printf("[Watchdog] 任务 %s 严重超时 (已超时 %u 次)\n",
                         task->task_name, task->miss_count);

                /* 尝试恢复任务 */
                OS_printf("[Watchdog] 尝试重启任务 %s\n", task->task_name);
                /* TODO: 实现任务重启逻辑 */
            }
            else if (task->miss_count >= 1)
            {
                task->health = TASK_HEALTH_WARNING;
                OS_printf("[Watchdog] 任务 %s 心跳超时 (超时 %u ms)\n",
                         task->task_name, elapsed);
            }
        }
        else
        {
            if (task->miss_count > 0)
            {
                OS_printf("[Watchdog] 任务 %s 已恢复\n", task->task_name);
            }
            task->miss_count = 0;
            task->health = TASK_HEALTH_OK;
        }
    }

    OS_MutexUnlock(ctx->mutex);
}

/*
 * 看门狗主任务
 */
static void watchdog_task(void *arg)
{
    watchdog_context_t *ctx = (watchdog_context_t *)arg;

    OS_printf("[Watchdog] 看门狗任务启动\n");

    while (ctx->running)
    {
        /* 检查任务健康状态 */
        check_task_health(ctx);

        /* 喂硬件看门狗 */
        hw_watchdog_feed(ctx);

        /* 更新系统运行时间 */
        ctx->system_uptime += ctx->config.check_interval_ms / 1000;

        /* 延时 */
        OS_TaskDelay(ctx->config.check_interval_ms);
    }

    OS_printf("[Watchdog] 看门狗任务退出\n");
}

/*
 * 公共接口实现
 */
int32 Watchdog_Init(const watchdog_config_t *config)
{
    int32 ret;

    if (config == NULL)
        return OS_INVALID_POINTER;

    if (g_watchdog_ctx != NULL)
    {
        OS_printf("[Watchdog] 看门狗已初始化\n");
        return OS_ERROR;
    }

    /* 分配上下文 */
    g_watchdog_ctx = malloc(sizeof(watchdog_context_t));
    if (g_watchdog_ctx == NULL)
    {
        OS_printf("[Watchdog] 内存分配失败\n");
        return OS_ERROR;
    }

    memset(g_watchdog_ctx, 0, sizeof(watchdog_context_t));
    memcpy(&g_watchdog_ctx->config, config, sizeof(watchdog_config_t));
    g_watchdog_ctx->hw_watchdog_fd = -1;
    g_watchdog_ctx->running = true;

    /* 创建互斥锁 */
    ret = OS_MutexCreate(&g_watchdog_ctx->mutex, "WDG_MTX", 0);
    if (ret != OS_SUCCESS)
    {
        OS_printf("[Watchdog] 创建互斥锁失败\n");
        free(g_watchdog_ctx);
        g_watchdog_ctx = NULL;
        return ret;
    }

    /* 打开硬件看门狗 */
    hw_watchdog_open(g_watchdog_ctx);

    /* 创建看门狗任务 */
    ret = OS_TaskCreate(&g_watchdog_ctx->watchdog_task_id,
                        "WATCHDOG",
                        watchdog_task,
                        (uint32 *)g_watchdog_ctx,
                        32 * 1024,
                        50,  /* 中等优先级 */
                        0);
    if (ret != OS_SUCCESS)
    {
        OS_printf("[Watchdog] 创建看门狗任务失败\n");
        hw_watchdog_close(g_watchdog_ctx);
        OS_MutexDelete(g_watchdog_ctx->mutex);
        free(g_watchdog_ctx);
        g_watchdog_ctx = NULL;
        return ret;
    }

    OS_printf("[Watchdog] 看门狗系统已初始化\n");
    return OS_SUCCESS;
}

int32 Watchdog_Deinit(void)
{
    if (g_watchdog_ctx == NULL)
        return OS_ERROR;

    /* 停止看门狗任务 */
    g_watchdog_ctx->running = false;
    OS_TaskDelay(1000);  /* 等待任务退出 */

    /* 关闭硬件看门狗 */
    hw_watchdog_close(g_watchdog_ctx);

    /* 删除互斥锁 */
    OS_MutexDelete(g_watchdog_ctx->mutex);

    /* 释放上下文 */
    free(g_watchdog_ctx);
    g_watchdog_ctx = NULL;

    OS_printf("[Watchdog] 看门狗系统已关闭\n");
    return OS_SUCCESS;
}

int32 Watchdog_RegisterTask(osal_id_t task_id, const char *task_name, uint32 timeout_ms)
{
    if (g_watchdog_ctx == NULL)
        return OS_ERROR;

    if (task_name == NULL || timeout_ms == 0)
        return OS_INVALID_POINTER;

    OS_MutexLock(g_watchdog_ctx->mutex);

    /* 查找空闲槽 */
    for (uint32 i = 0; i < MAX_MONITORED_TASKS; i++)
    {
        if (!g_watchdog_ctx->tasks[i].in_use)
        {
            g_watchdog_ctx->tasks[i].in_use = true;
            g_watchdog_ctx->tasks[i].task_id = task_id;
            strncpy(g_watchdog_ctx->tasks[i].task_name, task_name, OS_MAX_API_NAME - 1);
            g_watchdog_ctx->tasks[i].task_name[OS_MAX_API_NAME - 1] = '\0';
            g_watchdog_ctx->tasks[i].timeout_ms = timeout_ms;
            g_watchdog_ctx->tasks[i].last_heartbeat = OS_GetTickCount();
            g_watchdog_ctx->tasks[i].miss_count = 0;
            g_watchdog_ctx->tasks[i].health = TASK_HEALTH_OK;

            OS_MutexUnlock(g_watchdog_ctx->mutex);

            OS_printf("[Watchdog] 已注册任务: %s (超时: %u ms)\n", task_name, timeout_ms);
            return OS_SUCCESS;
        }
    }

    OS_MutexUnlock(g_watchdog_ctx->mutex);

    OS_printf("[Watchdog] 无可用槽位\n");
    return OS_ERROR;
}

int32 Watchdog_Heartbeat(osal_id_t task_id)
{
    if (g_watchdog_ctx == NULL)
        return OS_ERROR;

    OS_MutexLock(g_watchdog_ctx->mutex);

    for (uint32 i = 0; i < MAX_MONITORED_TASKS; i++)
    {
        if (g_watchdog_ctx->tasks[i].in_use &&
            g_watchdog_ctx->tasks[i].task_id == task_id)
        {
            g_watchdog_ctx->tasks[i].last_heartbeat = OS_GetTickCount();
            OS_MutexUnlock(g_watchdog_ctx->mutex);
            return OS_SUCCESS;
        }
    }

    OS_MutexUnlock(g_watchdog_ctx->mutex);
    return OS_ERR_INVALID_ID;
}

int32 Watchdog_GetTaskHealth(osal_id_t task_id, task_health_t *health)
{
    if (g_watchdog_ctx == NULL || health == NULL)
        return OS_INVALID_POINTER;

    OS_MutexLock(g_watchdog_ctx->mutex);

    for (uint32 i = 0; i < MAX_MONITORED_TASKS; i++)
    {
        if (g_watchdog_ctx->tasks[i].in_use &&
            g_watchdog_ctx->tasks[i].task_id == task_id)
        {
            *health = g_watchdog_ctx->tasks[i].health;
            OS_MutexUnlock(g_watchdog_ctx->mutex);
            return OS_SUCCESS;
        }
    }

    OS_MutexUnlock(g_watchdog_ctx->mutex);
    return OS_ERR_INVALID_ID;
}

task_health_t Watchdog_GetSystemHealth(void)
{
    if (g_watchdog_ctx == NULL)
        return TASK_HEALTH_UNKNOWN;

    task_health_t worst_health = TASK_HEALTH_OK;

    OS_MutexLock(g_watchdog_ctx->mutex);

    for (uint32 i = 0; i < MAX_MONITORED_TASKS; i++)
    {
        if (g_watchdog_ctx->tasks[i].in_use)
        {
            if (g_watchdog_ctx->tasks[i].health > worst_health)
            {
                worst_health = g_watchdog_ctx->tasks[i].health;
            }
        }
    }

    OS_MutexUnlock(g_watchdog_ctx->mutex);

    return worst_health;
}
