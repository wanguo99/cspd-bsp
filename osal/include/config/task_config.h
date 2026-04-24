/************************************************************************
 * 任务配置
 ************************************************************************/

#ifndef TASK_CONFIG_H
#define TASK_CONFIG_H

/* 任务栈大小 */
#define TASK_STACK_SIZE_SMALL   (32 * 1024)   /* 32KB */
#define TASK_STACK_SIZE_MEDIUM  (64 * 1024)   /* 64KB */
#define TASK_STACK_SIZE_LARGE   (128 * 1024)  /* 128KB */

/* 任务优先级定义 */
/* 优先级: 数字越小优先级越高 */
#define PRIORITY_CRITICAL       10   /* CAN网关 */
#define PRIORITY_HIGH           50   /* 协议转换 */
#define PRIORITY_NORMAL         100  /* 状态监控 */
#define PRIORITY_LOW            150  /* 日志、缓存 */
#define PRIORITY_IDLE           200  /* 后台任务 */

#endif /* TASK_CONFIG_H */
