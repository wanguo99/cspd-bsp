/************************************************************************
 * OSAL - select系统调用封装实现（Linux）
 ************************************************************************/

#include "osal_select.h"
#include <sys/select.h>
#include <string.h>

/*===========================================================================
 * fd_set操作实现
 *===========================================================================*/

void OSAL_FD_ZERO(osal_fd_set_t *set)
{
    memset(set, 0, sizeof(osal_fd_set_t));
}

void OSAL_FD_SET(int32 fd, osal_fd_set_t *set)
{
    if (fd >= 0 && fd < OSAL_FD_SETSIZE) {
        set->fds_bits[fd / 32] |= (1U << (fd % 32));
    }
}

void OSAL_FD_CLR(int32 fd, osal_fd_set_t *set)
{
    if (fd >= 0 && fd < OSAL_FD_SETSIZE) {
        set->fds_bits[fd / 32] &= ~(1U << (fd % 32));
    }
}

int32 OSAL_FD_ISSET(int32 fd, const osal_fd_set_t *set)
{
    if (fd >= 0 && fd < OSAL_FD_SETSIZE) {
        return (set->fds_bits[fd / 32] & (1U << (fd % 32))) ? 1 : 0;
    }
    return 0;
}

/*===========================================================================
 * select系统调用实现
 *===========================================================================*/

int32 OSAL_select(int32 nfds, osal_fd_set_t *readfds, osal_fd_set_t *writefds,
                  osal_fd_set_t *exceptfds, osal_timeval_t *timeout)
{
    struct timeval tv;
    struct timeval *ptv = NULL;

    if (timeout) {
        tv.tv_sec = timeout->tv_sec;
        tv.tv_usec = timeout->tv_usec;
        ptv = &tv;
    }

    int32 result = (int32)select(nfds, (fd_set *)readfds, (fd_set *)writefds,
                                 (fd_set *)exceptfds, ptv);

    if (timeout && ptv) {
        timeout->tv_sec = tv.tv_sec;
        timeout->tv_usec = tv.tv_usec;
    }

    return result;
}

int32 OSAL_pselect(int32 nfds, osal_fd_set_t *readfds, osal_fd_set_t *writefds,
                   osal_fd_set_t *exceptfds, const osal_timespec_t *timeout,
                   const void *sigmask)
{
    struct timespec ts;
    struct timespec *pts = NULL;

    if (timeout) {
        ts.tv_sec = timeout->tv_sec;
        ts.tv_nsec = timeout->tv_nsec;
        pts = &ts;
    }

    return (int32)pselect(nfds, (fd_set *)readfds, (fd_set *)writefds,
                         (fd_set *)exceptfds, pts, (const sigset_t *)sigmask);
}
