/************************************************************************
 * OSAL - 进程控制系统调用封装实现（POSIX）
 ************************************************************************/

#include "sys/osal_process.h"
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

/*
 * 进程控制函数
 */

void OSAL_Exit(int32_t status)
{
    exit(status);
}

int32_t OSAL_Getpid(void)
{
    return getpid();
}

int32_t OSAL_Kill(int32_t pid, int32_t sig)
{
    return kill(pid, sig);
}

void OSAL_Abort(void)
{
    abort();
}
