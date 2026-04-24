/************************************************************************
 * HAL层 - CAN驱动接口
 *
 * 提供统一的CAN总线访问接口
 ************************************************************************/

#ifndef HAL_CAN_H
#define HAL_CAN_H

#include "osa_types.h"
#include "protocol/can_protocol.h"

typedef void* hal_can_handle_t;

typedef struct
{
    const char *interface;  /* e.g., "can0", "vcan0" */
    uint32      baudrate;
    uint32      rx_timeout;
    uint32      tx_timeout;
} hal_can_config_t;

/**
 * @brief 初始化CAN驱动
 *
 * @param[in] config CAN配置
 * @param[out] handle 返回的CAN句柄
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 失败
 */
int32 HAL_CAN_Init(const hal_can_config_t *config, hal_can_handle_t *handle);

/**
 * @brief 关闭CAN驱动
 *
 * @param[in] handle CAN句柄
 *
 * @return OS_SUCCESS 成功
 */
int32 HAL_CAN_Deinit(hal_can_handle_t handle);

/**
 * @brief 发送CAN帧
 *
 * @param[in] handle CAN句柄
 * @param[in] frame CAN帧
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR_TIMEOUT 超时
 * @return OS_ERROR 其他错误
 */
int32 HAL_CAN_Send(hal_can_handle_t handle, const can_frame_t *frame);

/**
 * @brief 接收CAN帧
 *
 * @param[in] handle CAN句柄
 * @param[out] frame 接收到的CAN帧
 * @param[in] timeout 超时时间(ms), OS_PEND表示永久等待
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR_TIMEOUT 超时
 * @return OS_ERROR 其他错误
 */
int32 HAL_CAN_Recv(hal_can_handle_t handle, can_frame_t *frame, int32 timeout);

/**
 * @brief 设置CAN过滤器
 *
 * @param[in] handle CAN句柄
 * @param[in] filter_id 过滤ID
 * @param[in] filter_mask 过滤掩码
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 失败
 */
int32 HAL_CAN_SetFilter(hal_can_handle_t handle, uint32 filter_id, uint32 filter_mask);

/**
 * @brief 获取CAN统计信息
 *
 * @param[in] handle CAN句柄
 * @param[out] tx_count 发送计数
 * @param[out] rx_count 接收计数
 * @param[out] err_count 错误计数
 *
 * @return OS_SUCCESS 成功
 */
int32 HAL_CAN_GetStats(hal_can_handle_t handle,
                       uint32 *tx_count,
                       uint32 *rx_count,
                       uint32 *err_count);

#endif /* HAL_CAN_H */
