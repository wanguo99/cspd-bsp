/************************************************************************
 * 互斥锁API
 ************************************************************************/

#ifndef OSAPI_MUTEX_H
#define OSAPI_MUTEX_H

#include "osal_types.h"

/* 死锁检测超时阈值（毫秒） */
#define OSAL_MUTEX_DEADLOCK_TIMEOUT_MSEC  5000U  /* 5秒 */

/**
 * @brief 创建互斥锁
 *
 * @param[out] mutex_id   返回的互斥锁ID
 * @param[in]  mutex_name 互斥锁名称
 * @param[in]  flags      保留,传0
 *
 * @return OSAL_SUCCESS 成功
 * @return OSAL_ERR_INVALID_POINTER mutex_id为NULL
 * @return OSAL_ERR_NAME_TOO_LONG 名称过长
 * @return OSAL_ERR_NO_FREE_IDS 无可用互斥锁ID
 * @return OSAL_ERR_NAME_TAKEN 名称已被使用
 * @return OSAL_ERR_GENERIC 其他错误
 */
int32_t OSAL_MutexCreate(osal_id_t *mutex_id, const str_t *mutex_name, uint32_t flags);

/**
 * @brief 删除互斥锁
 *
 * @param[in] mutex_id 互斥锁ID
 *
 * @return OSAL_SUCCESS 成功
 * @return OSAL_ERR_INVALID_ID 无效的互斥锁ID
 * @return OSAL_ERR_GENERIC 删除失败
 */
int32_t OSAL_MutexDelete(osal_id_t mutex_id);

/**
 * @brief 获取互斥锁
 *
 * @param[in] mutex_id 互斥锁ID
 *
 * @return OSAL_SUCCESS 成功
 * @return OSAL_ERR_INVALID_ID 无效的互斥锁ID
 * @return OSAL_ERR_GENERIC 获取失败
 */
int32_t OSAL_MutexLock(osal_id_t mutex_id);

/**
 * @brief 释放互斥锁
 *
 * @param[in] mutex_id 互斥锁ID
 *
 * @return OSAL_SUCCESS 成功
 * @return OSAL_ERR_INVALID_ID 无效的互斥锁ID
 * @return OSAL_ERR_GENERIC 释放失败
 */
int32_t OSAL_MutexUnlock(osal_id_t mutex_id);

/**
 * @brief 根据名称获取互斥锁ID
 *
 * @param[out] mutex_id   返回的互斥锁ID
 * @param[in]  mutex_name 互斥锁名称
 *
 * @return OSAL_SUCCESS 成功
 * @return OSAL_ERR_INVALID_POINTER mutex_id为NULL
 * @return OSAL_ERR_NAME_NOT_FOUND 未找到互斥锁
 */
int32_t OSAL_MutexGetIdByName(osal_id_t *mutex_id, const str_t *mutex_name);

/**
 * @brief 带超时的互斥锁获取
 *
 * @param[in] mutex_id      互斥锁ID
 * @param[in] timeout_msec  超时时间(毫秒)
 *
 * @return OSAL_SUCCESS 成功
 * @return OSAL_ERR_INVALID_ID 无效的互斥锁ID
 * @return OSAL_ERR_TIMEOUT 超时
 * @return OSAL_ERR_GENERIC 其他错误
 */
int32_t OSAL_MutexLockTimeout(osal_id_t mutex_id, uint32_t timeout_msec);

/**
 * @brief 死锁检测回调函数类型
 *
 * @param[in] mutex_name 互斥锁名称
 * @param[in] wait_time  等待时间(毫秒)
 */
typedef void (*deadlock_callback_t)(const str_t *mutex_name, uint32_t wait_time);

/**
 * @brief 设置死锁检测阈值和回调
 *
 * @param[in] threshold_msec 死锁检测阈值(毫秒)
 * @param[in] callback       死锁检测回调函数
 *
 * @return OSAL_SUCCESS 成功
 */
int32_t OSAL_MutexSetDeadlockDetection(uint32_t threshold_msec, deadlock_callback_t callback);

#endif /* OSAPI_MUTEX_H */
