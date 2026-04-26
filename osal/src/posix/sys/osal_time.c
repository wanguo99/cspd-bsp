/************************************************************************
 * OSAL POSIX实现 - 时间延迟操作
 ************************************************************************/

#define _DEFAULT_SOURCE  /* 启用usleep等函数 */
#include "sys/osal_time.h"
#include <unistd.h>
#include <time.h>
#include <errno.h>

int32_t OSAL_msleep(uint32_t msec)
{
    return usleep(msec * 1000);
}

int32_t OSAL_usleep(uint32_t usec)
{
    return usleep(usec);
}

int32_t OSAL_sleep(uint32_t sec)
{
    return (int32_t)sleep(sec);
}

int32_t OSAL_nanosleep(uint64_t nsec)
{
    struct timespec req;
    req.tv_sec = nsec / 1000000000ULL;
    req.tv_nsec = nsec % 1000000000ULL;
    return nanosleep(&req, NULL);
}

int32_t OSAL_TaskDelay(uint32_t millisecond)
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
