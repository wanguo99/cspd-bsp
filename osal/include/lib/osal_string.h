/************************************************************************
 * OSAL - string系统调用封装
 *
 * 功能：
 * - 封装字符串和内存操作函数
 * - 1:1映射系统调用，不引入业务逻辑
 * - 使用固定大小类型，避免平台相关类型
 *
 * 设计原则：
 * - 提供标准C库字符串函数的封装
 * - 返回值与系统调用保持一致
 * - 便于RTOS移植
 ************************************************************************/

#ifndef OSAL_STRING_H
#define OSAL_STRING_H

#include "osal_types.h"

/*
 * 内存操作
 */
void* OSAL_Memset(void *ptr, int32 value, osal_size_t size);
void* OSAL_Memcpy(void *dest, const void *src, osal_size_t size);
void* OSAL_Memmove(void *dest, const void *src, osal_size_t size);
int32 OSAL_Memcmp(const void *ptr1, const void *ptr2, osal_size_t size);

/*
 * 字符串操作
 */
osal_size_t OSAL_Strlen(const char *str);
int32 OSAL_Strcmp(const char *str1, const char *str2);
int32 OSAL_Strncmp(const char *str1, const char *str2, osal_size_t n);
char* OSAL_Strcpy(char *dest, const char *src);
char* OSAL_Strncpy(char *dest, const char *src, osal_size_t n);
char* OSAL_Strcat(char *dest, const char *src);
char* OSAL_Strncat(char *dest, const char *src, osal_size_t n);
char* OSAL_Strstr(const char *haystack, const char *needle);

/*
 * 字符串格式化
 */
int32 OSAL_Sprintf(char *str, const char *format, ...);
int32 OSAL_Snprintf(char *str, osal_size_t size, const char *format, ...);
int32 OSAL_Sscanf(const char *str, const char *format, ...);

#endif /* OSAL_STRING_H */
