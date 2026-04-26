/************************************************************************
 * OSAL POSIX实现 - 错误处理
 ************************************************************************/

#include "osal.h"

/* 错误处理API */
const char *OS_GetErrorName(int32 error_num)
{
    switch (error_num)
    {
        case OS_SUCCESS:                return "OS_SUCCESS";
        case OS_ERROR:                  return "OS_ERROR";
        case OS_INVALID_POINTER:        return "OS_INVALID_POINTER";
        case OS_ERROR_ADDRESS_MISALIGNED: return "OS_ERROR_ADDRESS_MISALIGNED";
        case OS_ERROR_TIMEOUT:          return "OS_ERROR_TIMEOUT";
        case OS_INVALID_INT_NUM:        return "OS_INVALID_INT_NUM";
        case OS_SEM_FAILURE:            return "OS_SEM_FAILURE";
        case OS_SEM_TIMEOUT:            return "OS_SEM_TIMEOUT";
        case OS_QUEUE_EMPTY:            return "OS_QUEUE_EMPTY";
        case OS_QUEUE_FULL:             return "OS_QUEUE_FULL";
        case OS_QUEUE_TIMEOUT:          return "OS_QUEUE_TIMEOUT";
        case OS_QUEUE_INVALID_SIZE:     return "OS_QUEUE_INVALID_SIZE";
        case OS_QUEUE_ID_ERROR:         return "OS_QUEUE_ID_ERROR";
        case OS_ERR_NAME_TOO_LONG:      return "OS_ERR_NAME_TOO_LONG";
        case OS_ERR_NO_FREE_IDS:        return "OS_ERR_NO_FREE_IDS";
        case OS_ERR_NAME_TAKEN:         return "OS_ERR_NAME_TAKEN";
        case OS_ERR_INVALID_ID:         return "OS_ERR_INVALID_ID";
        case OS_ERR_NAME_NOT_FOUND:     return "OS_ERR_NAME_NOT_FOUND";
        case OS_ERR_SEM_NOT_FULL:       return "OS_ERR_SEM_NOT_FULL";
        case OS_ERR_INVALID_PRIORITY:   return "OS_ERR_INVALID_PRIORITY";
        case OS_INVALID_SEM_VALUE:      return "OS_INVALID_SEM_VALUE";
        case OS_ERR_FILE:               return "OS_ERR_FILE";
        case OS_ERR_NOT_IMPLEMENTED:    return "OS_ERR_NOT_IMPLEMENTED";
        case OS_TIMER_ERR_INVALID_ARGS: return "OS_TIMER_ERR_INVALID_ARGS";
        case OS_TIMER_ERR_TIMER_ID:     return "OS_TIMER_ERR_TIMER_ID";
        case OS_TIMER_ERR_UNAVAILABLE:  return "OS_TIMER_ERR_UNAVAILABLE";
        case OS_TIMER_ERR_INTERNAL:     return "OS_TIMER_ERR_INTERNAL";
        case OS_ERR_INVALID_SIZE:       return "OS_ERR_INVALID_SIZE";
        case OS_ERR_NO_MEMORY:          return "OS_ERR_NO_MEMORY";
        case OS_ERR_BUSY:               return "OS_ERR_BUSY";
        case OS_ERR_PERMISSION:         return "OS_ERR_PERMISSION";
        case OS_ERR_NOT_SUPPORTED:      return "OS_ERR_NOT_SUPPORTED";
        case OS_ERR_ALREADY_EXISTS:     return "OS_ERR_ALREADY_EXISTS";
        case OS_ERR_WOULD_BLOCK:        return "OS_ERR_WOULD_BLOCK";
        case OS_ERR_INTERRUPTED:        return "OS_ERR_INTERRUPTED";
        case OS_ERR_BAD_ADDRESS:        return "OS_ERR_BAD_ADDRESS";
        case OS_ERR_INVALID_STATE:      return "OS_ERR_INVALID_STATE";
        case OS_ERR_RESOURCE_LIMIT:     return "OS_ERR_RESOURCE_LIMIT";
        default:                        return "UNKNOWN_ERROR";
    }
}
