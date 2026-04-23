/************************************************************************
 * 内存管理和错误处理API
 ************************************************************************/

#ifndef OSAPI_HEAP_H
#define OSAPI_HEAP_H

#include "osa_types.h"

/**
 * @brief 获取堆内存信息
 *
 * @param[out] free_bytes  可用内存(字节)
 * @param[out] total_bytes 总内存(字节)
 *
 * @return OS_SUCCESS 成功
 */
int32 OS_HeapGetInfo(uint32 *free_bytes, uint32 *total_bytes);

#endif /* OSAPI_HEAP_H */
