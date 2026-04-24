/************************************************************************
 * 协议转换应用 - 头文件
 ************************************************************************/

#ifndef PROTOCOL_CONVERTER_H
#define PROTOCOL_CONVERTER_H

#include "osa_types.h"
#include "payload_service.h"

/**
 * @brief 初始化协议转换模块
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 失败
 */
int32 Protocol_Converter_Init(void);

/**
 * @brief 获取统计信息
 *
 * @param[out] cmd_count 命令总数
 * @param[out] success_count 成功数
 * @param[out] fail_count 失败数
 * @param[out] timeout_count 超时数
 */
void Protocol_Converter_GetStats(uint32 *cmd_count, uint32 *success_count,
                                  uint32 *fail_count, uint32 *timeout_count);

/**
 * @brief 切换载荷通信通道
 *
 * @param[in] channel 通道类型
 *
 * @return OS_SUCCESS 成功
 */
int32 Protocol_Converter_SwitchChannel(payload_channel_t channel);

/**
 * @brief 获取当前通道
 *
 * @return payload_channel_t 当前通道
 */
payload_channel_t Protocol_Converter_GetChannel(void);

#endif /* PROTOCOL_CONVERTER_H */
