/************************************************************************
 * 信号处理实现 (Linux)
 ************************************************************************/

#include "osal.h"
#include <signal.h>
#include <string.h>
#include <errno.h>

int32 OSAL_SignalRegister(int32 signum, os_signal_handler_t handler)
{
    struct sigaction sa;

    if (handler == NULL)
        return OS_INVALID_POINTER;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = (void (*)(int))handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(signum, &sa, NULL) < 0)
    {
        OSAL_Printf("[OSAL] sigaction失败, signum=%d, errno=%d\n", signum, errno);
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

int32 OSAL_SignalIgnore(int32 signum)
{
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(signum, &sa, NULL) < 0)
    {
        OSAL_Printf("[OSAL] sigaction(SIG_IGN)失败, signum=%d, errno=%d\n", signum, errno);
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

int32 OSAL_SignalDefault(int32 signum)
{
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(signum, &sa, NULL) < 0)
    {
        OSAL_Printf("[OSAL] sigaction(SIG_DFL)失败, signum=%d, errno=%d\n", signum, errno);
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

int32 OSAL_SignalBlock(int32 signum)
{
    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set, signum);

    if (sigprocmask(SIG_BLOCK, &set, NULL) < 0)
    {
        OSAL_Printf("[OSAL] sigprocmask(SIG_BLOCK)失败, signum=%d, errno=%d\n", signum, errno);
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

int32 OSAL_SignalUnblock(int32 signum)
{
    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set, signum);

    if (sigprocmask(SIG_UNBLOCK, &set, NULL) < 0)
    {
        OSAL_Printf("[OSAL] sigprocmask(SIG_UNBLOCK)失败, signum=%d, errno=%d\n", signum, errno);
        return OS_ERROR;
    }

    return OS_SUCCESS;
}
