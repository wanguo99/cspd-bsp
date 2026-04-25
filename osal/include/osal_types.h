/************************************************************************
 * 通用类型定义
 *
 * 与NASA cFS兼容的基础类型定义
 ************************************************************************/

#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/*
 * 基础整数类型 (与cFS兼容)
 */
typedef int8_t   int8;
typedef int16_t  int16;
typedef int32_t  int32;
typedef int64_t  int64;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

/*
 * OSAL对象ID类型
 */
typedef uint32 osal_id_t;

#define OS_OBJECT_ID_UNDEFINED  ((osal_id_t)0)

/*
 * 平台相关的大小类型
 * - 32位平台：使用32位类型
 * - 64位平台：使用64位类型
 * - 保证应用层代码在不同平台间兼容
 */
#if defined(__LP64__) || defined(_WIN64) || defined(__x86_64__) || defined(__aarch64__)
    /* 64位平台 */
    typedef uint64 osal_size_t;
    typedef int64  osal_ssize_t;
#else
    /* 32位平台 */
    typedef uint32 osal_size_t;
    typedef int32  osal_ssize_t;
#endif

/*
 * 返回值类型 (参考Linux errno风格)
 * 成功返回0，错误返回正数错误码
 */
#define OS_SUCCESS                  0    /* 成功 */
#define OS_ERROR                    1    /* 通用错误 */
#define OS_INVALID_POINTER          2    /* 无效指针 (EFAULT) */
#define OS_ERROR_ADDRESS_MISALIGNED 3    /* 地址未对齐 */
#define OS_ERROR_TIMEOUT            4    /* 超时 (ETIMEDOUT) */
#define OS_INVALID_INT_NUM          5    /* 无效中断号 */
#define OS_SEM_FAILURE              6    /* 信号量失败 */
#define OS_SEM_TIMEOUT              7    /* 信号量超时 */
#define OS_QUEUE_EMPTY              8    /* 队列为空 */
#define OS_QUEUE_FULL               9    /* 队列已满 */
#define OS_QUEUE_TIMEOUT            10   /* 队列超时 */
#define OS_QUEUE_INVALID_SIZE       11   /* 队列大小无效 (EINVAL) */
#define OS_QUEUE_ID_ERROR           12   /* 队列ID错误 */
#define OS_ERR_NAME_TOO_LONG        13   /* 名称过长 (ENAMETOOLONG) */
#define OS_ERR_NO_FREE_IDS          14   /* 无可用ID (ENOMEM) */
#define OS_ERR_NAME_TAKEN           15   /* 名称已被占用 (EEXIST) */
#define OS_ERR_INVALID_ID           16   /* 无效ID (EINVAL) */
#define OS_ERR_NAME_NOT_FOUND       17   /* 名称未找到 (ENOENT) */
#define OS_ERR_SEM_NOT_FULL         18   /* 信号量未满 */
#define OS_ERR_INVALID_PRIORITY     19   /* 无效优先级 */
#define OS_INVALID_SEM_VALUE        20   /* 无效信号量值 */
#define OS_ERR_FILE                 21   /* 文件错误 (EIO) */
#define OS_ERR_NOT_IMPLEMENTED      22   /* 未实现 (ENOSYS) */
#define OS_TIMER_ERR_INVALID_ARGS   23   /* 定时器参数无效 */
#define OS_TIMER_ERR_TIMER_ID       24   /* 定时器ID错误 */
#define OS_TIMER_ERR_UNAVAILABLE    25   /* 定时器不可用 (EAGAIN) */
#define OS_TIMER_ERR_INTERNAL       26   /* 定时器内部错误 */
#define OS_ERR_INVALID_SIZE         27   /* 无效大小 (EINVAL) */
#define OS_ERR_NO_MEMORY            28   /* 内存不足 (ENOMEM) */
#define OS_ERR_BUSY                 29   /* 资源忙 (EBUSY) */
#define OS_ERR_PERMISSION           30   /* 权限不足 (EPERM) */
#define OS_ERR_NOT_SUPPORTED        31   /* 不支持的操作 (ENOTSUP) */
#define OS_ERR_ALREADY_EXISTS       32   /* 已存在 (EEXIST) */
#define OS_ERR_WOULD_BLOCK          33   /* 操作会阻塞 (EWOULDBLOCK) */
#define OS_ERR_INTERRUPTED          34   /* 操作被中断 (EINTR) */
#define OS_ERR_BAD_ADDRESS          35   /* 错误的地址 (EFAULT) */
#define OS_ERR_INVALID_STATE        36   /* 无效状态 */
#define OS_ERR_RESOURCE_LIMIT       37   /* 资源限制 (EMFILE) */

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
