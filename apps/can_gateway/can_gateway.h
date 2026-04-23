/************************************************************************
 * CAN网关应用 - 头文件
 ************************************************************************/

#ifndef CAN_GATEWAY_H
#define CAN_GATEWAY_H

#include "osa_types.h"
#include "can_protocol.h"

/**
 * @brief 初始化CAN网关
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 失败
 */
int32 CAN_Gateway_Init(void);

/**
 * @brief 获取CAN接收队列ID
 *
 * @return 队列ID
 */
osal_id_t CAN_Gateway_GetRxQueue(void);

/**
 * @brief 获取CAN发送队列ID
 *
 * @return 队列ID
 */
osal_id_t CAN_Gateway_GetTxQueue(void);

/**
 * @brief 发送CAN响应
 *
 * @param[in] seq_num 序列号
 * @param[in] status 状态码
 * @param[in] result 结果数据
 *
 * @return OS_SUCCESS 成功
 */
int32 CAN_Gateway_SendResponse(uint16 seq_num, can_status_t status, uint32 result);

/**
 * @brief 发送状态上报
 *
 * @param[in] status_data 状态数据
 *
 * @return OS_SUCCESS 成功
 */
int32 CAN_Gateway_SendStatus(uint32 status_data);

/**
 * @brief 获取统计信息
 *
 * @param[out] rx_count 接收计数
 * @param[out] tx_count 发送计数
 * @param[out] err_count 错误计数
 */
void CAN_Gateway_GetStats(uint32 *rx_count, uint32 *tx_count, uint32 *err_count);

#endif /* CAN_GATEWAY_H */
