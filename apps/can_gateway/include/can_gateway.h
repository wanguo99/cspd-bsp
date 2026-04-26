/************************************************************************
 * CAN网关应用
 ************************************************************************/

#ifndef CAN_GATEWAY_H
#define CAN_GATEWAY_H

#include "osal_types.h"
#include "config/can_protocol.h"

/**
 * @brief 初始化CAN网关
 */
int32_t CAN_Gateway_Init(void);

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
int32_t CAN_Gateway_SendResponse(uint16_t seq_num, can_status_t status, uint32_t result);

/**
 * @brief 发送状态上报
 */
int32_t CAN_Gateway_SendStatus(uint32_t status_data);

/**
 * @brief 获取统计信息
 */
void CAN_Gateway_GetStats(uint32_t *rx_count, uint32_t *tx_count, uint32_t *err_count);

#endif /* CAN_GATEWAY_H */
