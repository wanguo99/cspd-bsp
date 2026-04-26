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
void* OSAL_Memset(void *ptr, int32_t value, osal_size_t size);
void* OSAL_Memcpy(void *dest, const void *src, osal_size_t size);
void* OSAL_Memmove(void *dest, const void *src, osal_size_t size);
int32_t OSAL_Memcmp(const void *ptr1, const void *ptr2, osal_size_t size);

/*
 * 字符串操作
 */
osal_size_t OSAL_Strlen(const str_t *str);
int32_t OSAL_Strcmp(const str_t *str1, const str_t *str2);
int32_t OSAL_Strncmp(const str_t *str1, const str_t *str2, osal_size_t n);
str_t* OSAL_Strcpy(str_t *dest, const str_t *src);
str_t* OSAL_Strncpy(str_t *dest, const str_t *src, osal_size_t n);
str_t* OSAL_Strcat(str_t *dest, const str_t *src);
str_t* OSAL_Strncat(str_t *dest, const str_t *src, osal_size_t n);
str_t* OSAL_Strstr(const str_t *haystack, const str_t *needle);

/*
 * 字符串格式化
 */
int32_t OSAL_Sprintf(str_t *str, const str_t *format, ...);
int32_t OSAL_Snprintf(str_t *str, osal_size_t size, const str_t *format, ...);
int32_t OSAL_Sscanf(const str_t *str, const str_t *format, ...);

#endif /* OSAL_STRING_H */
