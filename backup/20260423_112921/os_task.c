/************************************************************************
 * OSAL Linux/POSIX实现 - 任务管理
 ************************************************************************/

#include "osal.h"
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

/*
 * 任务表项
 */
typedef struct
{
    bool        is_used;
    osal_id_t   id;
    char        name[OS_MAX_API_NAME];
    pthread_t   thread;
    uint32      priority;
    uint32      stack_size;
} OS_task_record_t;

/*
 * 任务表
 */
static OS_task_record_t OS_task_table[OS_MAX_TASKS];
static pthread_mutex_t  task_table_mutex = PTHREAD_MUTEX_INITIALIZER;
static uint32           next_task_id = 1;

/*
 * 初始化任务表
 */
void OS_TaskTableInit(void)
{
    pthread_mutex_lock(&task_table_mutex);
    memset(OS_task_table, 0, sizeof(OS_task_table));
    next_task_id = 1;
    pthread_mutex_unlock(&task_table_mutex);
}

/*
 * 任务包装器
 */
typedef struct
{
    osal_task_entry entry_func;
    void *user_arg;
} task_wrapper_arg_t;

static void *task_wrapper(void *arg)
{
    task_wrapper_arg_t *wrapper_arg = (task_wrapper_arg_t *)arg;
    osal_task_entry entry_func = wrapper_arg->entry_func;
    void *user_arg = wrapper_arg->user_arg;

    free(wrapper_arg);

    /* 调用用户任务函数 */
    entry_func(user_arg);

    return NULL;
}

/*
 * 查找空闲任务槽
 */
static int32 find_free_task_slot(uint32 *slot)
{
    pthread_mutex_lock(&task_table_mutex);

    for (uint32 i = 0; i < OS_MAX_TASKS; i++)
    {
        if (!OS_task_table[i].is_used)
        {
            *slot = i;
            pthread_mutex_unlock(&task_table_mutex);
            return OS_SUCCESS;
        }
    }

    pthread_mutex_unlock(&task_table_mutex);
    return OS_ERR_NO_FREE_IDS;
}

/*
 * API实现
 */
int32 OS_TaskCreate(osal_id_t *task_id,
                    const char *task_name,
                    osal_task_entry function_pointer,
                    uint32 *stack_pointer __attribute__((unused)),
                    uint32 stack_size,
                    uint32 priority,
                    uint32 flags __attribute__((unused)))
{
    uint32 slot;
    int32 ret;
    pthread_attr_t attr;
    task_wrapper_arg_t *wrapper_arg;

    /* 参数检查 */
    if (task_id == NULL || function_pointer == NULL)
        return OS_INVALID_POINTER;

    if (task_name == NULL || strlen(task_name) >= OS_MAX_API_NAME)
        return OS_ERR_NAME_TOO_LONG;

    if (priority < OS_TASK_PRIORITY_MIN || priority > OS_TASK_PRIORITY_MAX)
        return OS_ERR_INVALID_PRIORITY;

    /* 查找空闲槽 */
    ret = find_free_task_slot(&slot);
    if (ret != OS_SUCCESS)
        return ret;

    /* 检查名称是否已存在 */
    pthread_mutex_lock(&task_table_mutex);
    for (uint32 i = 0; i < OS_MAX_TASKS; i++)
    {
        if (OS_task_table[i].is_used &&
            strcmp(OS_task_table[i].name, task_name) == 0)
        {
            pthread_mutex_unlock(&task_table_mutex);
            return OS_ERR_NAME_TAKEN;
        }
    }

    /* 分配包装器参数 */
    wrapper_arg = malloc(sizeof(task_wrapper_arg_t));
    if (wrapper_arg == NULL)
    {
        pthread_mutex_unlock(&task_table_mutex);
        return OS_ERROR;
    }
    wrapper_arg->entry_func = function_pointer;
    wrapper_arg->user_arg = stack_pointer;  /* 使用 stack_pointer 传递用户参数 */

    /* 初始化线程属性 */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    if (stack_size > 0)
        pthread_attr_setstacksize(&attr, stack_size);

    /* 创建线程 */
    if (pthread_create(&OS_task_table[slot].thread, &attr,
                       task_wrapper, wrapper_arg) != 0)
    {
        pthread_attr_destroy(&attr);
        free(wrapper_arg);
        pthread_mutex_unlock(&task_table_mutex);
        return OS_ERROR;
    }

    pthread_attr_destroy(&attr);

    /* 填充任务表 */
    OS_task_table[slot].is_used = true;
    OS_task_table[slot].id = next_task_id++;
    strncpy(OS_task_table[slot].name, task_name, OS_MAX_API_NAME - 1);
    OS_task_table[slot].priority = priority;
    OS_task_table[slot].stack_size = stack_size;

    *task_id = OS_task_table[slot].id;

    pthread_mutex_unlock(&task_table_mutex);

    return OS_SUCCESS;
}

