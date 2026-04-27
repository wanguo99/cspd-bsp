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
    str_t       name[OS_MAX_API_NAME];
    pthread_t   thread;
    uint32_t      priority;
    uint32_t      stack_size;
    atomic_int  ref_count;
    task_state_t state;
    volatile bool shutdown_requested;
} osal_task_record_t;

static osal_task_record_t g_osal_task_table[OS_MAX_TASKS] = {0};  /* 静态变量自动初始化为0 */
static pthread_mutex_t g_task_table_mutex = PTHREAD_MUTEX_INITIALIZER;
static uint32_t g_next_task_id = 1;

/* 移除 osal_task_table_init() - 静态变量已自动初始化 */

typedef struct
{
    osal_task_entry entry_func;
    void *user_arg;
    osal_id_t task_id;
} task_wrapper_arg_t;

static void *task_wrapper(void *arg)
{
    task_wrapper_arg_t *wrapper_arg = (task_wrapper_arg_t *)arg;
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

    pthread_mutex_lock(&g_task_table_mutex);
    for (uint32_t i = 0; i < OS_MAX_TASKS; i++)
    {
        if (g_osal_task_table[i].is_used && g_osal_task_table[i].id == task_id)
        {
            g_osal_task_table[i].state = TASK_STATE_TERMINATED;
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
            return OS_SUCCESS;
        }
    }

    pthread_mutex_unlock(&g_task_table_mutex);
    return OS_ERR_NO_FREE_IDS;
}

