/************************************************************************
 * OSAL - string系统调用封装实现（POSIX）
 ************************************************************************/

#include "lib/osal_string.h"
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

/*
 * 内存操作
 */

void* OSAL_Memset(void *ptr, int32_t value, osal_size_t size)
{
    size_t mem_size = size;
    return memset(ptr, value, mem_size);
}

void* OSAL_Memcpy(void *dest, const void *src, osal_size_t size)
{
    size_t mem_size = size;
    return memcpy(dest, src, mem_size);
}

void* OSAL_Memmove(void *dest, const void *src, osal_size_t size)
{
    size_t mem_size = size;
    return memmove(dest, src, mem_size);
}

int32_t OSAL_Memcmp(const void *ptr1, const void *ptr2, osal_size_t size)
{
    size_t mem_size = size;
    return memcmp(ptr1, ptr2, mem_size);
}

/*
 * 字符串操作
 */

osal_size_t OSAL_Strlen(const char *str)
{
    size_t len = strlen(str);
    return len;
}

int32_t OSAL_Strcmp(const char *str1, const char *str2)
{
    return strcmp(str1, str2);
}

int32_t OSAL_Strncmp(const char *str1, const char *str2, osal_size_t n)
{
    size_t cmp_len = n;
    return strncmp(str1, str2, cmp_len);
}

int32_t OSAL_Strcasecmp(const char *str1, const char *str2)
{
    return strcasecmp(str1, str2);
}

char* OSAL_Strcpy(char *dest, const char *src)
{
    return strcpy(dest, src);
}

char* OSAL_Strncpy(char *dest, const char *src, osal_size_t n)
{
    size_t copy_len = n;
    return strncpy(dest, src, copy_len);
}

char* OSAL_Strcat(char *dest, const char *src)
{
    return strcat(dest, src);
}

char* OSAL_Strncat(char *dest, const char *src, osal_size_t n)
{
    size_t cat_len = n;
    return strncat(dest, src, cat_len);
}

char* OSAL_Strstr(const char *haystack, const char *needle)
{
    return strstr(haystack, needle);
}

osal_size_t OSAL_Strcspn(const char *str, const char *reject)
{
    size_t result = strcspn(str, reject);
    return result;
}

/*
 * 字符串格式化
 */

int32_t OSAL_Sprintf(char *str, const char *format, ...)
{
    va_list args;
    int32_t ret;

    va_start(args, format);
    ret = vsprintf(str, format, args);
    va_end(args);

    return ret;
}

int32_t OSAL_Snprintf(char *str, osal_size_t size, const char *format, ...)
{
    va_list args;
    int32_t ret;

    va_start(args, format);
    size_t buf_size = size;
    ret = vsnprintf(str, buf_size, format, args);
    va_end(args);

    return ret;
}

int32_t OSAL_Sscanf(const char *str, const char *format, ...)
{
    va_list args;
    int32_t ret;

    va_start(args, format);
    ret = vsscanf(str, format, args);
    va_end(args);

    return ret;
}

/*
 * 字符串转换
 */

int32_t OSAL_Atoi(const char *str)
{
    return atoi(str);
}

int64_t OSAL_Atol(const char *str)
{
    long result = atol(str);
    return result;
}

int64_t OSAL_Strtol(const char *str, char **endptr, int32_t base)
{
    long result = strtol(str, endptr, base);
    return result;
}
