/************************************************************************
 * 通用类型定义
 *
 * 与NASA cFS兼容的基础类型定义
 * 支持多平台：Linux/RTOS/VxWorks/FreeRTOS等
 ************************************************************************/

#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

/*===========================================================================
 * 平台兼容性处理：C99标准类型
 *===========================================================================*/

/* 优先使用C99标准头文件 */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
    /* C99或更高版本 */
    #include <stdint.h>
    #include <stdbool.h>
    #include <stddef.h>
#elif defined(__GNUC__) || defined(__clang__)
    /* GCC/Clang通常提供stdint.h */
    #include <stdint.h>
    #include <stdbool.h>
    #include <stddef.h>
#else
    /* 平台不支持stdint.h，手动定义固定宽度类型 */

    /* 有符号整数类型 */
    typedef signed char        int8_t;
    typedef signed short       int16_t;
    typedef signed int         int32_t;
    typedef signed long long   int64_t;

    /* 无符号整数类型 */
    typedef unsigned char      uint8_t;
    typedef unsigned short     uint16_t;
    typedef unsigned int       uint32_t;
    typedef unsigned long long uint64_t;

    /* 布尔类型 */
    #ifndef __cplusplus
        #ifndef bool
            typedef unsigned char bool;
            #define true  1
            #define false 0
        #endif
    #endif

    /* size_t和NULL */
    #ifndef NULL
        #define NULL ((void *)0)
    #endif

    #ifndef _SIZE_T_DEFINED
        #define _SIZE_T_DEFINED
        typedef unsigned long size_t;
    #endif
#endif

/*===========================================================================
 * 字符串类型（语义化别名）
 *===========================================================================*/

/*
 * str_t: 字符串类型（底层是char，与标准C库兼容）
 *
 * 设计原则：
 * - 用于文本数据（设备名、日志消息、版本字符串等）
 * - 与标准C库（strcpy, strlen, fopen等）完全兼容
 * - 区别于uint8_t（用于二进制字节数据）
 *
 * 使用示例：
 *   str_t device_name[64];           // 设备名称
 *   str_t log_message[256];          // 日志消息
 *   const str_t *interface;          // 字符串指针
 *   str_t parity;                    // 校验位字符 ('N', 'E', 'O')
 */
typedef char str_t;

/*===========================================================================
 * OSAL对象ID类型
 *===========================================================================*/

typedef uint32_t osal_id_t;

#define OS_OBJECT_ID_UNDEFINED  ((osal_id_t)0)

/*===========================================================================
 * 平台相关的大小类型
 *===========================================================================*/

/*
 * osal_size_t / osal_ssize_t: 平台相关的大小类型
 * - 32位平台：使用32位类型
 * - 64位平台：使用64位类型
 * - 保证应用层代码在不同平台间兼容
 *
 * 支持架构：x86_64, ARM32, ARM64, RISC-V 32/64
 */
#if defined(__LP64__) || defined(_WIN64) || \
    defined(__x86_64__) || defined(__amd64__) || \
    defined(__aarch64__) || \
    (defined(__riscv) && (__riscv_xlen == 64))
    /* 64位平台：x86_64, ARM64, RISC-V 64 */
    typedef uint64_t osal_size_t;
    typedef int64_t  osal_ssize_t;
#else
    /* 32位平台：ARM32, RISC-V 32, x86 */
    typedef uint32_t osal_size_t;
    typedef int32_t  osal_ssize_t;
#endif

/*===========================================================================
 * 字节序转换宏（支持大小端平台）
 *===========================================================================*/

/*
 * 字节序检测
 * - __BYTE_ORDER__ 是 GCC/Clang 内置宏
 * - 支持 x86_64(小端), ARM32/ARM64(小端), RISC-V(小端/大端可配置)
 */
#if defined(__BYTE_ORDER__)
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        #define OSAL_LITTLE_ENDIAN 1
        #define OSAL_BIG_ENDIAN    0
    #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        #define OSAL_LITTLE_ENDIAN 0
        #define OSAL_BIG_ENDIAN    1
    #else
        #error "Unknown byte order"
    #endif
#else
    /* 如果编译器不提供字节序宏，假设小端（x86/ARM/RISC-V 默认） */
    #warning "Byte order not detected, assuming little-endian"
    #define OSAL_LITTLE_ENDIAN 1
    #define OSAL_BIG_ENDIAN    0
#endif

/*
 * 字节序转换宏
 * - OSAL_HTONS/HTONL: 主机序 -> 网络序（大端）
 * - OSAL_NTOHS/NTOHL: 网络序（大端）-> 主机序
 */
#if OSAL_LITTLE_ENDIAN
    /* 小端平台需要字节交换 */
    #if defined(__GNUC__) || defined(__clang__)
        /* GCC/Clang 内置函数（编译器优化为单条指令） */
        #define OSAL_HTONS(x)  __builtin_bswap16(x)
        #define OSAL_HTONL(x)  __builtin_bswap32(x)
        #define OSAL_HTONLL(x) __builtin_bswap64(x)
        #define OSAL_NTOHS(x)  __builtin_bswap16(x)
        #define OSAL_NTOHL(x)  __builtin_bswap32(x)
        #define OSAL_NTOHLL(x) __builtin_bswap64(x)
    #else
        /* 手动实现字节交换 */
        #define OSAL_HTONS(x)  ((uint16_t)(((x) >> 8) | ((x) << 8)))
        #define OSAL_HTONL(x)  ((uint32_t)(((x) >> 24) | (((x) & 0x00FF0000) >> 8) | \
                                           (((x) & 0x0000FF00) << 8) | ((x) << 24)))
        #define OSAL_HTONLL(x) ((uint64_t)(((uint64_t)OSAL_HTONL((x) & 0xFFFFFFFF) << 32) | \
                                           OSAL_HTONL((x) >> 32)))
        #define OSAL_NTOHS(x)  OSAL_HTONS(x)
        #define OSAL_NTOHL(x)  OSAL_HTONL(x)
        #define OSAL_NTOHLL(x) OSAL_HTONLL(x)
    #endif
