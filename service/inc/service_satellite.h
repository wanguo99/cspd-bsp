/************************************************************************
 * 卫星平台接口服务
 *
 * 功能：
 * - 封装CAN总线通信协议
 * - 处理平台命令请求和响应
 * - 管理心跳和状态上报
 * - 提供统一的平台交互接口
 ************************************************************************/

#ifndef SERVICE_SATELLITE_H
#define SERVICE_SATELLITE_H

#include "osa_types.h"
#include "protocol/can_protocol.h"

/*
 * 卫星平台服务句柄
 */
typedef void* satellite_service_handle_t;

/*
 * 卫星平台服务配置
 */
typedef struct
{
    const char *can_device;        /* CAN设备名 */
    uint32 can_bitrate;            /* CAN波特率 */
    uint32 heartbeat_interval_ms;  /* 心跳间隔(ms) */
    uint32 cmd_timeout_ms;         /* 命令超时(ms) */
} satellite_service_config_t;

/*
 * 命令回调函数类型
 */
typedef void (*satellite_cmd_callback_t)(can_cmd_type_t cmd_type, uint32 param, void *user_data);

/**
 * @brief 初始化卫星平台服务
 *
 * @param[in] config 配置参数
 * @param[out] handle 服务句柄
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 失败
 */
int32 SatelliteService_Init(const satellite_service_config_t *config,
                            satellite_service_handle_t *handle);

/**
 * @brief 反初始化卫星平台服务
 *
 * @param[in] handle 服务句柄
 *
 * @return OS_SUCCESS 成功
 */
int32 SatelliteService_Deinit(satellite_service_handle_t handle);

/**
 * @brief 注册命令回调函数
 *
 * @param[in] handle 服务句柄
 * @param[in] callback 回调函数
 * @param[in] user_data 用户数据
 *
 * @return OS_SUCCESS 成功
 */
int32 SatelliteService_RegisterCallback(satellite_service_handle_t handle,
                                        satellite_cmd_callback_t callback,
                                        void *user_data);

/**
 * @brief 发送响应到卫星平台
 *
 * @param[in] handle 服务句柄
 * @param[in] seq_num 序列号
 * @param[in] status 状态码
 * @param[in] result 结果数据
 *
 * @return OS_SUCCESS 成功
 */
int32 SatelliteService_SendResponse(satellite_service_handle_t handle,
                                    uint32 seq_num,
                                    can_status_t status,
                                    uint32 result);

/**
 * @brief 发送心跳到卫星平台
 *
 * @param[in] handle 服务句柄
 * @param[in] status 当前状态
 *
 * @return OS_SUCCESS 成功
 */
int32 SatelliteService_SendHeartbeat(satellite_service_handle_t handle,
                                     can_status_t status);

/**
 * @brief 获取服务统计信息
 *
 * @param[in] handle 服务句柄
 * @param[out] rx_count 接收计数
 * @param[out] tx_count 发送计数
 * @param[out] error_count 错误计数
 *
 * @return OS_SUCCESS 成功
 */
int32 SatelliteService_GetStats(satellite_service_handle_t handle,
                                uint32 *rx_count,
                                uint32 *tx_count,
                                uint32 *error_count);

#endif /* SERVICE_SATELLITE_H */
