/************************************************************************
 * OSAL FreeRTOS实现 - 任务管理
 *
 * 这是FreeRTOS平台的OSAL实现示例
 * 展示如何将相同的业务代码移植到MCU平台
 ************************************************************************/

#include "osal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <string.h>

/*
 * 任务表项
 */
typedef struct
{
    bool        is_used;
    osal_id_t   id;
    char        name[OS_MAX_API_NAME];
    TaskHandle_t handle;
    uint32      priority;
    uint32      stack_size;
} OS_task_record_t;

/*
 * 任务表
 */
static OS_task_record_t OS_task_table[OS_MAX_TASKS];
static SemaphoreHandle_t task_table_mutex;
static uint32           next_task_id = 1;

/*
 * 任务包装器
 */
typedef struct
{
    osal_task_entry entry_func;
} task_wrapper_arg_t;

static void task_wrapper(void *arg)
{
    task_wrapper_arg_t *wrapper_arg = (task_wrapper_arg_t *)arg;
    osal_task_entry entry_func = wrapper_arg->entry_func;

    vPortFree(wrapper_arg);

    /* 调用用户任务函数 */
    entry_func();

    /* 任务退出时删除自己 */
    vTaskDelete(NULL);
}

/*
 * 查找空闲任务槽
 */
static int32 find_free_task_slot(uint32 *slot)
{
    xSemaphoreTake(task_table_mutex, portMAX_DELAY);

    for (uint32 i = 0; i < OS_MAX_TASKS; i++)
    {
        if (!OS_task_table[i].is_used)
        {
            *slot = i;
            xSemaphoreGive(task_table_mutex);
            return OS_SUCCESS;
        }
    }

    xSemaphoreGive(task_table_mutex);
    return OS_ERR_NO_FREE_IDS;
}

/*
 * API实现
 */
int32 OS_TaskCreate(osal_id_t *task_id,
                    const char *task_name,
                    osal_task_entry function_pointer,
                    uint32 *stack_pointer,
                    uint32 stack_size,
                    uint32 priority,
                    uint32 flags)
{
    uint32 slot;
    int32 ret;
    task_wrapper_arg_t *wrapper_arg;
    BaseType_t xRet;

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
    xSemaphoreTake(task_table_mutex, portMAX_DELAY);
    for (uint32 i = 0; i < OS_MAX_TASKS; i++)
    {
        if (OS_task_table[i].is_used &&
            strcmp(OS_task_table[i].name, task_name) == 0)
        {
            xSemaphoreGive(task_table_mutex);
            return OS_ERR_NAME_TAKEN;
        }
    }

    /* 分配包装器参数 */
    wrapper_arg = pvPortMalloc(sizeof(task_wrapper_arg_t));
    if (wrapper_arg == NULL)
    {
        xSemaphoreGive(task_table_mutex);
        return OS_ERROR;
    }
    wrapper_arg->entry_func = function_pointer;

    /* 转换优先级 (OSAL: 1-255, FreeRTOS: 0-configMAX_PRIORITIES-1) */
    UBaseType_t freertos_priority = (configMAX_PRIORITIES - 1) -
                                    (priority * configMAX_PRIORITIES / 256);

    /* 转换栈大小 (字节 -> 字) */
    uint32 stack_words = (stack_size > 0) ? (stack_size / sizeof(StackType_t)) :
                                            configMINIMAL_STACK_SIZE;

    /* 创建FreeRTOS任务 */
    xRet = xTaskCreate(task_wrapper,
                       task_name,
                       stack_words,
                       wrapper_arg,
                       freertos_priority,
                       &OS_task_table[slot].handle);

    if (xRet != pdPASS)
    {
        vPortFree(wrapper_arg);
        xSemaphoreGive(task_table_mutex);
        return OS_ERROR;
    }

    /* 填充任务表 */
    OS_task_table[slot].is_used = true;
    OS_task_table[slot].id = next_task_id++;
    strncpy(OS_task_table[slot].name, task_name, OS_MAX_API_NAME - 1);
    OS_task_table[slot].priority = priority;
    OS_task_table[slot].stack_size = stack_size;

    *task_id = OS_task_table[slot].id;

    xSemaphoreGive(task_table_mutex);

    return OS_SUCCESS;
}

