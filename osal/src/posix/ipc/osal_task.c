/************************************************************************
 * OSAL POSIX实现 - 任务管理
 ************************************************************************/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "osal.h"
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <stdatomic.h>
#include <limits.h>

typedef enum
{
    TASK_STATE_READY = 0,
    TASK_STATE_RUNNING,
    TASK_STATE_TERMINATED
} task_state_t;

typedef struct
{
    bool        is_used;
    osal_id_t   id;
    char        name[OS_MAX_API_NAME];
    pthread_t   thread;
    uint32_t      priority;
    uint32_t      stack_size;
    _Atomic int32_t  ref_count;  /* 使用 C11 原子类型，固定大小 */
    task_state_t state;
    volatile bool shutdown_requested;
    pthread_cond_t exit_cond;    /* 退出条件变量 */
    pthread_mutex_t exit_mutex;  /* 退出互斥锁 */
    bool exited;                 /* 退出标志 */
} osal_task_record_t;

static osal_task_record_t g_osal_task_table[OS_MAX_TASKS] = {0};  /* 静态变量自动初始化为0 */
static pthread_mutex_t g_task_table_mutex = PTHREAD_MUTEX_INITIALIZER;

/* ID位图管理（支持1-64的ID，0保留为无效ID）*/
static uint64_t g_task_id_bitmap = 0;  /* 每个bit代表一个ID是否被使用 */

/**
 * @brief 分配一个空闲的任务ID
 * @param[out] task_id 分配的任务ID
 * @return OSAL_SUCCESS 成功, OSAL_ERR_NO_FREE_IDS 无可用ID
 * @note 调用前必须持有 g_task_table_mutex
 */
static int32_t allocate_task_id(osal_id_t *task_id)
{
    /* 查找第一个空闲的bit（ID范围：1-64） */
    for (uint32_t i = 0; i < OS_MAX_TASKS; i++)
    {
        uint64_t mask = (1ULL << i);
        if (0 == (g_task_id_bitmap & mask))
        {
            /* 找到空闲ID，标记为已使用 */
            g_task_id_bitmap |= mask;
            *task_id = i + 1;  /* ID从1开始 */
            return OSAL_SUCCESS;
        }
    }

    return OSAL_ERR_NO_FREE_IDS;
}

/**
 * @brief 释放一个任务ID
 * @param task_id 要释放的任务ID
 * @note 调用前必须持有 g_task_table_mutex
 */
static void release_task_id(osal_id_t task_id)
{
    if (task_id >= 1 && task_id <= OS_MAX_TASKS)
    {
        uint64_t mask = (1ULL << (task_id - 1));
        g_task_id_bitmap &= ~mask;  /* 清除对应bit */
    }
}

/* 移除 osal_task_table_init() - 静态变量已自动初始化 */

typedef struct
{
    osal_task_entry entry_func;
    void *user_arg;
    osal_id_t task_id;
} task_wrapper_arg_t;

static void *task_wrapper(void *arg)
{
    union {
        void *raw_ptr;
        task_wrapper_arg_t *typed_ptr;
    } arg_union;

    arg_union.raw_ptr = arg;
    task_wrapper_arg_t *wrapper_arg = arg_union.typed_ptr;

    osal_task_entry entry_func = wrapper_arg->entry_func;
    void *user_arg = wrapper_arg->user_arg;
    osal_id_t task_id = wrapper_arg->task_id;

    free(wrapper_arg);

    pthread_mutex_lock(&g_task_table_mutex);
    for (uint32_t i = 0; i < OS_MAX_TASKS; i++)
    {
        if (g_osal_task_table[i].is_used && g_osal_task_table[i].id == task_id)
        {
            g_osal_task_table[i].state = TASK_STATE_RUNNING;
            break;
        }
    }
    pthread_mutex_unlock(&g_task_table_mutex);

    entry_func(user_arg);

    /* 标记任务已退出并通知等待者 */
    pthread_mutex_lock(&g_task_table_mutex);
    for (uint32_t i = 0; i < OS_MAX_TASKS; i++)
    {
        if (g_osal_task_table[i].is_used && g_osal_task_table[i].id == task_id)
        {
            g_osal_task_table[i].state = TASK_STATE_TERMINATED;

            pthread_mutex_lock(&g_osal_task_table[i].exit_mutex);
            g_osal_task_table[i].exited = true;
            pthread_cond_signal(&g_osal_task_table[i].exit_cond);
            pthread_mutex_unlock(&g_osal_task_table[i].exit_mutex);
            break;
        }
    }
    pthread_mutex_unlock(&g_task_table_mutex);

    return NULL;
}

