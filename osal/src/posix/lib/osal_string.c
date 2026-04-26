/************************************************************************
 * OSAL - string系统调用封装实现（POSIX）
 ************************************************************************/

#include "lib/osal_string.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/*
 * 内存操作
 */

void* OSAL_Memset(void *ptr, int32_t value, osal_size_t size)
{
    return memset(ptr, value, (size_t)size);
}

void* OSAL_Memcpy(void *dest, const void *src, osal_size_t size)
{
    return memcpy(dest, src, (size_t)size);
}

void* OSAL_Memmove(void *dest, const void *src, osal_size_t size)
{
    return memmove(dest, src, (size_t)size);
}

int32_t OSAL_Memcmp(const void *ptr1, const void *ptr2, osal_size_t size)
{
    return memcmp(ptr1, ptr2, (size_t)size);
}

/*
 * 字符串操作
 */

osal_size_t OSAL_Strlen(const char *str)
{
    return (osal_size_t)strlen(str);
}

int32_t OSAL_Strcmp(const char *str1, const char *str2)
{
    return strcmp(str1, str2);
}

int32_t OSAL_Strncmp(const char *str1, const char *str2, osal_size_t n)
{
    return strncmp(str1, str2, (size_t)n);
}

char* OSAL_Strcpy(char *dest, const char *src)
{
    return strcpy(dest, src);
}

char* OSAL_Strncpy(char *dest, const char *src, osal_size_t n)
{
    return strncpy(dest, src, (size_t)n);
}

char* OSAL_Strcat(char *dest, const char *src)
{
    return strcat(dest, src);
}

char* OSAL_Strncat(char *dest, const char *src, osal_size_t n)
{
    return strncat(dest, src, (size_t)n);
}

char* OSAL_Strstr(const char *haystack, const char *needle)
{
    return strstr(haystack, needle);
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
    ret = vsnprintf(str, (size_t)size, format, args);
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