int32 OS_TaskDelete(osal_id_t task_id)
{
    pthread_mutex_lock(&task_table_mutex);

    for (uint32 i = 0; i < OS_MAX_TASKS; i++)
    {
        if (OS_task_table[i].is_used && OS_task_table[i].id == task_id)
        {
            pthread_cancel(OS_task_table[i].thread);
            OS_task_table[i].is_used = false;
            pthread_mutex_unlock(&task_table_mutex);
            return OS_SUCCESS;
        }
    }

    pthread_mutex_unlock(&task_table_mutex);
    return OS_ERR_INVALID_ID;
}

int32 OS_TaskDelay(uint32 millisecond)
{
    struct timespec ts;
    ts.tv_sec = millisecond / 1000;
    ts.tv_nsec = (millisecond % 1000) * 1000000;
    nanosleep(&ts, NULL);
    return OS_SUCCESS;
}

osal_id_t OS_TaskGetId(void)
{
    pthread_t self = pthread_self();

    pthread_mutex_lock(&task_table_mutex);

    for (uint32 i = 0; i < OS_MAX_TASKS; i++)
    {
        if (OS_task_table[i].is_used &&
            pthread_equal(OS_task_table[i].thread, self))
        {
            osal_id_t id = OS_task_table[i].id;
            pthread_mutex_unlock(&task_table_mutex);
            return id;
        }
    }

    pthread_mutex_unlock(&task_table_mutex);
    return OS_OBJECT_ID_UNDEFINED;
}

int32 OS_TaskGetIdByName(osal_id_t *task_id, const char *task_name)
{
    if (task_id == NULL || task_name == NULL)
        return OS_INVALID_POINTER;

    pthread_mutex_lock(&task_table_mutex);

    for (uint32 i = 0; i < OS_MAX_TASKS; i++)
    {
        if (OS_task_table[i].is_used &&
            strcmp(OS_task_table[i].name, task_name) == 0)
        {
            *task_id = OS_task_table[i].id;
            pthread_mutex_unlock(&task_table_mutex);
            return OS_SUCCESS;
        }
    }

    pthread_mutex_unlock(&task_table_mutex);
    return OS_ERR_NAME_NOT_FOUND;
}

int32 OS_TaskSetPriority(osal_id_t task_id, uint32 priority)
{
    if (priority < OS_TASK_PRIORITY_MIN || priority > OS_TASK_PRIORITY_MAX)
        return OS_ERR_INVALID_PRIORITY;

    pthread_mutex_lock(&task_table_mutex);

    for (uint32 i = 0; i < OS_MAX_TASKS; i++)
    {
        if (OS_task_table[i].is_used && OS_task_table[i].id == task_id)
        {
            OS_task_table[i].priority = priority;
            /* 注意: Linux用户空间线程优先级调整需要特权 */
            pthread_mutex_unlock(&task_table_mutex);
            return OS_SUCCESS;
        }
    }

    pthread_mutex_unlock(&task_table_mutex);
    return OS_ERR_INVALID_ID;
}

int32 OS_TaskGetInfo(osal_id_t task_id, OS_TaskProp_t *task_prop)
{
    if (task_prop == NULL)
        return OS_INVALID_POINTER;

    pthread_mutex_lock(&task_table_mutex);

    for (uint32 i = 0; i < OS_MAX_TASKS; i++)
    {
        if (OS_task_table[i].is_used && OS_task_table[i].id == task_id)
        {
            task_prop->priority = OS_task_table[i].priority;
            task_prop->stack_size = OS_task_table[i].stack_size;
            pthread_mutex_unlock(&task_table_mutex);
            return OS_SUCCESS;
        }
    }

    pthread_mutex_unlock(&task_table_mutex);
    return OS_ERR_INVALID_ID;
}