static int32_t osal_task_find_free_slot(uint32_t *slot)
{
    pthread_mutex_lock(&g_task_table_mutex);

    for (uint32_t i = 0; i < OS_MAX_TASKS; i++)
    {
        if (!g_osal_task_table[i].is_used)
        {
            *slot = i;
            pthread_mutex_unlock(&g_task_table_mutex);
            return OSAL_SUCCESS;
        }
    }

    pthread_mutex_unlock(&g_task_table_mutex);
    return OSAL_ERR_NO_FREE_IDS;
}

int32_t OSAL_TaskCreate(osal_id_t *task_id,
                    const char *task_name,
                    osal_task_entry function_pointer,
                    void *user_arg,
                    uint32_t stack_size,
                    uint32_t priority,
                    uint32_t flags __attribute__((unused)))
{
    uint32_t slot;
    int32_t ret;
    pthread_attr_t attr;
    task_wrapper_arg_t *wrapper_arg = NULL;

    if (NULL == task_id || NULL == function_pointer)
        return OSAL_ERR_INVALID_POINTER;

    if (NULL == task_name || strlen(task_name) >= OS_MAX_API_NAME)
        return OSAL_ERR_NAME_TOO_LONG;

    if (priority < OS_TASK_PRIORITY_MIN || priority > OS_TASK_PRIORITY_MAX)
        return OSAL_ERR_INVALID_PRIORITY;

    ret = osal_task_find_free_slot(&slot);
    if (OSAL_SUCCESS != ret)
        return ret;

    pthread_mutex_lock(&g_task_table_mutex);
    for (uint32_t i = 0; i < OS_MAX_TASKS; i++)
    {
        if (g_osal_task_table[i].is_used &&
            strcmp(g_osal_task_table[i].name, task_name) == 0)
        {
            pthread_mutex_unlock(&g_task_table_mutex);
            return OSAL_ERR_NAME_TAKEN;
        }
    }

    wrapper_arg = malloc(sizeof(task_wrapper_arg_t));
    if (NULL == wrapper_arg)
    {
        pthread_mutex_unlock(&g_task_table_mutex);
        return OSAL_ERR_GENERIC;
    }

    /* 分配任务ID */
    osal_id_t new_task_id;
    ret = allocate_task_id(&new_task_id);
    if (OSAL_SUCCESS != ret)
    {
        free(wrapper_arg);
        pthread_mutex_unlock(&g_task_table_mutex);
        return ret;
    }

    wrapper_arg->entry_func = function_pointer;
    wrapper_arg->user_arg = user_arg;
    wrapper_arg->task_id = new_task_id;

    pthread_attr_init(&attr);
    /* 使用JOINABLE模式，以便在TaskDelete时可以join等待线程退出 */
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    if (stack_size > 0)
    {
        osal_size_t min_stack = PTHREAD_STACK_MIN;
        if (stack_size < min_stack)
            stack_size = min_stack;
        pthread_attr_setstacksize(&attr, stack_size);
    }

    g_osal_task_table[slot].is_used = true;
    g_osal_task_table[slot].id = new_task_id;
    strncpy(g_osal_task_table[slot].name, task_name, OS_MAX_API_NAME - 1);
    g_osal_task_table[slot].name[OS_MAX_API_NAME - 1] = '\0';
    g_osal_task_table[slot].priority = priority;
    g_osal_task_table[slot].stack_size = stack_size;
    g_osal_task_table[slot].state = TASK_STATE_READY;
    atomic_init(&g_osal_task_table[slot].ref_count, 1);

    /* 初始化退出同步机制，检查返回值 */
    if (0 != pthread_cond_init(&g_osal_task_table[slot].exit_cond, NULL))
    {
        g_osal_task_table[slot].is_used = false;
        release_task_id(new_task_id);
        pthread_attr_destroy(&attr);
        free(wrapper_arg);
        pthread_mutex_unlock(&g_task_table_mutex);
        return OSAL_ERR_GENERIC;
    }

    if (0 != pthread_mutex_init(&g_osal_task_table[slot].exit_mutex, NULL))
    {
        pthread_cond_destroy(&g_osal_task_table[slot].exit_cond);
        g_osal_task_table[slot].is_used = false;
        release_task_id(new_task_id);
        pthread_attr_destroy(&attr);
        free(wrapper_arg);
        pthread_mutex_unlock(&g_task_table_mutex);
        return OSAL_ERR_GENERIC;
    }

    g_osal_task_table[slot].exited = false;

    if (0 != pthread_create(&g_osal_task_table[slot].thread, &attr,
                       task_wrapper, wrapper_arg))
    {
        g_osal_task_table[slot].is_used = false;
        release_task_id(new_task_id);  /* 释放已分配的ID */
        pthread_cond_destroy(&g_osal_task_table[slot].exit_cond);
        pthread_mutex_destroy(&g_osal_task_table[slot].exit_mutex);
        pthread_attr_destroy(&attr);
        free(wrapper_arg);
        pthread_mutex_unlock(&g_task_table_mutex);
        return OSAL_ERR_GENERIC;
    }

    pthread_attr_destroy(&attr);

    *task_id = new_task_id;

    pthread_mutex_unlock(&g_task_table_mutex);

    /* 注册资源到泄漏检测系统 */
    OSAL_ResourceRegister(new_task_id, OSAL_RESOURCE_TYPE_TASK, task_name, __FILE__, __LINE__);

    return OSAL_SUCCESS;
}

