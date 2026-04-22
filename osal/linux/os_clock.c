/************************************************************************
 * OSAL Linux实现 - 时间服务
 ************************************************************************/

#include "osal.h"
#include <sys/time.h>
#include <time.h>

/* 时间API */
int32 OS_GetLocalTime(OS_time_t *time_struct)
{
    struct timeval tv;

    if (time_struct == NULL)
        return OS_INVALID_POINTER;

    gettimeofday(&tv, NULL);
    time_struct->seconds = tv.tv_sec;
    time_struct->microsecs = tv.tv_usec;

    return OS_SUCCESS;
}

int32 OS_SetLocalTime(const OS_time_t *time_struct __attribute__((unused)))
{
    /* Linux用户空间通常无权设置系统时间 */
    return OS_ERR_NOT_IMPLEMENTED;
}

uint32 OS_GetTickCount(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
}

int32 OS_Milli2Ticks(uint32 milliseconds, uint32 *ticks)
{
    if (ticks == NULL)
        return OS_INVALID_POINTER;

    *ticks = milliseconds;
    return OS_SUCCESS;
}
