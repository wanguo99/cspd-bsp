/************************************************************************
 * 消息队列API
 ************************************************************************/

#ifndef OSAPI_QUEUE_H
#define OSAPI_QUEUE_H

#include "osal_types.h"

/* 队列配置 */
#define OSAL_QUEUE_MAX_DEPTH        50      /* 默认队列深度 */
#define OSAL_QUEUE_MAX_MESSAGE_SIZE 512     /* 默认消息最大字节数 */
#define OSAL_QUEUE_MAX_COMMAND_SIZE 100     /* 命令缓存大小 */

/**
 * @brief 创建消息队列
 *
 * @param[out] queue_id    返回的队列ID
 * @param[in]  queue_name  队列名称
 * @param[in]  queue_depth 队列深度(最大消息数)
 * @param[in]  data_size   每条消息的最大字节数
 * @param[in]  flags       保留,传0
 *
 * @return OSAL_SUCCESS 成功
 * @return OSAL_ERR_INVALID_POINTER queue_id为NULL
 * @return OSAL_ERR_NAME_TOO_LONG 名称过长
 * @return OSAL_ERR_NO_FREE_IDS 无可用队列ID
 * @return OSAL_ERR_NAME_TAKEN 名称已被使用
 * @return OSAL_ERR_QUEUE_INVALID_SIZE 队列大小无效
 * @return OSAL_ERR_GENERIC 其他错误
 */
int32_t OSAL_QueueCreate(osal_id_t *queue_id,
                     const str_t *queue_name,
                     uint32_t queue_depth,
                     uint32_t data_size,
                     uint32_t flags);

/**
 * @brief 删除消息队列
 *
 * @param[in] queue_id 队列ID
 *
 * @return OSAL_SUCCESS 成功
 * @return OSAL_ERR_INVALID_ID 无效的队列ID
 * @return OSAL_ERR_GENERIC 删除失败
 */
int32_t OSAL_QueueDelete(osal_id_t queue_id);

/**
 * @brief 发送消息到队列
 *
 * @param[in] queue_id 队列ID
 * @param[in] data     消息数据指针
 * @param[in] size     消息大小(字节)
 * @param[in] flags    保留,传0
 *
 * @return OSAL_SUCCESS 成功
 * @return OSAL_ERR_INVALID_ID 无效的队列ID
 * @return OSAL_ERR_INVALID_POINTER data为NULL
 * @return OSAL_ERR_QUEUE_FULL 队列已满
 * @return OSAL_ERR_GENERIC 其他错误
 */
int32_t OSAL_QueuePut(osal_id_t queue_id, const void *data, uint32_t size, uint32_t flags);

/**
 * @brief 从队列接收消息
 *
 * @param[in]  queue_id 队列ID
 * @param[out] data     接收缓冲区
 * @param[in]  size     缓冲区大小
 * @param[out] size_copied 实际接收的字节数(可为NULL)
 * @param[in]  timeout  超时时间(毫秒), OS_PEND表示永久等待, OS_CHECK表示非阻塞
 *
 * @return OSAL_SUCCESS 成功
 * @return OSAL_ERR_INVALID_ID 无效的队列ID
 * @return OSAL_ERR_INVALID_POINTER data为NULL
 * @return OSAL_ERR_QUEUE_EMPTY 队列为空(仅非阻塞模式)
 * @return OSAL_ERR_QUEUE_TIMEOUT 超时
 * @return OSAL_ERR_GENERIC 其他错误
 */
int32_t OSAL_QueueGet(osal_id_t queue_id, void *data, uint32_t size,
                  uint32_t *size_copied, int32_t timeout);

/**
 * @brief 根据名称获取队列ID
 *
 * @param[out] queue_id   返回的队列ID
 * @param[in]  queue_name 队列名称
 *
 * @return OSAL_SUCCESS 成功
 * @return OSAL_ERR_INVALID_POINTER queue_id为NULL
 * @return OSAL_ERR_NAME_NOT_FOUND 未找到队列
 */
int32_t OSAL_QueueGetIdByName(osal_id_t *queue_id, const str_t *queue_name);

#endif /* OSAPI_QUEUE_H */
