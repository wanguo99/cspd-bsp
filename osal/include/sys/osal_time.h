/************************************************************************
 * OSAL - 时间延迟操作封装
 *
 * 功能：
 * - 封装sleep/usleep/nanosleep等延迟函数
 * - 提供统一的时间延迟接口
 * - 便于RTOS移植
 *
 * 设计原则：
 * - 使用固定大小类型（uint32/uint64）
 * - 提供多种时间精度的延迟接口
 * - 跨平台兼容
 ************************************************************************/

#ifndef OSAL_TIME_H
#define OSAL_TIME_H

#include "osal_types.h"

/*===========================================================================
 * 时间延迟接口
 *===========================================================================*/

/**
 * @brief 毫秒级延迟（推荐使用）
 * @param msec 延迟时间（毫秒）
 * @return 0成功，-1失败
 * @note 这是最常用的延迟接口，精度为毫秒
 */
int32 OSAL_msleep(uint32 msec);

/**
 * @brief 微秒级延迟
 * @param usec 延迟时间（微秒）
 * @return 0成功，-1失败
 * @note 精度为微秒，适用于需要高精度延迟的场景
 */
int32 OSAL_usleep(uint32 usec);

/**
 * @brief 秒级延迟
 * @param sec 延迟时间（秒）
 * @return 0成功，-1失败
 * @note 精度为秒，适用于长时间延迟
 */
int32 OSAL_sleep(uint32 sec);

/**
 * @brief 纳秒级延迟
 * @param nsec 延迟时间（纳秒）
 * @return 0成功，-1失败
 * @note 精度为纳秒，适用于极高精度延迟场景
 */
int32 OSAL_nanosleep(uint64 nsec);

/**
 * @brief 任务延迟（兼容接口）
 * @param millisecond 延迟时间（毫秒）
 * @return OS_SUCCESS成功，OS_ERROR失败
 * @note 这是为了兼容旧代码，推荐使用OSAL_msleep()
 */
int32 OSAL_TaskDelay(uint32 millisecond);

#endif /* OSAL_TIME_H */
