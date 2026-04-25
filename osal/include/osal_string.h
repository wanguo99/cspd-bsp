/************************************************************************
 * PMC-BSP - OSAL String Operations
 *
 * 功能：
 * - 封装标准库字符串和内存操作函数
 * - 提供跨平台统一接口，便于适配 RTOS
 *
 * 设计原则：
 * - 接口与标准库保持一致，降低学习成本
 * - 支持 Linux 和 RTOS 平台
 ************************************************************************/

#ifndef OSAL_STRING_H
#define OSAL_STRING_H

#include "osal_types.h"
#include <stddef.h>

/*
 * 内存操作
 */

/**
 * @brief 设置内存块
 * @param ptr 内存指针
 * @param value 设置的值
 * @param size 字节数
 * @return 内存指针
 */
void* OSAL_Memset(void *ptr, int value, size_t size);

/**
 * @brief 复制内存块
 * @param dest 目标内存
 * @param src 源内存
 * @param size 字节数
 * @return 目标内存指针
 */
void* OSAL_Memcpy(void *dest, const void *src, size_t size);

/**
 * @brief 移动内存块（支持重叠）
 * @param dest 目标内存
 * @param src 源内存
 * @param size 字节数
 * @return 目标内存指针
 */
void* OSAL_Memmove(void *dest, const void *src, size_t size);

/**
 * @brief 比较内存块
 * @param ptr1 内存块1
 * @param ptr2 内存块2
 * @param size 字节数
 * @return 0=相等, <0=ptr1<ptr2, >0=ptr1>ptr2
 */
int OSAL_Memcmp(const void *ptr1, const void *ptr2, size_t size);

/*
 * 字符串操作
 */

/**
 * @brief 获取字符串长度
 * @param str 字符串
 * @return 字符串长度（不包括'\0'）
 */
size_t OSAL_Strlen(const char *str);

/**
 * @brief 比较字符串
 * @param str1 字符串1
 * @param str2 字符串2
 * @return 0=相等, <0=str1<str2, >0=str1>str2
 */
int OSAL_Strcmp(const char *str1, const char *str2);

/**
 * @brief 比较字符串（限定长度）
 * @param str1 字符串1
 * @param str2 字符串2
 * @param n 最大比较长度
 * @return 0=相等, <0=str1<str2, >0=str1>str2
 */
int OSAL_Strncmp(const char *str1, const char *str2, size_t n);

/**
 * @brief 复制字符串
 * @param dest 目标缓冲区
 * @param src 源字符串
 * @return 目标字符串指针
 */
char* OSAL_Strcpy(char *dest, const char *src);

/**
 * @brief 复制字符串（限定长度）
 * @param dest 目标缓冲区
 * @param src 源字符串
 * @param n 最大复制长度
 * @return 目标字符串指针
 */
char* OSAL_Strncpy(char *dest, const char *src, size_t n);

/**
 * @brief 连接字符串
 * @param dest 目标字符串
 * @param src 源字符串
 * @return 目标字符串指针
 */
char* OSAL_Strcat(char *dest, const char *src);

/**
 * @brief 连接字符串（限定长度）
 * @param dest 目标字符串
 * @param src 源字符串
 * @param n 最大连接长度
 * @return 目标字符串指针
 */
char* OSAL_Strncat(char *dest, const char *src, size_t n);

/*
 * 字符串格式化
 */

/**
 * @brief 格式化字符串
 * @param str 目标缓冲区
 * @param format 格式字符串
 * @param ... 可变参数
 * @return 写入的字符数（不包括'\0'）
 */
int OSAL_Sprintf(char *str, const char *format, ...);

/**
 * @brief 格式化字符串（限定长度）
 * @param str 目标缓冲区
 * @param size 缓冲区大小
 * @param format 格式字符串
 * @param ... 可变参数
 * @return 写入的字符数（不包括'\0'）
 */
int OSAL_Snprintf(char *str, size_t size, const char *format, ...);

/**
 * @brief 解析格式化字符串
 * @param str 源字符串
 * @param format 格式字符串
 * @param ... 可变参数
 * @return 成功解析的参数个数
 */
int OSAL_Sscanf(const char *str, const char *format, ...);

#endif /* OSAL_STRING_H */
