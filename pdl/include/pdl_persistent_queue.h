/************************************************************************
 * 持久化命令队列API
 *
 * 功能：
 * - 将命令持久化到磁盘
 * - 系统启动时恢复未执行的命令
 * - 支持命令优先级和超时
 * - 二进制格式存储，无外部依赖
 ************************************************************************/

#ifndef PERSISTENT_QUEUE_H
#define PERSISTENT_QUEUE_H

#include "osa_types.h"
#include "config/can_protocol.h"

/*
 * 命令优先级
 */
typedef enum
{
    CMD_PRIORITY_LOW    = 0,
    CMD_PRIORITY_NORMAL = 1,
    CMD_PRIORITY_HIGH   = 2,
} cmd_priority_t;

/*
 * 命令状态
 */
typedef enum
{
    CMD_STATUS_PENDING   = 0,
    CMD_STATUS_EXECUTING = 1,
    CMD_STATUS_COMPLETED = 2,
    CMD_STATUS_FAILED    = 3,
} cmd_status_t;

/*
 * 持久化命令结构
 */
typedef struct
{
    can_cmd_type_t cmd_type;      /* 命令类型 */
    uint16 seq_num;               /* 序列号 */
    uint32 param;                 /* 参数 */
    cmd_priority_t priority;      /* 优先级 */
    uint32 timeout_ms;            /* 超时时间(ms) */
    cmd_status_t status;          /* 命令状态 */
    uint64 timestamp;             /* 时间戳(ms) */
} persistent_cmd_t;

/*
 * 持久化队列句柄
 */
typedef void* persistent_queue_handle_t;

/**
 * @brief 初始化持久化命令队列
 *
 * @param[in] queue_file 队列文件路径
 * @param[in] max_commands 最大命令数
 * @param[out] handle 队列句柄
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 失败
 */
int32 PersistentQueue_Init(const char *queue_file,
                           uint32 max_commands,
                           persistent_queue_handle_t *handle);

/**
 * @brief 关闭持久化命令队列
 *
 * @param[in] handle 队列句柄
 *
 * @return OS_SUCCESS 成功
 */
int32 PersistentQueue_Shutdown(persistent_queue_handle_t handle);

/**
 * @brief 添加命令到队列
 *
 * @param[in] handle 队列句柄
 * @param[in] cmd 命令指针
 *
 * @return OS_SUCCESS 成功
 * @return OS_QUEUE_FULL 队列已满
 * @return OS_ERROR 失败
 */
int32 PersistentQueue_Enqueue(persistent_queue_handle_t handle,
                              const persistent_cmd_t *cmd);

/**
 * @brief 从队列取出命令
 *
 * @param[in] handle 队列句柄
 * @param[out] cmd 命令指针
 * @param[in] timeout 超时时间(ms), OS_CHECK表示非阻塞
 *
 * @return OS_SUCCESS 成功
 * @return OS_QUEUE_EMPTY 队列为空
 * @return OS_QUEUE_TIMEOUT 超时
 * @return OS_ERROR 失败
 */
int32 PersistentQueue_Dequeue(persistent_queue_handle_t handle,
                              persistent_cmd_t *cmd,
                              int32 timeout);

/**
 * @brief 标记命令为已完成
 *
 * @param[in] handle 队列句柄
 * @param[in] seq_num 序列号
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 失败
 */
int32 PersistentQueue_MarkCompleted(persistent_queue_handle_t handle,
                                    uint16 seq_num);

/**
 * @brief 标记命令为失败
 *
 * @param[in] handle 队列句柄
 * @param[in] seq_num 序列号
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 失败
 */
int32 PersistentQueue_MarkFailed(persistent_queue_handle_t handle,
                                 uint16 seq_num);

/**
 * @brief 获取队列中的命令数
 *
 * @param[in] handle 队列句柄
 * @param[out] count 命令数
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 失败
 */
int32 PersistentQueue_GetCount(persistent_queue_handle_t handle,
                               uint32 *count);

/**
 * @brief 清空队列
 *
 * @param[in] handle 队列句柄
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 失败
 */
int32 PersistentQueue_Clear(persistent_queue_handle_t handle);

/**
 * @brief 将队列持久化到磁盘
 *
 * @param[in] handle 队列句柄
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 失败
 */
int32 PersistentQueue_Flush(persistent_queue_handle_t handle);

#endif /* PERSISTENT_QUEUE_H */
