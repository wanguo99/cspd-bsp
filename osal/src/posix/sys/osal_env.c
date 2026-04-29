/************************************************************************
 * OSAL POSIX实现 - 环境变量操作
 ************************************************************************/

#include "sys/osal_env.h"
#include <stdlib.h>

str_t *OSAL_getenv(const str_t *name)
{
    char *result = getenv(name);
    return result;
}

int32_t OSAL_setenv(const str_t *name, const str_t *value, int32_t overwrite)
{
    int32_t result = setenv(name, value, overwrite);
    return result;
}

int32_t OSAL_unsetenv(const str_t *name)
{
    int32_t result = unsetenv(name);
    return result;
}
