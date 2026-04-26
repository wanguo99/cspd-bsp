/************************************************************************
 * OSAL - POSIX线程封装
 *
 * 功能：
 * - 封装pthread基本操作
 * - 1:1映射系统调用，不引入业务逻辑
 * - 使用固定大小类型，避免平台相关类型
 *
 * 设计原则：
 * - 提供标准pthread函数的封装
 * - 返回值与系统调用保持一致
 * - 便于RTOS移植
 *
 * 注意：
 * - 这是原始系统调用封装，用于测试等特殊场景
 * - 应用层应优先使用OSAL_TaskCreate等高层接口
 ************************************************************************/

#ifndef OSAL_THREAD_H
#define OSAL_THREAD_H

#include "osal_types.h"

/*
 * 线程类型（平台无关）
 */
typedef uint64_t osal_thread_t;

/*
 * 线程函数类型
 */
typedef void *(*osal_thread_func_t)(void *arg);

/**
 * @brief 创建线程
 *
 * @param[out] thread    线程句柄
 * @param[in]  attr      线程属性（NULL表示默认）
 * @param[in]  start_routine 线程函数
 * @param[in]  arg       传递给线程函数的参数
 *
 * @return 0 成功
 * @return 非0 错误码
 */
int32_t OSAL_pthread_create(osal_thread_t *thread,
                            void *attr,
                            osal_thread_func_t start_routine,
                            void *arg);

/**
 * @brief 等待线程结束
 *
 * @param[in]  thread   线程句柄
 * @param[out] retval   线程返回值（NULL表示不关心）
 *
 * @return 0 成功
 * @return 非0 错误码
 */
int32_t OSAL_pthread_join(osal_thread_t thread, void **retval);

#endif /* OSAL_THREAD_H */
