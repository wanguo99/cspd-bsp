/************************************************************************
 * 任务管理API
 ************************************************************************/

#ifndef OSAPI_TASK_H
#define OSAPI_TASK_H

#include "osa_types.h"

/*
 * 任务入口函数类型
 */
typedef void (*osal_task_entry)(void *arg);

/*
 * 任务属性
 */
typedef struct
{
    uint32 stack_size;    /* 栈大小(字节) */
    uint32 priority;      /* 优先级 1-255 */
} OS_TaskProp_t;

/**
 * @brief 创建任务
 *
 * @param[out] task_id      返回的任务ID
 * @param[in]  task_name    任务名称(最大OS_MAX_API_NAME字符)
 * @param[in]  function_pointer 任务入口函数
 * @param[in]  stack_pointer    栈指针(可为NULL,由系统分配)
 * @param[in]  stack_size       栈大小(字节)
 * @param[in]  priority         优先级(1-255, 数字越小优先级越高)
 * @param[in]  flags            保留,传0
 *
 * @return OS_SUCCESS 成功
 * @return OS_INVALID_POINTER task_id或function_pointer为NULL
 * @return OS_ERR_NAME_TOO_LONG 名称过长
 * @return OS_ERR_INVALID_PRIORITY 优先级无效
 * @return OS_ERR_NO_FREE_IDS 无可用任务ID
 * @return OS_ERR_NAME_TAKEN 名称已被使用
 * @return OS_ERROR 其他错误
 */
int32 OS_TaskCreate(osal_id_t *task_id,
                    const char *task_name,
                    osal_task_entry function_pointer,
                    uint32 *stack_pointer,
                    uint32 stack_size,
                    uint32 priority,
                    uint32 flags);

/**
 * @brief 删除任务
 *
 * @param[in] task_id 任务ID
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERR_INVALID_ID 无效的任务ID
 * @return OS_ERROR 删除失败
 */
int32 OS_TaskDelete(osal_id_t task_id);

/**
 * @brief 任务延时
 *
 * @param[in] millisecond 延时时间(毫秒)
 *
 * @return OS_SUCCESS 成功
 */
int32 OS_TaskDelay(uint32 millisecond);

/**
 * @brief 设置任务优先级
 *
 * @param[in] task_id  任务ID
 * @param[in] priority 新优先级
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERR_INVALID_ID 无效的任务ID
 * @return OS_ERR_INVALID_PRIORITY 无效的优先级
 */
int32 OS_TaskSetPriority(osal_id_t task_id, uint32 priority);

/**
 * @brief 获取当前任务ID
 *
 * @return 当前任务的ID
 */
osal_id_t OS_TaskGetId(void);

/**
 * @brief 根据名称获取任务ID
 *
 * @param[out] task_id   返回的任务ID
 * @param[in]  task_name 任务名称
 *
 * @return OS_SUCCESS 成功
 * @return OS_INVALID_POINTER task_id为NULL
 * @return OS_ERR_NAME_NOT_FOUND 未找到任务
 */
int32 OS_TaskGetIdByName(osal_id_t *task_id, const char *task_name);

/**
 * @brief 获取任务信息
 *
 * @param[in]  task_id 任务ID
 * @param[out] task_prop 任务属性
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERR_INVALID_ID 无效的任务ID
 * @return OS_INVALID_POINTER task_prop为NULL
 */
int32 OS_TaskGetInfo(osal_id_t task_id, OS_TaskProp_t *task_prop);

#endif /* OSAPI_TASK_H */
