/************************************************************************
 * 文件名  ：hal_serial.h
 * 功能描述：串口硬件抽象层接口定义
 *
 * 注意：串口属于具体硬件设备，因此放在HAL层而不是OSAL层
 ************************************************************************/

#ifndef HAL_SERIAL_H
#define HAL_SERIAL_H

#include "osal.h"

/*
 * 串口句柄类型
 */
typedef void* hal_serial_handle_t;

/*
 * 串口配置参数
 */
typedef struct
{
    uint32 baud_rate;      /* 波特率 */
    uint8  data_bits;      /* 数据位 (5, 6, 7, 8) */
    uint8  stop_bits;      /* 停止位 (1, 2) */
    uint8  parity;         /* 校验位 (0=无, 1=奇, 2=偶) */
    uint8  flow_control;   /* 流控 (0=无, 1=硬件, 2=软件) */
} hal_serial_config_t;

/* 校验位定义 */
#define HAL_SERIAL_PARITY_NONE  0
#define HAL_SERIAL_PARITY_ODD   1
#define HAL_SERIAL_PARITY_EVEN  2

/* 流控定义 */
#define HAL_SERIAL_FLOW_NONE    0
#define HAL_SERIAL_FLOW_HW      1
#define HAL_SERIAL_FLOW_SW      2

/************************************************************************
 * 函数名  ：HAL_Serial_Open
 * 功能描述：打开串口设备
 * 输入参数：device - 设备路径 (如 "/dev/ttyS0")
 *          config - 串口配置参数
 * 输出参数：handle - 串口句柄
 * 返回值  ：OS_SUCCESS 成功，其他值失败
 ************************************************************************/
int32 HAL_Serial_Open(const char *device, const hal_serial_config_t *config,
                      hal_serial_handle_t *handle);

/************************************************************************
 * 函数名  ：HAL_Serial_Close
 * 功能描述：关闭串口设备
 * 输入参数：handle - 串口句柄
 * 返回值  ：OS_SUCCESS 成功，其他值失败
 ************************************************************************/
int32 HAL_Serial_Close(hal_serial_handle_t handle);

/************************************************************************
 * 函数名  ：HAL_Serial_Write
 * 功能描述：向串口写入数据
 * 输入参数：handle  - 串口句柄
 *          buffer  - 数据缓冲区
 *          size    - 要写入的字节数
 *          timeout - 超时时间(毫秒)，-1表示阻塞，0表示非阻塞
 * 返回值  ：实际写入的字节数，负值表示错误
 ************************************************************************/
int32 HAL_Serial_Write(hal_serial_handle_t handle, const void *buffer,
                       uint32 size, int32 timeout);

/************************************************************************
 * 函数名  ：HAL_Serial_Read
 * 功能描述：从串口读取数据
 * 输入参数：handle  - 串口句柄
 *          buffer  - 数据缓冲区
 *          size    - 缓冲区大小
 *          timeout - 超时时间(毫秒)，-1表示阻塞，0表示非阻塞
 * 返回值  ：实际读取的字节数，负值表示错误
 ************************************************************************/
int32 HAL_Serial_Read(hal_serial_handle_t handle, void *buffer,
                      uint32 size, int32 timeout);

/************************************************************************
 * 函数名  ：HAL_Serial_Flush
 * 功能描述：刷新串口缓冲区
 * 输入参数：handle - 串口句柄
 * 返回值  ：OS_SUCCESS 成功，其他值失败
 ************************************************************************/
int32 HAL_Serial_Flush(hal_serial_handle_t handle);

/************************************************************************
 * 函数名  ：HAL_Serial_SetConfig
 * 功能描述：设置串口配置
 * 输入参数：handle - 串口句柄
 *          config - 新的配置参数
 * 返回值  ：OS_SUCCESS 成功，其他值失败
 ************************************************************************/
int32 HAL_Serial_SetConfig(hal_serial_handle_t handle,
                           const hal_serial_config_t *config);

#endif /* HAL_SERIAL_H */
