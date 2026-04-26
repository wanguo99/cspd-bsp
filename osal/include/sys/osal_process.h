/************************************************************************
 * OSAL - 进程控制系统调用封装
 *
 * 功能：
 * - 封装进程控制函数
 * - 1:1映射系统调用，不引入业务逻辑
 * - 使用固定大小类型，避免平台相关类型
 *
 * 设计原则：
 * - 提供标准进程控制函数的封装
 * - 返回值与系统调用保持一致
 * - 便于RTOS移植
 ************************************************************************/

#ifndef OSAL_PROCESS_H
#define OSAL_PROCESS_H

#include "osal_types.h"

/*
 * 进程控制函数
 */
void OSAL_Exit(int32_t status);
int32_t OSAL_Getpid(void);
int32_t OSAL_Kill(int32_t pid, int32_t sig);
void OSAL_Abort(void);

#endif /* OSAL_PROCESS_H */
