/************************************************************************
 * 外设管理核心实现
 *
 * 功能：
 * - 外设驱动注册管理
 * - 外设实例创建和销毁
 * - 统一的外设操作接口
 ************************************************************************/

#include "pdl_peripheral_device.h"
#include "osal.h"
#include <string.h>

#define MAX_PERIPHERAL_TYPES 16
#define MAX_PERIPHERAL_INSTANCES 32

/*
 * 外设实例上下文
 */
typedef struct
{
    bool in_use;
    peripheral_type_t type;
    const peripheral_ops_t *ops;
    void *driver_handle;
    peripheral_event_callback_t callback;
    void *user_data;
    peripheral_stats_t stats;
} peripheral_context_t;

/*
 * 外设驱动注册表
 */
static const peripheral_ops_t *g_peripheral_drivers[MAX_PERIPHERAL_TYPES] = {NULL};

/*
 * 外设实例池
 */
static peripheral_context_t g_peripheral_instances[MAX_PERIPHERAL_INSTANCES] = {{0}};

/*
 * 互斥锁
 */
static osal_id_t g_peripheral_mutex = 0;

/**
 * @brief 初始化外设管理模块
 */
static int32 peripheral_core_init(void)
{
    if (g_peripheral_mutex == 0)
    {
        if (OS_MutexCreate(&g_peripheral_mutex, "peripheral_mutex", 0) != OS_SUCCESS)
        {
            return OS_ERROR;
        }
    }
    return OS_SUCCESS;
}

/**
 * @brief 注册外设驱动
 */
int32 Peripheral_RegisterDriver(peripheral_type_t type, const peripheral_ops_t *ops)
{
    if (type >= MAX_PERIPHERAL_TYPES || ops == NULL)
    {
        return OS_ERROR;
    }

    if (peripheral_core_init() != OS_SUCCESS)
    {
        return OS_ERROR;
    }

    OS_MutexLock(g_peripheral_mutex);
    g_peripheral_drivers[type] = ops;
    OS_MutexUnlock(g_peripheral_mutex);

    return OS_SUCCESS;
}

/**
 * @brief 创建外设实例
 */
int32 Peripheral_Create(peripheral_type_t type, void *config, peripheral_handle_t *handle)
{
    if (type >= MAX_PERIPHERAL_TYPES || handle == NULL)
    {
        return OS_ERROR;
    }

    if (peripheral_core_init() != OS_SUCCESS)
    {
        return OS_ERROR;
    }

    OS_MutexLock(g_peripheral_mutex);

    /* 查找驱动 */
    const peripheral_ops_t *ops = g_peripheral_drivers[type];
    if (ops == NULL || ops->init == NULL)
    {
        OS_MutexUnlock(g_peripheral_mutex);
        return OS_ERROR;
    }

    /* 查找空闲实例槽 */
    peripheral_context_t *ctx = NULL;
    for (int i = 0; i < MAX_PERIPHERAL_INSTANCES; i++)
    {
        if (!g_peripheral_instances[i].in_use)
        {
            ctx = &g_peripheral_instances[i];
            break;
        }
    }

    if (ctx == NULL)
    {
        OS_MutexUnlock(g_peripheral_mutex);
        return OS_ERROR;
    }

    /* 初始化驱动 */
    void *driver_handle = NULL;
    int32 ret = ops->init(config, &driver_handle);
    if (ret != OS_SUCCESS)
    {
        OS_MutexUnlock(g_peripheral_mutex);
        return ret;
    }

    /* 初始化上下文 */
    memset(ctx, 0, sizeof(peripheral_context_t));
    ctx->in_use = true;
    ctx->type = type;
    ctx->ops = ops;
    ctx->driver_handle = driver_handle;

    *handle = (peripheral_handle_t)ctx;

    OS_MutexUnlock(g_peripheral_mutex);
    return OS_SUCCESS;
}

/**
 * @brief 销毁外设实例
 */
