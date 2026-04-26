/************************************************************************
 * 内存管理和错误处理API
 ************************************************************************/

#ifndef OSAPI_HEAP_H
#define OSAPI_HEAP_H

#include "osal_types.h"
#include <stdbool.h>

/**
 * @brief 获取堆内存信息
 *
 * @param[out] free_bytes  可用内存(字节)
 * @param[out] total_bytes 总内存(字节)
 *
 * @return OS_SUCCESS 成功
 */
int32_t OSAL_HeapGetInfo(uint32_t *free_bytes, uint32_t *total_bytes);

/**
 * @brief 设置内存使用阈值
 *
 * @param[in] percent 阈值百分比(0-100)
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERR_INVALID_PARAM 参数无效
 */
int32_t OSAL_HeapSetThreshold(uint32_t percent);

/**
 * @brief 检查内存使用是否超过阈值
 *
 * @param[out] exceeded 是否超过阈值
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERR_INVALID_PARAM 参数无效
 */
int32_t OSAL_HeapCheckThreshold(bool *exceeded);

/**
 * @brief 获取内存统计信息
 *
 * @param[out] current 当前使用内存(字节)
 * @param[out] peak 峰值使用内存(字节)
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERR_INVALID_PARAM 参数无效
 */
int32_t OSAL_HeapGetStats(uint32_t *current, uint32_t *peak);

/**
 * @brief 分配内存
 *
 * @param[in] size 要分配的字节数
 *
 * @return 成功返回内存指针，失败返回NULL
 */
void *OSAL_Malloc(size_t size);

/**
 * @brief 释放内存
 *
 * @param[in] ptr 要释放的内存指针
 */
void OSAL_Free(void *ptr);

#endif /* OSAPI_HEAP_H */
