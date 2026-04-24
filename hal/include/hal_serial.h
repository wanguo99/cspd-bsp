/************************************************************************
 * HAL层 - 串口硬件抽象接口
 *
 * 串口属于具体硬件设备，因此放在HAL层而不是OSAL层
 ************************************************************************/

#ifndef HAL_SERIAL_H
#define HAL_SERIAL_H

#include "osal.h"

typedef void* hal_serial_handle_t;

typedef struct
{
    uint32 baud_rate;
    uint8  data_bits;      /* 5, 6, 7, 8 */
    uint8  stop_bits;      /* 1, 2 */
    uint8  parity;         /* NONE, ODD, EVEN */
    uint8  flow_control;   /* NONE, HW, SW */
} hal_serial_config_t;

#define HAL_SERIAL_PARITY_NONE  0
#define HAL_SERIAL_PARITY_ODD   1
#define HAL_SERIAL_PARITY_EVEN  2

#define HAL_SERIAL_FLOW_NONE    0
#define HAL_SERIAL_FLOW_HW      1
#define HAL_SERIAL_FLOW_SW      2

/**
 * @brief 打开串口设备
 *
 * @param[in]  device 设备路径 (e.g., "/dev/ttyS0")
 * @param[in]  config 串口配置
 * @param[out] handle 串口句柄
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 失败
 */
int32 HAL_Serial_Open(const char *device, const hal_serial_config_t *config,
                      hal_serial_handle_t *handle);

/**
 * @brief 关闭串口设备
 */
int32 HAL_Serial_Close(hal_serial_handle_t handle);

/**
 * @brief 向串口写入数据
 *
 * @param[in] timeout -1=阻塞, 0=非阻塞, >0=超时(ms)
 *
 * @return 实际写入的字节数，负值表示错误
 */
int32 HAL_Serial_Write(hal_serial_handle_t handle, const void *buffer,
                       uint32 size, int32 timeout);

/**
 * @brief 从串口读取数据
 *
 * @param[in] timeout -1=阻塞, 0=非阻塞, >0=超时(ms)
 *
 * @return 实际读取的字节数，负值表示错误
 */
int32 HAL_Serial_Read(hal_serial_handle_t handle, void *buffer,
                      uint32 size, int32 timeout);

/**
 * @brief 刷新串口缓冲区
 */
int32 HAL_Serial_Flush(hal_serial_handle_t handle);

/**
 * @brief 设置串口配置
 */
int32 HAL_Serial_SetConfig(hal_serial_handle_t handle,
                           const hal_serial_config_t *config);

#endif /* HAL_SERIAL_H */
