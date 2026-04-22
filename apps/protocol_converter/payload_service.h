/************************************************************************
 * 载荷服务接口
 *
 * 提供以太网和UART双通道通信能力，支持自动切换
 ************************************************************************/

#ifndef PAYLOAD_SERVICE_H
#define PAYLOAD_SERVICE_H

#include "common_types.h"

/*
 * ============================================================================
 * 类型定义
 * ============================================================================
 */

/* 通道类型 */
typedef enum
{
    PAYLOAD_CHANNEL_ETHERNET = 0,  /* 以太网通道 */
    PAYLOAD_CHANNEL_UART     = 1,  /* UART通道 */
} payload_channel_t;

/* 以太网配置 */
typedef struct
{
    const char *ip_addr;      /* IP地址 */
    uint16      port;         /* 端口号 */
    uint32      timeout_ms;   /* 超时时间(ms) */
} payload_ethernet_config_t;

/* UART配置 */
typedef struct
{
    const char *device;       /* 设备路径 */
    uint32      baudrate;     /* 波特率 */
    uint32      timeout_ms;   /* 超时时间(ms) */
} payload_uart_config_t;

/* 载荷服务配置 */
typedef struct
{
    payload_ethernet_config_t ethernet;    /* 以太网配置 */
    payload_uart_config_t     uart;        /* UART配置 */
    bool                      auto_switch; /* 自动切换使能 */
    uint32                    retry_count; /* 重试次数 */
} payload_service_config_t;

/* 载荷服务句柄 */
typedef void* payload_service_handle_t;

/*
 * ============================================================================
 * 函数声明
 * ============================================================================
 */

/**
 * @brief 初始化载荷服务
 *
 * @param[in] config 配置参数
 * @param[out] handle 服务句柄
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 失败
 */
int32 PayloadService_Init(const payload_service_config_t *config,
                          payload_service_handle_t *handle);

/**
 * @brief 反初始化载荷服务
 *
 * @param[in] handle 服务句柄
 *
 * @return OS_SUCCESS 成功
 */
int32 PayloadService_Deinit(payload_service_handle_t handle);

/**
 * @brief 发送数据到载荷
 *
 * @param[in] handle 服务句柄
 * @param[in] data 数据缓冲区
 * @param[in] len 数据长度
 *
 * @return 发送的字节数 (>= 0)
 * @return OS_ERROR 失败
 */
int32 PayloadService_Send(payload_service_handle_t handle,
                          const void *data,
                          uint32 len);

/**
 * @brief 从载荷接收数据
 *
 * @param[in] handle 服务句柄
 * @param[out] buf 接收缓冲区
 * @param[in] buf_size 缓冲区大小
 * @param[in] timeout_ms 超时时间(ms)
 *
 * @return 接收的字节数 (>= 0)
 * @return OS_ERROR_TIMEOUT 超时
 * @return OS_ERROR 失败
 */
int32 PayloadService_Recv(payload_service_handle_t handle,
                          void *buf,
                          uint32 buf_size,
                          uint32 timeout_ms);

/**
 * @brief 检查载荷连接状态
 *
 * @param[in] handle 服务句柄
 *
 * @return true 已连接
 * @return false 未连接
 */
bool PayloadService_IsConnected(payload_service_handle_t handle);

/**
 * @brief 切换通信通道
 *
 * @param[in] handle 服务句柄
 * @param[in] channel 目标通道
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 失败
 */
int32 PayloadService_SwitchChannel(payload_service_handle_t handle,
                                   payload_channel_t channel);

/**
 * @brief 获取当前通道
 *
 * @param[in] handle 服务句柄
 *
 * @return payload_channel_t 当前通道
 */
payload_channel_t PayloadService_GetChannel(payload_service_handle_t handle);

#endif /* PAYLOAD_SERVICE_H */
