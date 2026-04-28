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
    return usleep(msec * OSAL_USEC_PER_MSEC);
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
    req.tv_sec = nsec / OSAL_NSEC_PER_SEC;
    req.tv_nsec = nsec % OSAL_NSEC_PER_SEC;
    return nanosleep(&req, NULL);
}

int32_t OSAL_TaskDelay(uint32_t millisecond)
{
    struct timespec ts;
    ts.tv_sec = millisecond / OSAL_MSEC_PER_SEC;
    ts.tv_nsec = (millisecond % OSAL_MSEC_PER_SEC) * OSAL_NSEC_PER_MSEC;

    /* 使用nanosleep并处理中断 */
    while (nanosleep(&ts, &ts) == -1)
    {
        if (errno != EINTR)
            return OSAL_ERR_GENERIC;
    }

    return OSAL_SUCCESS;
}
