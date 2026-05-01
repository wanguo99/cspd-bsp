/************************************************************************
 * 任务管理API
 ************************************************************************/

#ifndef OSAPI_TASK_H
#define OSAPI_TASK_H

#include "osal_types.h"
#include <stdbool.h>

/* 任务栈大小配置 */
#define OSAL_TASK_STACK_SIZE_SMALL   (32 * 1024)   /* 32KB */
#define OSAL_TASK_STACK_SIZE_MEDIUM  (64 * 1024)   /* 64KB */
#define OSAL_TASK_STACK_SIZE_LARGE   (128 * 1024)  /* 128KB */

/* 任务优先级定义（数字越小优先级越高） */
#define OSAL_TASK_PRIORITY_CRITICAL  10   /* 关键任务 */
#define OSAL_TASK_PRIORITY_HIGH      50   /* 高优先级 */
#define OSAL_TASK_PRIORITY_NORMAL    100  /* 普通优先级 */
#define OSAL_TASK_PRIORITY_LOW       150  /* 低优先级 */
#define OSAL_TASK_PRIORITY_IDLE      200  /* 空闲任务 */

/* 任务删除超时时间（秒） */
#define OSAL_TASK_DELETE_TIMEOUT_SEC 5    /* 等待任务退出的超时时间 */

typedef void (*osal_task_entry)(void *arg);

typedef struct
{
    uint32_t stack_size;
    uint32_t priority;      /* 1-255 */
} OS_TaskProp_t;

/**
 * @brief 创建任务
 *
 * @param[out] task_id      任务ID
 * @param[in]  task_name    任务名称(最大OS_MAX_API_NAME字符)
 * @param[in]  function_pointer 任务入口函数
 * @param[in]  user_arg         用户参数(传递给任务入口函数)
 * @param[in]  stack_size       栈大小(字节)
 * @param[in]  priority         优先级(1-255, 数字越小优先级越高)
 * @param[in]  flags            保留,传0
 *
 * @return OSAL_SUCCESS 成功
 * @return OSAL_ERR_INVALID_POINTER task_id或function_pointer为NULL
 * @return OSAL_ERR_NAME_TOO_LONG 名称过长
 * @return OSAL_ERR_INVALID_PRIORITY 优先级无效
 * @return OSAL_ERR_NO_FREE_IDS 无可用任务ID
 * @return OSAL_ERR_NAME_TAKEN 名称已被使用
 * @return OSAL_ERR_GENERIC 其他错误
 */
int32_t OSAL_TaskCreate(osal_id_t *task_id,
                    const char *task_name,
                    osal_task_entry function_pointer,
                    void *user_arg,
                    uint32_t stack_size,
                    uint32_t priority,
                    uint32_t flags);

/**
 * @brief 删除任务
 *
 * @return OSAL_SUCCESS 成功
 * @return OSAL_ERR_INVALID_ID 无效的任务ID
 */
int32_t OSAL_TaskDelete(osal_id_t task_id);

/**
 * @brief 设置任务优先级
 *
 * @return OSAL_SUCCESS 成功
 * @return OSAL_ERR_INVALID_ID 无效的任务ID
 * @return OSAL_ERR_INVALID_PRIORITY 无效的优先级
 */
int32_t OSAL_TaskSetPriority(osal_id_t task_id, uint32_t priority);

/**
 * @brief 获取当前任务ID
 */
osal_id_t OSAL_TaskGetId(void);

/**
 * @brief 检查当前任务是否应该关闭
 * @return true 任务应该关闭
 * @return false 任务继续运行
 */
bool OSAL_TaskShouldShutdown(void);

/**
 * @brief 根据名称获取任务ID
 *
 * @return OSAL_SUCCESS 成功
 * @return OSAL_ERR_INVALID_POINTER task_id为NULL
 * @return OSAL_ERR_NAME_NOT_FOUND 未找到任务
 */
int32_t OSAL_TaskGetIdByName(osal_id_t *task_id, const char *task_name);

/**
 * @brief 获取任务信息
 *
 * @return OSAL_SUCCESS 成功
 * @return OSAL_ERR_INVALID_ID 无效的任务ID
 * @return OSAL_ERR_INVALID_POINTER task_prop为NULL
 */
int32_t OSAL_TaskGetInfo(osal_id_t task_id, OS_TaskProp_t *task_prop);

#endif /* OSAPI_TASK_H */
