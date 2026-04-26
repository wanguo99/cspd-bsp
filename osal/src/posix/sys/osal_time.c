/************************************************************************
 * OSAL POSIX实现 - 时间延迟操作
 ************************************************************************/

#define _DEFAULT_SOURCE  /* 启用usleep等函数 */
#include "sys/osal_time.h"
#include <unistd.h>
#include <time.h>
#include <errno.h>

int32 OSAL_msleep(uint32 msec)
{
    return usleep(msec * 1000);
}

int32 OSAL_usleep(uint32 usec)
{
    return usleep(usec);
}

int32 OSAL_sleep(uint32 sec)
{
    return (int32)sleep(sec);
}

int32 OSAL_nanosleep(uint64 nsec)
{
    struct timespec req;
    req.tv_sec = nsec / 1000000000ULL;
    req.tv_nsec = nsec % 1000000000ULL;
    return nanosleep(&req, NULL);
}

int32 OSAL_TaskDelay(uint32 millisecond)
{
    struct timespec ts;
    ts.tv_sec = millisecond / 1000;
    ts.tv_nsec = (millisecond % 1000) * 1000000;

    /* 使用nanosleep并处理中断 */
    while (nanosleep(&ts, &ts) == -1)
    {
        if (errno != EINTR)
            return OS_ERROR;
    }

    return OS_SUCCESS;
}
