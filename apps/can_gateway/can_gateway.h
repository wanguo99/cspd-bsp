/************************************************************************
 * CAN网关应用
 ************************************************************************/

#ifndef CAN_GATEWAY_H
#define CAN_GATEWAY_H

#include "osa_types.h"
#include "protocol/can_protocol.h"

/**
 * @brief 初始化CAN网关
 */
int32 CAN_Gateway_Init(void);

/**
 * @brief 获取CAN接收队列ID
 */
osal_id_t CAN_Gateway_GetRxQueue(void);

/**
 * @brief 获取CAN发送队列ID
 */
osal_id_t CAN_Gateway_GetTxQueue(void);

/**
 * @brief 发送CAN响应
 */
int32 CAN_Gateway_SendResponse(uint16 seq_num, can_status_t status, uint32 result);

/**
 * @brief 发送状态上报
 */
int32 CAN_Gateway_SendStatus(uint32 status_data);

/**
 * @brief 获取统计信息
 */
void CAN_Gateway_GetStats(uint32 *rx_count, uint32 *tx_count, uint32 *err_count);

#endif /* CAN_GATEWAY_H */
