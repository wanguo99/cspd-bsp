/************************************************************************
 * MCU外设驱动接口
 *
 * 功能：
 * - 与MCU通信（支持CAN/串口等多种接口）
 * - MCU状态监控和控制
 * - MCU固件升级
 *
 * 设计理念：
 * - 对外只暴露业务接口（版本查询、状态读取、命令执行等）
 * - 内部封装通信细节（CAN/串口协议、帧封装、CRC校验等）
 * - 支持多种通信方式，由配置决定使用哪种
 ************************************************************************/

#ifndef PDL_MCU_H
#define PDL_MCU_H

#include "osal_types.h"

/*
 * MCU服务句柄
 */
typedef void* mcu_handle_t;

/*
 * MCU通信接口类型
 */
typedef enum
{
    MCU_INTERFACE_CAN = 0,     /* CAN总线 */
    MCU_INTERFACE_SERIAL = 1,  /* 串口 */
    MCU_INTERFACE_I2C = 2,     /* I2C（预留） */
    MCU_INTERFACE_SPI = 3      /* SPI（预留） */
} mcu_interface_t;

/*
 * MCU配置
 */
typedef struct
{
    str_t name[64];                   /* MCU名称 */
    mcu_interface_t interface;        /* 通信接口 */

    /* CAN配置 */
    struct {
        const char *device;           /* CAN设备（如can0） */
        uint32 bitrate;               /* 波特率 */
        uint32 tx_id;                 /* 发送CAN ID */
        uint32 rx_id;                 /* 接收CAN ID */
    } can;

    /* 串口配置 */
    struct {
        const char *device;           /* 串口设备（如/dev/ttyS1） */
        uint32 baudrate;              /* 波特率 */
        uint8 data_bits;              /* 数据位（5-8） */
        uint8 stop_bits;              /* 停止位（1-2） */
        str_t parity;                 /* 校验位（'N'/'E'/'O'） */
    } serial;

    /* 通用配置 */
    uint32 cmd_timeout_ms;            /* 命令超时（ms） */
    uint32 retry_count;               /* 重试次数 */
    bool enable_crc;                  /* 启用CRC校验 */
} mcu_config_t;

/*
 * MCU版本信息
 */
typedef struct
{
    uint8 major;
    uint8 minor;
    uint8 patch;
    uint8 build;
    str_t version_string[32];
} mcu_version_t;

/*
 * MCU状态信息
 */
typedef struct
{
    bool online;                      /* 在线状态 */
    uint32 uptime_sec;                /* 运行时间 */
    uint8 error_code;                 /* 错误码 */
    float temperature;                /* 温度 */
    uint16 voltage_mv;                /* 电压（mV） */
} mcu_status_t;

/**
 * @brief 初始化MCU驱动
 *
 * @param[in] config 配置参数
 * @param[out] handle MCU句柄
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 失败
 */
int32 PDL_MCU_Init(const mcu_config_t *config, mcu_handle_t *handle);

/**
 * @brief 反初始化MCU驱动
 *
 * @param[in] handle MCU句柄
 *
 * @return OS_SUCCESS 成功
 */
int32 PDL_MCU_Deinit(mcu_handle_t handle);

/**
 * @brief 获取MCU版本
 *
 * @param[in] handle MCU句柄
 * @param[out] version 版本信息
 *
 * @return OS_SUCCESS 成功
 */
int32 PDL_MCU_GetVersion(mcu_handle_t handle, mcu_version_t *version);

/**
 * @brief 获取MCU状态
 *
 * @param[in] handle MCU句柄
 * @param[out] status 状态信息
 *
 * @return OS_SUCCESS 成功
 */
int32 PDL_MCU_GetStatus(mcu_handle_t handle, mcu_status_t *status);

/**
 * @brief MCU复位
 *
 * @param[in] handle MCU句柄
 *
 * @return OS_SUCCESS 成功
 */
int32 PDL_MCU_Reset(mcu_handle_t handle);

/**
 * @brief 读取MCU寄存器
 *
 * @param[in] handle MCU句柄
 * @param[in] reg_addr 寄存器地址
 * @param[out] value 读取值
 *
 * @return OS_SUCCESS 成功
 */
int32 PDL_MCU_ReadRegister(mcu_handle_t handle, uint8 reg_addr, uint8 *value);

/**
 * @brief 写入MCU寄存器
 *
 * @param[in] handle MCU句柄
 * @param[in] reg_addr 寄存器地址
 * @param[in] value 写入值
 *
 * @return OS_SUCCESS 成功
 */
int32 PDL_MCU_WriteRegister(mcu_handle_t handle, uint8 reg_addr, uint8 value);

/**
 * @brief 发送自定义命令到MCU
 *
 * @param[in] handle MCU句柄
 * @param[in] cmd_code 命令码
 * @param[in] data 命令数据
 * @param[in] data_len 数据长度
 * @param[out] response 响应缓冲区
 * @param[in] resp_size 缓冲区大小
 * @param[out] actual_size 实际响应长度
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR_TIMEOUT 超时
 * @return OS_ERROR 失败
 */
int32 PDL_MCU_SendCommand(mcu_handle_t handle,
                          uint8 cmd_code,
                          const uint8 *data,
                          uint32 data_len,
                          uint8 *response,
                          uint32 resp_size,
                          uint32 *actual_size);

/**
 * @brief MCU固件升级
 *
 * @param[in] handle MCU句柄
 * @param[in] firmware_path 固件文件路径
 * @param[in] progress_callback 进度回调（可选）
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 失败
 */
int32 PDL_MCU_FirmwareUpdate(mcu_handle_t handle,
                             const char *firmware_path,
                             void (*progress_callback)(uint32 percent));

#endif /* PDL_MCU_H */
