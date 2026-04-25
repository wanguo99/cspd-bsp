/************************************************************************
 * OSAL - errno系统调用封装实现（Linux）
 ************************************************************************/

#include "lib/osal_errno.h"
#include <errno.h>
#include <string.h>

/*===========================================================================
 * errno访问函数实现
 *===========================================================================*/

int32 OSAL_GetErrno(void)
{
    return errno;
}

void OSAL_SetErrno(int32 err)
{
    errno = err;
}

const char *OSAL_StrError(int32 errnum)
{
    return strerror(errnum);
}