int32 Peripheral_Destroy(peripheral_handle_t handle)
{
    if (handle == NULL)
    {
        return OS_ERROR;
    }

    peripheral_context_t *ctx = (peripheral_context_t *)handle;

    OS_MutexLock(g_peripheral_mutex);

    if (!ctx->in_use)
    {
        OS_MutexUnlock(g_peripheral_mutex);
        return OS_ERROR;
    }

    /* 反初始化驱动 */
    if (ctx->ops && ctx->ops->deinit)
    {
        ctx->ops->deinit(ctx->driver_handle);
    }

    /* 清空上下文 */
    memset(ctx, 0, sizeof(peripheral_context_t));

    OS_MutexUnlock(g_peripheral_mutex);
    return OS_SUCCESS;
}

/**
 * @brief 外设上电
 */
int32 Peripheral_PowerOn(peripheral_handle_t handle)
{
    if (handle == NULL)
    {
        return OS_ERROR;
    }

    peripheral_context_t *ctx = (peripheral_context_t *)handle;

    if (!ctx->in_use || ctx->ops == NULL || ctx->ops->power_on == NULL)
    {
        return OS_ERROR;
    }

    ctx->stats.cmd_count++;
    int32 ret = ctx->ops->power_on(ctx->driver_handle);

    if (ret == OS_SUCCESS)
    {
        ctx->stats.success_count++;
    }
    else
    {
        ctx->stats.fail_count++;
        ctx->stats.last_error_code = ret;
    }

    return ret;
}

/**
 * @brief 外设下电
 */
int32 Peripheral_PowerOff(peripheral_handle_t handle)
{
    if (handle == NULL)
    {
        return OS_ERROR;
    }

    peripheral_context_t *ctx = (peripheral_context_t *)handle;

    if (!ctx->in_use || ctx->ops == NULL || ctx->ops->power_off == NULL)
    {
        return OS_ERROR;
    }

    ctx->stats.cmd_count++;
    int32 ret = ctx->ops->power_off(ctx->driver_handle);

    if (ret == OS_SUCCESS)
    {
        ctx->stats.success_count++;
    }
    else
    {
        ctx->stats.fail_count++;
        ctx->stats.last_error_code = ret;
    }

    return ret;
}

/**
 * @brief 外设复位
 */
int32 Peripheral_Reset(peripheral_handle_t handle)
{
    if (handle == NULL)
    {
        return OS_ERROR;
    }

    peripheral_context_t *ctx = (peripheral_context_t *)handle;

    if (!ctx->in_use || ctx->ops == NULL || ctx->ops->reset == NULL)
    {
        return OS_ERROR;
    }

    ctx->stats.cmd_count++;
    ctx->stats.reset_count++;
    int32 ret = ctx->ops->reset(ctx->driver_handle);

    if (ret == OS_SUCCESS)
    {
        ctx->stats.success_count++;
    }
    else
    {
        ctx->stats.fail_count++;
        ctx->stats.last_error_code = ret;
    }

    return ret;
}

/**
 * @brief 获取外设状态
 */
int32 Peripheral_GetStatus(peripheral_handle_t handle, peripheral_status_t *status)
{
    if (handle == NULL || status == NULL)
    {
        return OS_ERROR;
    }

    peripheral_context_t *ctx = (peripheral_context_t *)handle;

    if (!ctx->in_use || ctx->ops == NULL || ctx->ops->get_status == NULL)
    {
        return OS_ERROR;
    }

    return ctx->ops->get_status(ctx->driver_handle, status);
}

/**
 * @brief 获取外设信息
 */
int32 Peripheral_GetInfo(peripheral_handle_t handle, peripheral_info_t *info)
{
    if (handle == NULL || info == NULL)
    {
        return OS_ERROR;
    }

    peripheral_context_t *ctx = (peripheral_context_t *)handle;

    if (!ctx->in_use || ctx->ops == NULL || ctx->ops->get_info == NULL)
    {
        return OS_ERROR;
    }

    return ctx->ops->get_info(ctx->driver_handle, info);
}

