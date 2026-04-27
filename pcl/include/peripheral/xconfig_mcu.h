/************************************************************************
 * XConfig MCU外设配置
 *
 * 功能：
 * - MCU外设配置类型定义
 * - 对应PDL层的pdl_mcu.h
 *
 * 说明：
 * - MCU作为硬件微控制器，使用物理层硬件接口通信
 * - 支持CAN/UART/I2C/SPI等多种硬件接口
 ************************************************************************/

#ifndef XCONFIG_MCU_H
#define XCONFIG_MCU_H

#include "xconfig_common.h"
#include "xconfig_hardware_interface.h"

/*===========================================================================
 * MCU外设配置
 *===========================================================================*/

/**
 * @brief MCU外设配置
 *
 * MCU作为硬件微控制器，使用物理层硬件接口通信
 */
typedef struct {
    /* 外设基本信息 */
    const char *name;             /* MCU名称（如"stm32_mcu"） */
    const char *description;      /* 描述信息 */
    bool        enabled;          /* 是否启用 */

    /* 硬件通信接口配置（使用联合体） */
    xconfig_hw_interface_type_t interface_type;
    union {
        xconfig_can_cfg_t       can;
        xconfig_uart_cfg_t      uart;
        xconfig_i2c_cfg_t       i2c;
        xconfig_spi_cfg_t       spi;
    } interface_cfg;

    /* MCU特定配置 */
    uint32_t cmd_timeout_ms;        /* 命令超时（ms） */
    uint32_t retry_count;           /* 重试次数 */
    bool   enable_crc;            /* 启用CRC校验 */

    /* GPIO控制（可选） */
    xconfig_gpio_config_t *reset_gpio; /* 复位GPIO */
    xconfig_gpio_config_t *irq_gpio;   /* 中断GPIO */
} xconfig_mcu_cfg_t;

#endif /* XCONFIG_MCU_H */
