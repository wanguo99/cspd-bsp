/************************************************************************
 * OSAL - errno系统调用封装实现（POSIX）
 ************************************************************************/

#include "lib/osal_errno.h"
#include "osal_types.h"
#include <errno.h>
#include <string.h>

/*===========================================================================
 * errno访问函数实现
 *===========================================================================*/

int32_t OSAL_GetErrno(void)
{
    return errno;
}

void OSAL_SetErrno(int32_t err)
{
    errno = err;
}

const char *OSAL_StrError(int32_t errnum)
{
    return strerror(errnum);
}

/*===========================================================================
 * OSAL状态码转字符串实现
 *===========================================================================*/

const str_t *OSAL_GetStatusName(int32_t status_code)
{
    switch (status_code)
    {
        case OSAL_SUCCESS:                return "OSAL_SUCCESS";
        case OSAL_ERR_GENERIC:                  return "OSAL_ERR_GENERIC";
        case OSAL_ERR_INVALID_POINTER:        return "OSAL_ERR_INVALID_POINTER";
        case OSAL_ERR_ADDRESS_MISALIGNED: return "OSAL_ERR_ADDRESS_MISALIGNED";
        case OSAL_ERR_TIMEOUT:          return "OSAL_ERR_TIMEOUT";
        case OSAL_ERR_INVALID_INT_NUM:        return "OSAL_ERR_INVALID_INT_NUM";
        case OSAL_ERR_SEM_FAILURE:            return "OSAL_ERR_SEM_FAILURE";
        case OSAL_ERR_SEM_TIMEOUT:            return "OSAL_ERR_SEM_TIMEOUT";
        case OSAL_ERR_QUEUE_EMPTY:            return "OSAL_ERR_QUEUE_EMPTY";
        case OSAL_ERR_QUEUE_FULL:             return "OSAL_ERR_QUEUE_FULL";
        case OSAL_ERR_QUEUE_TIMEOUT:          return "OSAL_ERR_QUEUE_TIMEOUT";
        case OSAL_ERR_QUEUE_INVALID_SIZE:     return "OSAL_ERR_QUEUE_INVALID_SIZE";
        case OSAL_ERR_QUEUE_ID:         return "OSAL_ERR_QUEUE_ID";
        case OSAL_ERR_NAME_TOO_LONG:      return "OSAL_ERR_NAME_TOO_LONG";
        case OSAL_ERR_NO_FREE_IDS:        return "OSAL_ERR_NO_FREE_IDS";
        case OSAL_ERR_NAME_TAKEN:         return "OSAL_ERR_NAME_TAKEN";
        case OSAL_ERR_INVALID_ID:         return "OSAL_ERR_INVALID_ID";
        case OSAL_ERR_NAME_NOT_FOUND:     return "OSAL_ERR_NAME_NOT_FOUND";
        case OSAL_ERR_SEM_NOT_FULL:       return "OSAL_ERR_SEM_NOT_FULL";
        case OSAL_ERR_INVALID_PRIORITY:   return "OSAL_ERR_INVALID_PRIORITY";
        case OSAL_ERR_INVALID_SEM_VALUE:      return "OSAL_ERR_INVALID_SEM_VALUE";
        case OSAL_ERR_FILE:               return "OSAL_ERR_FILE";
        case OSAL_ERR_NOT_IMPLEMENTED:    return "OSAL_ERR_NOT_IMPLEMENTED";
        case OSAL_ERR_TIMER_INVALID_ARGS: return "OSAL_ERR_TIMER_INVALID_ARGS";
        case OSAL_ERR_TIMER_ID:     return "OSAL_ERR_TIMER_ID";
        case OSAL_ERR_TIMER_UNAVAILABLE:  return "OSAL_ERR_TIMER_UNAVAILABLE";
        case OSAL_ERR_TIMER_INTERNAL:     return "OSAL_ERR_TIMER_INTERNAL";
        case OSAL_ERR_INVALID_SIZE:       return "OSAL_ERR_INVALID_SIZE";
        case OSAL_ERR_NO_MEMORY:          return "OSAL_ERR_NO_MEMORY";
        case OSAL_ERR_BUSY:               return "OSAL_ERR_BUSY";
        case OSAL_ERR_PERMISSION:         return "OSAL_ERR_PERMISSION";
        case OSAL_ERR_NOT_SUPPORTED:      return "OSAL_ERR_NOT_SUPPORTED";
        case OSAL_ERR_ALREADY_EXISTS:     return "OSAL_ERR_ALREADY_EXISTS";
        case OSAL_ERR_WOULD_BLOCK:        return "OSAL_ERR_WOULD_BLOCK";
        case OSAL_ERR_INTERRUPTED:        return "OSAL_ERR_INTERRUPTED";
        case OSAL_ERR_BAD_ADDRESS:        return "OSAL_ERR_BAD_ADDRESS";
        case OSAL_ERR_INVALID_STATE:      return "OSAL_ERR_INVALID_STATE";
        case OSAL_ERR_RESOURCE_LIMIT:     return "OSAL_ERR_RESOURCE_LIMIT";
        default:                        return "UNKNOWN_ERROR";
    }
}
