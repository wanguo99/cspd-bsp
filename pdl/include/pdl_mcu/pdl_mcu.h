/************************************************************************
 * MCU外设驱动
 *
 * 功能：
 * - 支持通过I2C/SPI/UART与MCU通信
 * - MCU固件升级
 * - MCU状态监控
 * - MCU命令执行
 *
 * 典型应用场景：
 * - 电源管理MCU
 * - 传感器采集MCU
 * - 风扇控制MCU
 * - LED控制MCU
 ************************************************************************/

#ifndef PERIPHERAL_MCU_H
#define PERIPHERAL_MCU_H

#include "osal_types.h"
#include "pdl_device.h"

/*
 * MCU通信接口类型
 */
typedef enum
{
    MCU_INTERFACE_I2C = 0,
    MCU_INTERFACE_SPI = 1,
    MCU_INTERFACE_UART = 2
} mcu_interface_t;

/*
 * MCU配置
 */
typedef struct
{
    char name[64];                    /* MCU名称 */
    mcu_interface_t interface;        /* 通信接口 */

    /* I2C配置 */
    struct {
        const char *device;           /* I2C设备（如/dev/i2c-0） */
        uint8 slave_addr;             /* 从机地址 */
        uint32 speed_hz;              /* 速度（Hz） */
    } i2c;

    /* SPI配置 */
    struct {
        const char *device;           /* SPI设备（如/dev/spidev0.0） */
        uint32 speed_hz;              /* 速度（Hz） */
        uint8 mode;                   /* SPI模式（0-3） */
        uint8 bits_per_word;          /* 每字位数（通常8） */
    } spi;

    /* UART配置 */
    struct {
        const char *device;           /* UART设备（如/dev/ttyS1） */
        uint32 baudrate;              /* 波特率 */
        uint8 data_bits;              /* 数据位（5-8） */
        uint8 stop_bits;              /* 停止位（1-2） */
        char parity;                  /* 校验位（'N'/'E'/'O'） */
    } uart;

    /* 通用配置 */
    uint32 cmd_timeout_ms;            /* 命令超时（ms） */
    uint32 retry_count;               /* 重试次数 */
    bool enable_crc;                  /* 启用CRC校验 */
    uint32 heartbeat_interval_ms;     /* 心跳间隔（ms，0=禁用） */
} mcu_config_t;

/*
 * MCU命令类型
 */
typedef enum
{
    MCU_CMD_GET_VERSION = 0x01,       /* 获取版本 */
    MCU_CMD_GET_STATUS = 0x02,        /* 获取状态 */
    MCU_CMD_RESET = 0x03,             /* 复位 */
    MCU_CMD_POWER_ON = 0x10,          /* 上电 */
    MCU_CMD_POWER_OFF = 0x11,         /* 下电 */
    MCU_CMD_READ_SENSOR = 0x20,       /* 读取传感器 */
    MCU_CMD_WRITE_GPIO = 0x30,        /* 写GPIO */
    MCU_CMD_READ_GPIO = 0x31,         /* 读GPIO */
    MCU_CMD_FIRMWARE_UPDATE = 0xF0,   /* 固件升级 */
    MCU_CMD_CUSTOM = 0xFF             /* 自定义命令 */
} mcu_cmd_type_t;

/*
 * MCU命令帧格式
 */
typedef struct
{
    uint8 cmd;                        /* 命令字 */
    uint8 len;                        /* 数据长度 */
    uint8 data[256];                  /* 数据 */
    uint16 crc;                       /* CRC校验（可选） */
} mcu_cmd_frame_t;

/*
 * MCU响应帧格式
 */
typedef struct
{
    uint8 status;                     /* 状态码（0=成功） */
    uint8 len;                        /* 数据长度 */
    uint8 data[256];                  /* 数据 */
    uint16 crc;                       /* CRC校验（可选） */
} mcu_resp_frame_t;

/*
 * MCU版本信息
 */
typedef struct
{
    uint8 major;
    uint8 minor;
    uint8 patch;
    uint8 build;
    char version_string[32];
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
 * @brief 初始化MCU外设驱动
 *
 * @param[in] config 配置参数
 * @param[out] handle 外设句柄
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 失败
 */
int32 MCU_Init(const mcu_config_t *config, peripheral_handle_t *handle);

/**
 * @brief 反初始化MCU外设驱动
 *
 * @param[in] handle 外设句柄
 *
 * @return OS_SUCCESS 成功
 */
int32 MCU_Deinit(peripheral_handle_t handle);

/**
 * @brief 获取MCU版本
 *
 * @param[in] handle 外设句柄
 * @param[out] version 版本信息
 *
 * @return OS_SUCCESS 成功
 */
int32 MCU_GetVersion(peripheral_handle_t handle, mcu_version_t *version);

/**
 * @brief 获取MCU状态
 *
 * @param[in] handle 外设句柄
 * @param[out] status 状态信息
 *
 * @return OS_SUCCESS 成功
 */
int32 MCU_GetStatus(peripheral_handle_t handle, mcu_status_t *status);

/**
 * @brief MCU复位
 *
 * @param[in] handle 外设句柄
 *
 * @return OS_SUCCESS 成功
 */
int32 MCU_Reset(peripheral_handle_t handle);

/**
 * @brief 发送命令到MCU
 *
 * @param[in] handle 外设句柄
 * @param[in] cmd 命令帧
 * @param[out] resp 响应帧
 * @param[in] timeout_ms 超时时间（ms）
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR_TIMEOUT 超时
 * @return OS_ERROR 失败
 */
int32 MCU_SendCommand(peripheral_handle_t handle,
                     const mcu_cmd_frame_t *cmd,
                     mcu_resp_frame_t *resp,
                     uint32 timeout_ms);

/**
 * @brief 读取MCU寄存器
 *
 * @param[in] handle 外设句柄
 * @param[in] reg_addr 寄存器地址
 * @param[out] value 读取值
 *
 * @return OS_SUCCESS 成功
 */
int32 MCU_ReadRegister(peripheral_handle_t handle, uint8 reg_addr, uint8 *value);

/**
 * @brief 写入MCU寄存器
 *
 * @param[in] handle 外设句柄
 * @param[in] reg_addr 寄存器地址
 * @param[in] value 写入值
 *
 * @return OS_SUCCESS 成功
 */
int32 MCU_WriteRegister(peripheral_handle_t handle, uint8 reg_addr, uint8 value);

/**
 * @brief MCU固件升级
 *
 * @param[in] handle 外设句柄
 * @param[in] firmware_path 固件文件路径
 * @param[in] progress_callback 进度回调（可选）
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 失败
 */
int32 MCU_FirmwareUpdate(peripheral_handle_t handle,
                        const char *firmware_path,
                        void (*progress_callback)(uint32 percent));

/**
 * @brief 注册MCU外设驱动到外设管理框架
 *
 * @return OS_SUCCESS 成功
 */
int32 MCU_RegisterDriver(void);

#endif /* PERIPHERAL_MCU_H */