int32_t OSAL_TaskDelete(osal_id_t task_id)
{
    pthread_t thread_to_delete = 0;
    bool found = false;
    uint32_t slot_index = 0;

    if (OS_OBJECT_ID_UNDEFINED == task_id)
        return OSAL_ERR_INVALID_ID;

    pthread_mutex_lock(&g_task_table_mutex);

    for (uint32_t i = 0; i < OS_MAX_TASKS; i++)
    {
        if (g_osal_task_table[i].is_used && g_osal_task_table[i].id == task_id)
        {
            /* 检查引用计数：如果有其他模块持有引用，拒绝删除 */
            int32_t ref_count = atomic_load(&g_osal_task_table[i].ref_count);
            if (ref_count > 1)
            {
                pthread_mutex_unlock(&g_task_table_mutex);
                LOG_ERROR("OSAL_Task", "Task '%s' still has %d references, cannot delete",
                         g_osal_task_table[i].name, ref_count);
                return OSAL_ERR_BUSY;
            }

            thread_to_delete = g_osal_task_table[i].thread;
            g_osal_task_table[i].shutdown_requested = true;
            slot_index = i;
            found = true;
            break;
        }
    }

    pthread_mutex_unlock(&g_task_table_mutex);

    if (!found)
        return OSAL_ERR_INVALID_ID;

    /*
     * 等待线程优雅退出（带超时机制）
     *
     * 策略：
     * 1. 已设置 shutdown_requested 标志
     * 2. 使用条件变量等待任务退出，超时时间 5 秒
     * 3. 如果任务不响应，返回超时错误
     */
    struct timespec timeout;
    clock_gettime(CLOCK_REALTIME, &timeout);
    timeout.tv_sec += 5;  /* 5秒超时 */

    pthread_mutex_lock(&g_osal_task_table[slot_index].exit_mutex);
    while (!g_osal_task_table[slot_index].exited)
    {
        int32_t ret = pthread_cond_timedwait(&g_osal_task_table[slot_index].exit_cond,
                                             &g_osal_task_table[slot_index].exit_mutex,
                                             &timeout);
        if (ret == ETIMEDOUT)
        {
            LOG_ERROR("OSAL_Task", "Task '%s' did not exit within 5 seconds",
                     g_osal_task_table[slot_index].name);
            pthread_mutex_unlock(&g_osal_task_table[slot_index].exit_mutex);
            return OSAL_ERR_TIMEOUT;
        }
        else if (ret != 0 && ret != EINTR)
        {
            pthread_mutex_unlock(&g_osal_task_table[slot_index].exit_mutex);
            return OSAL_ERR_GENERIC;
        }
    }
    pthread_mutex_unlock(&g_osal_task_table[slot_index].exit_mutex);

    /* 任务已退出，执行 join 回收资源 */
    int32_t ret = pthread_join(thread_to_delete, NULL);

    if (0 != ret)
    {
        /* join 失败，记录错误 */
        OSAL_Printf("[OS_Task] 等待任务 %u 退出失败: %d\n", task_id, ret);

        /* 尝试分离线程，避免资源泄漏 */
        pthread_detach(thread_to_delete);
    }

    /* 从任务表中移除 */
    pthread_mutex_lock(&g_task_table_mutex);
    if (g_osal_task_table[slot_index].is_used && g_osal_task_table[slot_index].id == task_id)
    {
        g_osal_task_table[slot_index].is_used = false;
        release_task_id(task_id);  /* 释放任务ID，允许重用 */
        pthread_cond_destroy(&g_osal_task_table[slot_index].exit_cond);
        pthread_mutex_destroy(&g_osal_task_table[slot_index].exit_mutex);
    }
    pthread_mutex_unlock(&g_task_table_mutex);

    /* 从资源跟踪器中注销 */
    OSAL_ResourceUnregister(task_id, OSAL_RESOURCE_TYPE_TASK);

    return OSAL_SUCCESS;
}

