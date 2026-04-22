/************************************************************************
 * OSAL Linux实现 - 内存管理
 ************************************************************************/

#include "osal.h"

/* 内存API */
int32 OS_HeapGetInfo(uint32 *free_bytes, uint32 *total_bytes)
{
    /* Linux用户空间难以准确获取堆信息 */
    if (free_bytes != NULL)
        *free_bytes = 0;
    if (total_bytes != NULL)
        *total_bytes = 0;

    return OS_SUCCESS;
}
