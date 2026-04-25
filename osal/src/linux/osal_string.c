/************************************************************************
 * OSAL Linux实现 - 字符串和内存操作
 ************************************************************************/

#include "osal.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/*
 * 内存操作
 */

void* OSAL_Memset(void *ptr, int value, size_t size)
{
    return memset(ptr, value, size);
}

void* OSAL_Memcpy(void *dest, const void *src, size_t size)
{
    return memcpy(dest, src, size);
}

void* OSAL_Memmove(void *dest, const void *src, size_t size)
{
    return memmove(dest, src, size);
}

int OSAL_Memcmp(const void *ptr1, const void *ptr2, size_t size)
{
    return memcmp(ptr1, ptr2, size);
}

/*
 * 字符串操作
 */

size_t OSAL_Strlen(const char *str)
{
    return strlen(str);
}

int OSAL_Strcmp(const char *str1, const char *str2)
{
    return strcmp(str1, str2);
}

int OSAL_Strncmp(const char *str1, const char *str2, size_t n)
{
    return strncmp(str1, str2, n);
}

char* OSAL_Strcpy(char *dest, const char *src)
{
    return strcpy(dest, src);
}

char* OSAL_Strncpy(char *dest, const char *src, size_t n)
{
    return strncpy(dest, src, n);
}

char* OSAL_Strcat(char *dest, const char *src)
{
    return strcat(dest, src);
}

char* OSAL_Strncat(char *dest, const char *src, size_t n)
{
    return strncat(dest, src, n);
}

/*
 * 字符串格式化
 */

int OSAL_Sprintf(char *str, const char *format, ...)
{
    va_list args;
    int ret;

    va_start(args, format);
    ret = vsprintf(str, format, args);
    va_end(args);

    return ret;
}

int OSAL_Snprintf(char *str, size_t size, const char *format, ...)
{
    va_list args;
    int ret;

    va_start(args, format);
    ret = vsnprintf(str, size, format, args);
    va_end(args);

    return ret;
}

int OSAL_Sscanf(const char *str, const char *format, ...)
{
    va_list args;
    int ret;

    va_start(args, format);
    ret = vsscanf(str, format, args);
    va_end(args);

    return ret;
}