osal_id_t OSAL_TaskGetId(void)
{
    pthread_t self = pthread_self();

    pthread_mutex_lock(&g_task_table_mutex);

    for (uint32_t i = 0; i < OS_MAX_TASKS; i++)
    {
        if (g_osal_task_table[i].is_used &&
            pthread_equal(g_osal_task_table[i].thread, self))
        {
            osal_id_t id = g_osal_task_table[i].id;
            pthread_mutex_unlock(&g_task_table_mutex);
            return id;
        }
    }

    pthread_mutex_unlock(&g_task_table_mutex);
    return OS_OBJECT_ID_UNDEFINED;
}

bool OSAL_TaskShouldShutdown(void)
{
    pthread_t self = pthread_self();

    pthread_mutex_lock(&g_task_table_mutex);

    for (uint32_t i = 0; i < OS_MAX_TASKS; i++)
    {
        if (g_osal_task_table[i].is_used &&
            pthread_equal(g_osal_task_table[i].thread, self))
        {
            bool shutdown = g_osal_task_table[i].shutdown_requested;
            pthread_mutex_unlock(&g_task_table_mutex);
            return shutdown;
        }
    }

    pthread_mutex_unlock(&g_task_table_mutex);
    return false;
}

int32_t OSAL_TaskGetIdByName(osal_id_t *task_id, const char *task_name)
{
    if (NULL == task_id || NULL == task_name)
        return OSAL_ERR_INVALID_POINTER;

    pthread_mutex_lock(&g_task_table_mutex);

    for (uint32_t i = 0; i < OS_MAX_TASKS; i++)
    {
        if (g_osal_task_table[i].is_used &&
            strcmp(g_osal_task_table[i].name, task_name) == 0)
        {
            *task_id = g_osal_task_table[i].id;
            pthread_mutex_unlock(&g_task_table_mutex);
            return OSAL_SUCCESS;
        }
    }

    pthread_mutex_unlock(&g_task_table_mutex);
    return OSAL_ERR_NAME_NOT_FOUND;
}

int32_t OSAL_TaskSetPriority(osal_id_t task_id, uint32_t priority)
{
    if (priority < OS_TASK_PRIORITY_MIN || priority > OS_TASK_PRIORITY_MAX)
        return OSAL_ERR_INVALID_PRIORITY;

    pthread_mutex_lock(&g_task_table_mutex);

    for (uint32_t i = 0; i < OS_MAX_TASKS; i++)
    {
        if (g_osal_task_table[i].is_used && g_osal_task_table[i].id == task_id)
        {
            g_osal_task_table[i].priority = priority;
            /* Linux用户空间线程优先级调整需要特权 */
            pthread_mutex_unlock(&g_task_table_mutex);
            return OSAL_SUCCESS;
        }
    }

    pthread_mutex_unlock(&g_task_table_mutex);
    return OSAL_ERR_INVALID_ID;
}

int32_t OSAL_TaskGetInfo(osal_id_t task_id, OS_TaskProp_t *task_prop)
{
    if (NULL == task_prop)
        return OSAL_ERR_INVALID_POINTER;

    pthread_mutex_lock(&g_task_table_mutex);

    for (uint32_t i = 0; i < OS_MAX_TASKS; i++)
    {
        if (g_osal_task_table[i].is_used && g_osal_task_table[i].id == task_id)
        {
            task_prop->priority = g_osal_task_table[i].priority;
            task_prop->stack_size = g_osal_task_table[i].stack_size;
            pthread_mutex_unlock(&g_task_table_mutex);
            return OSAL_SUCCESS;
        }
    }

    pthread_mutex_unlock(&g_task_table_mutex);
    return OSAL_ERR_INVALID_ID;
}
