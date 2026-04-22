/************************************************************************
 * HAL层 - 网络通信接口
 *
 * 提供TCP/UDP网络通信的硬件抽象接口
 ************************************************************************/

#ifndef HAL_NETWORK_H
#define HAL_NETWORK_H

#include "common_types.h"

/*
 * 网络协议类型
 */
typedef enum
{
    HAL_NET_PROTO_TCP = 0,  /* TCP协议 */
    HAL_NET_PROTO_UDP = 1,  /* UDP协议 */
} hal_net_proto_t;

/*
 * 网络句柄
 */
typedef void* hal_network_handle_t;

/*
 * 网络配置
 */
typedef struct
{
    hal_net_proto_t protocol;  /* 协议类型 */
    const char     *ip_addr;   /* IP地址 */
    uint16          port;      /* 端口号 */
    uint32          timeout_ms;/* 超时时间 */
} hal_network_config_t;

/**
 * @brief 打开网络连接
 *
 * @param[in]  config 网络配置
 * @param[out] handle 网络句柄
 *
 * @return OS_SUCCESS 成功
 * @return OS_INVALID_POINTER 参数为空
 * @return OS_ERROR 连接失败
 */
int32 HAL_Network_Open(const hal_network_config_t *config, hal_network_handle_t *handle);

/**
 * @brief 关闭网络连接
 *
 * @param[in] handle 网络句柄
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERR_INVALID_ID 无效句柄
 */
int32 HAL_Network_Close(hal_network_handle_t handle);

/**
 * @brief 发送数据
 *
 * @param[in] handle  网络句柄
 * @param[in] buffer  数据缓冲区
 * @param[in] size    数据大小
 * @param[in] timeout 超时时间(毫秒)
 *
 * @return 实际发送的字节数(>=0)
 * @return OS_ERROR 发送失败
 * @return OS_ERROR_TIMEOUT 超时
 */
int32 HAL_Network_Send(hal_network_handle_t handle, const void *buffer, uint32 size, int32 timeout);

/**
 * @brief 接收数据
 *
 * @param[in]  handle  网络句柄
 * @param[out] buffer  接收缓冲区
 * @param[in]  size    缓冲区大小
 * @param[in]  timeout 超时时间(毫秒)
 *
 * @return 实际接收的字节数(>=0)
 * @return OS_ERROR 接收失败
 * @return OS_ERROR_TIMEOUT 超时
 */
int32 HAL_Network_Recv(hal_network_handle_t handle, void *buffer, uint32 size, int32 timeout);

/**
 * @brief 检查连接状态
 *
 * @param[in] handle 网络句柄
 *
 * @return true 已连接
 * @return false 未连接
 */
bool HAL_Network_IsConnected(hal_network_handle_t handle);

#endif /* HAL_NETWORK_H */