int32 OS_TaskDelete(osal_id_t task_id)
{
    xSemaphoreTake(task_table_mutex, portMAX_DELAY);

    for (uint32 i = 0; i < OS_MAX_TASKS; i++)
    {
        if (OS_task_table[i].is_used && OS_task_table[i].id == task_id)
        {
            vTaskDelete(OS_task_table[i].handle);
            OS_task_table[i].is_used = false;
            xSemaphoreGive(task_table_mutex);
            return OS_SUCCESS;
        }
    }

    xSemaphoreGive(task_table_mutex);
    return OS_ERR_INVALID_ID;
}

int32 OS_TaskDelay(uint32 millisecond)
{
    TickType_t ticks = pdMS_TO_TICKS(millisecond);
    vTaskDelay(ticks);
    return OS_SUCCESS;
}

osal_id_t OS_TaskGetId(void)
{
    TaskHandle_t self = xTaskGetCurrentTaskHandle();

    xSemaphoreTake(task_table_mutex, portMAX_DELAY);

    for (uint32 i = 0; i < OS_MAX_TASKS; i++)
    {
        if (OS_task_table[i].is_used &&
            OS_task_table[i].handle == self)
        {
            osal_id_t id = OS_task_table[i].id;
            xSemaphoreGive(task_table_mutex);
            return id;
        }
    }

    xSemaphoreGive(task_table_mutex);
    return OS_OBJECT_ID_UNDEFINED;
}

int32 OS_TaskGetIdByName(osal_id_t *task_id, const char *task_name)
{
    if (task_id == NULL || task_name == NULL)
        return OS_INVALID_POINTER;

    xSemaphoreTake(task_table_mutex, portMAX_DELAY);

    for (uint32 i = 0; i < OS_MAX_TASKS; i++)
    {
        if (OS_task_table[i].is_used &&
            strcmp(OS_task_table[i].name, task_name) == 0)
        {
            *task_id = OS_task_table[i].id;
            xSemaphoreGive(task_table_mutex);
            return OS_SUCCESS;
        }
    }

    xSemaphoreGive(task_table_mutex);
    return OS_ERR_NAME_NOT_FOUND;
}

int32 OS_TaskSetPriority(osal_id_t task_id, uint32 priority)
{
    if (priority < OS_TASK_PRIORITY_MIN || priority > OS_TASK_PRIORITY_MAX)
        return OS_ERR_INVALID_PRIORITY;

    xSemaphoreTake(task_table_mutex, portMAX_DELAY);

    for (uint32 i = 0; i < OS_MAX_TASKS; i++)
    {
        if (OS_task_table[i].is_used && OS_task_table[i].id == task_id)
        {
            UBaseType_t freertos_priority = (configMAX_PRIORITIES - 1) -
                                            (priority * configMAX_PRIORITIES / 256);

            vTaskPrioritySet(OS_task_table[i].handle, freertos_priority);
            OS_task_table[i].priority = priority;

            xSemaphoreGive(task_table_mutex);
            return OS_SUCCESS;
        }
    }

    xSemaphoreGive(task_table_mutex);
    return OS_ERR_INVALID_ID;
}

int32 OS_TaskGetInfo(osal_id_t task_id, OS_TaskProp_t *task_prop)
{
    if (task_prop == NULL)
        return OS_INVALID_POINTER;

    xSemaphoreTake(task_table_mutex, portMAX_DELAY);

    for (uint32 i = 0; i < OS_MAX_TASKS; i++)
    {
        if (OS_task_table[i].is_used && OS_task_table[i].id == task_id)
        {
            task_prop->priority = OS_task_table[i].priority;
            task_prop->stack_size = OS_task_table[i].stack_size;
            xSemaphoreGive(task_table_mutex);
            return OS_SUCCESS;
        }
    }

    xSemaphoreGive(task_table_mutex);
    return OS_ERR_INVALID_ID;
}

/*
 * OSAL初始化 (FreeRTOS)
 */
int32 OS_API_Init(void)
{
    /* 创建任务表互斥锁 */
    task_table_mutex = xSemaphoreCreateMutex();
    if (task_table_mutex == NULL)
        return OS_ERROR;

    /* 初始化任务表 */
    for (uint32 i = 0; i < OS_MAX_TASKS; i++)
    {
        OS_task_table[i].is_used = false;
    }

    /* 初始化其他OSAL组件... */

    return OS_SUCCESS;
}

void OS_API_Teardown(void)
{
    /* FreeRTOS通常不需要清理 */
}

int32 OS_IdleLoop(void)
{
    /* 启动FreeRTOS调度器 */
    vTaskStartScheduler();

    /* 不应该到达这里 */
    return OS_ERROR;
}