/**
 * @brief 执行外设命令
 */
int32 Peripheral_ExecuteCommand(peripheral_handle_t handle,
                               const void *cmd,
                               uint32 cmd_len,
                               void *response,
                               uint32 resp_size,
                               uint32 *actual_size)
{
    if (handle == NULL)
    {
        return OS_ERROR;
    }

    peripheral_context_t *ctx = (peripheral_context_t *)handle;

    if (!ctx->in_use || ctx->ops == NULL || ctx->ops->execute_command == NULL)
    {
        return OS_ERROR;
    }

    ctx->stats.cmd_count++;
    int32 ret = ctx->ops->execute_command(ctx->driver_handle, cmd, cmd_len,
                                         response, resp_size, actual_size);

    if (ret == OS_SUCCESS)
    {
        ctx->stats.success_count++;
    }
    else if (ret == OS_ERROR_TIMEOUT)
    {
        ctx->stats.timeout_count++;
    }
    else
    {
        ctx->stats.fail_count++;
        ctx->stats.last_error_code = ret;
    }

    return ret;
}

/**
 * @brief 读取外设数据
 */
int32 Peripheral_ReadData(peripheral_handle_t handle,
                         uint32 offset,
                         void *buffer,
                         uint32 size,
                         uint32 *actual_size)
{
    if (handle == NULL)
    {
        return OS_ERROR;
    }

    peripheral_context_t *ctx = (peripheral_context_t *)handle;

    if (!ctx->in_use || ctx->ops == NULL || ctx->ops->read_data == NULL)
    {
        return OS_ERROR;
    }

    return ctx->ops->read_data(ctx->driver_handle, offset, buffer, size, actual_size);
}

/**
 * @brief 写入外设数据
 */
int32 Peripheral_WriteData(peripheral_handle_t handle,
                          uint32 offset,
                          const void *buffer,
                          uint32 size)
{
    if (handle == NULL)
    {
        return OS_ERROR;
    }

    peripheral_context_t *ctx = (peripheral_context_t *)handle;

    if (!ctx->in_use || ctx->ops == NULL || ctx->ops->write_data == NULL)
    {
        return OS_ERROR;
    }

    return ctx->ops->write_data(ctx->driver_handle, offset, buffer, size);
}

/**
 * @brief 注册事件回调
 */
int32 Peripheral_RegisterCallback(peripheral_handle_t handle,
                                 peripheral_event_callback_t callback,
                                 void *user_data)
{
    if (handle == NULL)
    {
        return OS_ERROR;
    }

    peripheral_context_t *ctx = (peripheral_context_t *)handle;

    if (!ctx->in_use)
    {
        return OS_ERROR;
    }

    ctx->callback = callback;
    ctx->user_data = user_data;

    if (ctx->ops && ctx->ops->register_callback)
    {
        return ctx->ops->register_callback(ctx->driver_handle, callback, user_data);
    }

    return OS_SUCCESS;
}

/**
 * @brief 获取统计信息
 */
int32 Peripheral_GetStats(peripheral_handle_t handle, peripheral_stats_t *stats)
{
    if (handle == NULL || stats == NULL)
    {
        return OS_ERROR;
    }

    peripheral_context_t *ctx = (peripheral_context_t *)handle;

    if (!ctx->in_use)
    {
        return OS_ERROR;
    }

    memcpy(stats, &ctx->stats, sizeof(peripheral_stats_t));
    return OS_SUCCESS;
}

/**
 * @brief 重置统计信息
 */
int32 Peripheral_ResetStats(peripheral_handle_t handle)
{
    if (handle == NULL)
    {
        return OS_ERROR;
    }

    peripheral_context_t *ctx = (peripheral_context_t *)handle;

    if (!ctx->in_use)
    {
        return OS_ERROR;
    }

    memset(&ctx->stats, 0, sizeof(peripheral_stats_t));
    return OS_SUCCESS;
}
