/************************************************************************
 * 互斥锁API
 ************************************************************************/

#ifndef OSAPI_MUTEX_H
#define OSAPI_MUTEX_H

#include "common_types.h"

/**
 * @brief 创建互斥锁
 *
 * @param[out] mutex_id   返回的互斥锁ID
 * @param[in]  mutex_name 互斥锁名称
 * @param[in]  flags      保留,传0
 *
 * @return OS_SUCCESS 成功
 * @return OS_INVALID_POINTER mutex_id为NULL
 * @return OS_ERR_NAME_TOO_LONG 名称过长
 * @return OS_ERR_NO_FREE_IDS 无可用互斥锁ID
 * @return OS_ERR_NAME_TAKEN 名称已被使用
 * @return OS_ERROR 其他错误
 */
int32 OS_MutexCreate(osal_id_t *mutex_id, const char *mutex_name, uint32 flags);

/**
 * @brief 删除互斥锁
 *
 * @param[in] mutex_id 互斥锁ID
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERR_INVALID_ID 无效的互斥锁ID
 * @return OS_ERROR 删除失败
 */
int32 OS_MutexDelete(osal_id_t mutex_id);

/**
 * @brief 获取互斥锁
 *
 * @param[in] mutex_id 互斥锁ID
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERR_INVALID_ID 无效的互斥锁ID
 * @return OS_ERROR 获取失败
 */
int32 OS_MutexLock(osal_id_t mutex_id);

/**
 * @brief 释放互斥锁
 *
 * @param[in] mutex_id 互斥锁ID
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERR_INVALID_ID 无效的互斥锁ID
 * @return OS_ERROR 释放失败
 */
int32 OS_MutexUnlock(osal_id_t mutex_id);

/**
 * @brief 根据名称获取互斥锁ID
 *
 * @param[out] mutex_id   返回的互斥锁ID
 * @param[in]  mutex_name 互斥锁名称
 *
 * @return OS_SUCCESS 成功
 * @return OS_INVALID_POINTER mutex_id为NULL
 * @return OS_ERR_NAME_NOT_FOUND 未找到互斥锁
 */
int32 OS_MutexGetIdByName(osal_id_t *mutex_id, const char *mutex_name);

#endif /* OSAPI_MUTEX_H */
