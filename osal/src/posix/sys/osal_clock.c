/************************************************************************
 * OSAL POSIX实现 - 时间服务
 ************************************************************************/

#include "osal.h"
#include <stdint.h>
#include <sys/time.h>
#include <time.h>

/* 时间API */
int32_t OSAL_GetLocalTime(OS_time_t *time_struct)
{
    struct timeval tv;

    if (NULL == time_struct)
        return OSAL_ERR_INVALID_POINTER;

    gettimeofday(&tv, NULL);
    time_struct->seconds = tv.tv_sec;
    time_struct->microsecs = tv.tv_usec;

    return OSAL_SUCCESS;
}

int32_t OSAL_SetLocalTime(const OS_time_t *time_struct __attribute__((unused)))
{
    /* Linux用户空间通常无权设置系统时间 */
    return OSAL_ERR_NOT_IMPLEMENTED;
}

uint32_t OSAL_GetTickCount(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    /* 防止溢出：使用64位计算后再转换 */
    uint64_t ms = ((uint64_t)ts.tv_sec * OSAL_MS_PER_SEC) + (ts.tv_nsec / OSAL_NS_PER_MS);

    /* 返回低32位（约49.7天后会回绕） */
    return (uint32_t)ms;
}

int32_t OSAL_Milli2Ticks(uint32_t milliseconds, uint32_t *ticks)
{
    if (NULL == ticks)
        return OSAL_ERR_INVALID_POINTER;

    *ticks = milliseconds;
    return OSAL_SUCCESS;
}