#else
    /* 大端平台无需转换 */
    #define OSAL_HTONS(x)  (x)
    #define OSAL_HTONL(x)  (x)
    #define OSAL_HTONLL(x) (x)
    #define OSAL_NTOHS(x)  (x)
    #define OSAL_NTOHL(x)  (x)
    #define OSAL_NTOHLL(x) (x)
#endif

/*===========================================================================
 * 返回值类型（参考Linux errno风格）
 *===========================================================================*/

/*
 * OSAL状态码定义
 * 成功返回0，错误返回正数错误码
 * 统一使用 OSAL_ERR_ 前缀
 */
#define OSAL_SUCCESS                    0    /* 成功 */
#define OSAL_ERR_GENERIC                1    /* 通用错误 */
#define OSAL_ERR_INVALID_POINTER        2    /* 无效指针 (EFAULT) */
#define OSAL_ERR_ADDRESS_MISALIGNED     3    /* 地址未对齐 */
#define OSAL_ERR_TIMEOUT                4    /* 超时 (ETIMEDOUT) */
#define OSAL_ERR_INVALID_INT_NUM        5    /* 无效中断号 */
#define OSAL_ERR_SEM_FAILURE            6    /* 信号量失败 */
#define OSAL_ERR_SEM_TIMEOUT            7    /* 信号量超时 */
#define OSAL_ERR_QUEUE_EMPTY            8    /* 队列为空 */
#define OSAL_ERR_QUEUE_FULL             9    /* 队列已满 */
#define OSAL_ERR_QUEUE_TIMEOUT          10   /* 队列超时 */
#define OSAL_ERR_QUEUE_INVALID_SIZE     11   /* 队列大小无效 (EINVAL) */
#define OSAL_ERR_QUEUE_ID               12   /* 队列ID错误 */
#define OSAL_ERR_NAME_TOO_LONG          13   /* 名称过长 (ENAMETOOLONG) */
#define OSAL_ERR_NO_FREE_IDS            14   /* 无可用ID (ENOMEM) */
#define OSAL_ERR_NAME_TAKEN             15   /* 名称已被占用 (EEXIST) */
#define OSAL_ERR_INVALID_ID             16   /* 无效ID (EINVAL) */
#define OSAL_ERR_NAME_NOT_FOUND         17   /* 名称未找到 (ENOENT) */
#define OSAL_ERR_SEM_NOT_FULL           18   /* 信号量未满 */
#define OSAL_ERR_INVALID_PRIORITY       19   /* 无效优先级 */
#define OSAL_ERR_INVALID_SEM_VALUE      20   /* 无效信号量值 */
#define OSAL_ERR_FILE                   21   /* 文件错误 (EIO) */
#define OSAL_ERR_NOT_IMPLEMENTED        22   /* 未实现 (ENOSYS) */
#define OSAL_ERR_TIMER_INVALID_ARGS     23   /* 定时器参数无效 */
#define OSAL_ERR_TIMER_ID               24   /* 定时器ID错误 */
#define OSAL_ERR_TIMER_UNAVAILABLE      25   /* 定时器不可用 (EAGAIN) */
#define OSAL_ERR_TIMER_INTERNAL         26   /* 定时器内部错误 */
#define OSAL_ERR_INVALID_SIZE           27   /* 无效大小 (EINVAL) */
#define OSAL_ERR_NO_MEMORY              28   /* 内存不足 (ENOMEM) */
#define OSAL_ERR_BUSY                   29   /* 资源忙 (EBUSY) */
#define OSAL_ERR_PERMISSION             30   /* 权限不足 (EPERM) */
#define OSAL_ERR_NOT_SUPPORTED          31   /* 不支持的操作 (ENOTSUP) */
#define OSAL_ERR_ALREADY_EXISTS         32   /* 已存在 (EEXIST) */
#define OSAL_ERR_WOULD_BLOCK            33   /* 操作会阻塞 (EWOULDBLOCK) */
#define OSAL_ERR_INTERRUPTED            34   /* 操作被中断 (EINTR) */
#define OSAL_ERR_BAD_ADDRESS            35   /* 错误的地址 (EFAULT) */
#define OSAL_ERR_INVALID_STATE          36   /* 无效状态 */
#define OSAL_ERR_RESOURCE_LIMIT         37   /* 资源限制 (EMFILE) */


/*
 * 配置常量
 */
#define OS_MAX_TASKS              64
#define OS_MAX_QUEUES             64
#define OS_MAX_MUTEXES            64
#define OS_MAX_API_NAME           20

/*
 * 超时常量
 */
#define OS_PEND                   0
#define OS_CHECK                  (-1)

/*
 * 任务优先级
 */
#define OS_TASK_PRIORITY_MIN      1
#define OS_TASK_PRIORITY_MAX      255

#endif /* COMMON_TYPES_H */
