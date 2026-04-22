/************************************************************************
 * HAL层 - 串口通信接口
 *
 * 提供独立的串口驱动接口
 ************************************************************************/

#ifndef HAL_SERIAL_H
#define HAL_SERIAL_H

#include "common_types.h"

/*
 * 串口句柄类型
 */
typedef void* hal_serial_handle_t;

/*
 * 校验位类型
 */
#define HAL_SERIAL_PARITY_NONE  0  /* 无校验 */
#define HAL_SERIAL_PARITY_ODD   1  /* 奇校验 */
#define HAL_SERIAL_PARITY_EVEN  2  /* 偶校验 */

/*
 * 串口配置
 */
typedef struct
{
    uint32 baudrate;     /* 波特率 */
    uint8  databits;     /* 数据位(7或8) */
    uint8  stopbits;     /* 停止位(1或2) */
    uint8  parity;       /* 校验位(0=无, 1=奇, 2=偶) */
    uint32 timeout_ms;   /* 超时时间(毫秒) */
} hal_serial_config_t;

/**
 * @brief 打开串口
 *
 * @param[in]  device    设备名称(例如"/dev/ttyS0"或"COM1")
 * @param[in]  config    串口配置
 * @param[out] handle    串口句柄
 *
 * @return OS_SUCCESS 成功
 * @return OS_INVALID_POINTER device或handle为NULL
 * @return OS_ERROR 打开失败
 */
int32 HAL_Serial_Open(const char *device, const hal_serial_config_t *config, hal_serial_handle_t *handle);

/**
 * @brief 关闭串口
 *
 * @param[in] handle 串口句柄
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERR_INVALID_ID 无效的句柄
 */
int32 HAL_Serial_Close(hal_serial_handle_t handle);

/**
 * @brief 发送数据
 *
 * @param[in] handle  串口句柄
 * @param[in] buffer  数据缓冲区
 * @param[in] size    数据大小
 * @param[in] timeout 超时时间(毫秒)
 *
 * @return 实际发送的字节数(>=0)
 * @return OS_ERROR 发送失败
 */
int32 HAL_Serial_Write(hal_serial_handle_t handle, const void *buffer, uint32 size, int32 timeout);

/**
 * @brief 接收数据
 *
 * @param[in]  handle  串口句柄
 * @param[out] buffer  接收缓冲区
 * @param[in]  size    缓冲区大小
 * @param[in]  timeout 超时时间(毫秒)
 *
 * @return 实际接收的字节数(>=0)
 * @return OS_ERROR 接收失败
 * @return OS_ERROR_TIMEOUT 超时
 */
int32 HAL_Serial_Read(hal_serial_handle_t handle, void *buffer, uint32 size, int32 timeout);

/**
 * @brief 清空接收缓冲区
 *
 * @param[in] handle 串口句柄
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERR_INVALID_ID 无效的句柄
 */
int32 HAL_Serial_Flush(hal_serial_handle_t handle);

#endif /* HAL_SERIAL_H */
