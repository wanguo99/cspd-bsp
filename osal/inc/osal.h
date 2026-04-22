/************************************************************************
 * CSPD-BSP - Compute and Storage Payload Board Support Package
 *
 * 基于NASA cFS OSAL设计的轻量化操作系统抽象层
 *
 * 设计原则:
 * 1. 保留cFS OSAL的核心API设计
 * 2. 移除不常用的功能(文件系统、模块加载等)
 * 3. 简化实现，减少代码量
 * 4. 保持跨平台兼容性
 ************************************************************************/

#ifndef OSAL_H
#define OSAL_H

#include "common_types.h"
#include "osapi_error.h"
#include "osapi_task.h"
#include "osapi_queue.h"
#include "osapi_mutex.h"
#include "osapi_clock.h"
#include "osapi_network.h"
#include "osapi_heap.h"
#include "osapi_log.h"

/*
 * OSAL版本信息
 */
#define OSAL_LITE_VERSION_MAJOR  1
#define OSAL_LITE_VERSION_MINOR  0
#define OSAL_LITE_VERSION_PATCH  0

/*
 * 初始化和关闭
 */

/**
 * @brief 初始化OSAL
 *
 * 必须在使用任何OSAL功能前调用
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR   失败
 */
int32 OS_API_Init(void);

/**
 * @brief 关闭OSAL
 *
 * 清理所有OSAL资源
 *
 * @return OS_SUCCESS 成功
 */
int32 OS_API_Teardown(void);

/**
 * @brief 启动多任务调度
 *
 * 此函数不会返回(除非发生错误)
 *
 * @return OS_ERROR 启动失败
 */
int32 OS_IdleLoop(void);

/**
 * @brief 获取OSAL版本字符串
 *
 * @return 版本字符串，例如 "1.0.0"
 */
const char *OS_GetVersionString(void);

#endif /* OSAL_H */
