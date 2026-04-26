/************************************************************************
 * OSAL Linux实现 - 环境变量操作
 ************************************************************************/

#include "sys/osal_env.h"
#include <stdlib.h>

str_t *OSAL_getenv(const str_t *name)
{
    return (str_t *)getenv((const char *)name);
}

int32 OSAL_setenv(const str_t *name, const str_t *value, int32 overwrite)
{
    return (int32)setenv((const char *)name, (const char *)value, overwrite);
}

int32 OSAL_unsetenv(const str_t *name)
{
    return (int32)unsetenv((const char *)name);
}
