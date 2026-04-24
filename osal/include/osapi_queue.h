/************************************************************************
 * 消息队列API
 ************************************************************************/

#ifndef OSAPI_QUEUE_H
#define OSAPI_QUEUE_H

#include "osa_types.h"

/**
 * @brief 创建消息队列
 *
 * @param[out] queue_id    返回的队列ID
 * @param[in]  queue_name  队列名称
 * @param[in]  queue_depth 队列深度(最大消息数)
 * @param[in]  data_size   每条消息的最大字节数
 * @param[in]  flags       保留,传0
 *
 * @return OS_SUCCESS 成功
 * @return OS_INVALID_POINTER queue_id为NULL
 * @return OS_ERR_NAME_TOO_LONG 名称过长
 * @return OS_ERR_NO_FREE_IDS 无可用队列ID
 * @return OS_ERR_NAME_TAKEN 名称已被使用
 * @return OS_QUEUE_INVALID_SIZE 队列大小无效
 * @return OS_ERROR 其他错误
 */
int32 OS_QueueCreate(osal_id_t *queue_id,
                     const char *queue_name,
                     uint32 queue_depth,
                     uint32 data_size,
                     uint32 flags);

/**
 * @brief 删除消息队列
 *
 * @param[in] queue_id 队列ID
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERR_INVALID_ID 无效的队列ID
 * @return OS_ERROR 删除失败
 */
int32 OS_QueueDelete(osal_id_t queue_id);

/**
 * @brief 发送消息到队列
 *
 * @param[in] queue_id 队列ID
 * @param[in] data     消息数据指针
 * @param[in] size     消息大小(字节)
 * @param[in] flags    保留,传0
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERR_INVALID_ID 无效的队列ID
 * @return OS_INVALID_POINTER data为NULL
 * @return OS_QUEUE_FULL 队列已满
 * @return OS_ERROR 其他错误
 */
int32 OS_QueuePut(osal_id_t queue_id, const void *data, uint32 size, uint32 flags);

/**
 * @brief 从队列接收消息
 *
 * @param[in]  queue_id 队列ID
 * @param[out] data     接收缓冲区
 * @param[in]  size     缓冲区大小
 * @param[out] size_copied 实际接收的字节数(可为NULL)
 * @param[in]  timeout  超时时间(毫秒), OS_PEND表示永久等待, OS_CHECK表示非阻塞
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERR_INVALID_ID 无效的队列ID
 * @return OS_INVALID_POINTER data为NULL
 * @return OS_QUEUE_EMPTY 队列为空(仅非阻塞模式)
 * @return OS_QUEUE_TIMEOUT 超时
 * @return OS_ERROR 其他错误
 */
int32 OS_QueueGet(osal_id_t queue_id, void *data, uint32 size,
                  uint32 *size_copied, int32 timeout);

/**
 * @brief 根据名称获取队列ID
 *
 * @param[out] queue_id   返回的队列ID
 * @param[in]  queue_name 队列名称
 *
 * @return OS_SUCCESS 成功
 * @return OS_INVALID_POINTER queue_id为NULL
 * @return OS_ERR_NAME_NOT_FOUND 未找到队列
 */
int32 OS_QueueGetIdByName(osal_id_t *queue_id, const char *queue_name);

#endif /* OSAPI_QUEUE_H */