int32_t OSAL_TaskCreate(osal_id_t *task_id,
                    const char *task_name,
                    osal_task_entry function_pointer,
                    uint32_t *stack_pointer,
                    uint32_t stack_size,
                    uint32_t priority,
                    uint32_t flags __attribute__((unused)))
{
    uint32_t slot;
    int32_t ret;
    pthread_attr_t attr;
    task_wrapper_arg_t *wrapper_arg = NULL;

    if (task_id == NULL || function_pointer == NULL)
        return OS_INVALID_POINTER;

    if (task_name == NULL || strlen(task_name) >= OS_MAX_API_NAME)
        return OS_ERR_NAME_TOO_LONG;

    if (priority < OS_TASK_PRIORITY_MIN || priority > OS_TASK_PRIORITY_MAX)
        return OS_ERR_INVALID_PRIORITY;

    ret = osal_task_find_free_slot(&slot);
    if (OS_SUCCESS != ret)
        return ret;

    pthread_mutex_lock(&g_task_table_mutex);
    for (uint32_t i = 0; i < OS_MAX_TASKS; i++)
    {
        if (g_osal_task_table[i].is_used &&
            strcmp(g_osal_task_table[i].name, task_name) == 0)
        {
            pthread_mutex_unlock(&g_task_table_mutex);
            return OS_ERR_NAME_TAKEN;
        }
    }

    wrapper_arg = malloc(sizeof(task_wrapper_arg_t));
    if (NULL == wrapper_arg)
    {
        pthread_mutex_unlock(&g_task_table_mutex);
        return OS_ERROR;
    }

    osal_id_t new_task_id = g_next_task_id++;

    wrapper_arg->entry_func = function_pointer;
    wrapper_arg->user_arg = (void *)stack_pointer;
    wrapper_arg->task_id = new_task_id;

    pthread_attr_init(&attr);
    /* 使用JOINABLE模式，以便在TaskDelete时可以join等待线程退出 */
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    if (stack_size > 0)
    {
        size_t min_stack = PTHREAD_STACK_MIN;
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

    if (pthread_create(&g_osal_task_table[slot].thread, &attr,
                       task_wrapper, wrapper_arg) != 0)
    {
        g_osal_task_table[slot].is_used = false;
        pthread_attr_destroy(&attr);
        free(wrapper_arg);
        pthread_mutex_unlock(&g_task_table_mutex);
        return OS_ERROR;
    }

    pthread_attr_destroy(&attr);

    *task_id = new_task_id;

    pthread_mutex_unlock(&g_task_table_mutex);

    return OS_SUCCESS;
}

int32_t OSAL_TaskDelete(osal_id_t task_id)
{
    pthread_t thread_to_delete = 0;
    bool found = false;
    uint32_t slot_index = 0;

    if (task_id == OS_OBJECT_ID_UNDEFINED)
        return OS_ERR_INVALID_ID;

    pthread_mutex_lock(&g_task_table_mutex);

    for (uint32_t i = 0; i < OS_MAX_TASKS; i++)
    {
        if (g_osal_task_table[i].is_used && g_osal_task_table[i].id == task_id)
        {
            thread_to_delete = g_osal_task_table[i].thread;
            g_osal_task_table[i].shutdown_requested = true;
            slot_index = i;
            found = true;
            break;
        }
    }

    pthread_mutex_unlock(&g_task_table_mutex);

    if (!found)
        return OS_ERR_INVALID_ID;

    /* 等待线程优雅退出，超时时间为5秒 */
    struct timespec timeout;
    clock_gettime(CLOCK_REALTIME, &timeout);
    timeout.tv_sec += 5;

    int ret = pthread_timedjoin_np(thread_to_delete, NULL, &timeout);

    if (ETIMEDOUT == ret)
    {
        /* 超时后强制分离线程，允许其自然退出 */
        OSAL_Printf("[OS_Task] 任务 %u 优雅关闭超时，分离线程\n", task_id);
        pthread_detach(thread_to_delete);
    }
    else if (0 != ret)
    {
        /* join失败，记录错误但继续清理 */
        OSAL_Printf("[OS_Task] 等待任务 %u 退出失败: %d\n", task_id, ret);
    }

    /* 从任务表中移除 */
    pthread_mutex_lock(&g_task_table_mutex);
    if (g_osal_task_table[slot_index].is_used && g_osal_task_table[slot_index].id == task_id)
    {
        g_osal_task_table[slot_index].is_used = false;
    }
    pthread_mutex_unlock(&g_task_table_mutex);

    return OS_SUCCESS;
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
    if (task_id == NULL || task_name == NULL)
        return OS_INVALID_POINTER;

    pthread_mutex_lock(&g_task_table_mutex);

    for (uint32_t i = 0; i < OS_MAX_TASKS; i++)
    {
        if (g_osal_task_table[i].is_used &&
            strcmp(g_osal_task_table[i].name, task_name) == 0)
        {
            *task_id = g_osal_task_table[i].id;
            pthread_mutex_unlock(&g_task_table_mutex);
            return OS_SUCCESS;
        }
    }

    pthread_mutex_unlock(&g_task_table_mutex);
    return OS_ERR_NAME_NOT_FOUND;
}

int32_t OSAL_TaskSetPriority(osal_id_t task_id, uint32_t priority)
{
    if (priority < OS_TASK_PRIORITY_MIN || priority > OS_TASK_PRIORITY_MAX)
        return OS_ERR_INVALID_PRIORITY;

    pthread_mutex_lock(&g_task_table_mutex);

    for (uint32_t i = 0; i < OS_MAX_TASKS; i++)
    {
        if (g_osal_task_table[i].is_used && g_osal_task_table[i].id == task_id)
        {
            g_osal_task_table[i].priority = priority;
            /* Linux用户空间线程优先级调整需要特权 */
            pthread_mutex_unlock(&g_task_table_mutex);
            return OS_SUCCESS;
        }
    }

    pthread_mutex_unlock(&g_task_table_mutex);
    return OS_ERR_INVALID_ID;
}

int32_t OSAL_TaskGetInfo(osal_id_t task_id, OS_TaskProp_t *task_prop)
{
    if (NULL == task_prop)
        return OS_INVALID_POINTER;

    pthread_mutex_lock(&g_task_table_mutex);

    for (uint32_t i = 0; i < OS_MAX_TASKS; i++)
    {
        if (g_osal_task_table[i].is_used && g_osal_task_table[i].id == task_id)
        {
            task_prop->priority = g_osal_task_table[i].priority;
            task_prop->stack_size = g_osal_task_table[i].stack_size;
            pthread_mutex_unlock(&g_task_table_mutex);
            return OS_SUCCESS;
        }
    }

    pthread_mutex_unlock(&g_task_table_mutex);
    return OS_ERR_INVALID_ID;
}
