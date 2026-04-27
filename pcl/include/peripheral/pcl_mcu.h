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

#ifndef PCL_MCU_H
#define PCL_MCU_H

#include "pcl_common.h"
#include "pcl_hardware_interface.h"

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
    pcl_hw_interface_type_t interface_type;
    union {
        pcl_can_cfg_t       can;
        pcl_uart_cfg_t      uart;
        pcl_i2c_cfg_t       i2c;
        pcl_spi_cfg_t       spi;
    } interface_cfg;

    /* MCU特定配置 */
    uint32_t cmd_timeout_ms;        /* 命令超时（ms） */
    uint32_t retry_count;           /* 重试次数 */
    bool   enable_crc;            /* 启用CRC校验 */

    /* GPIO控制（可选） */
    pcl_gpio_config_t *reset_gpio; /* 复位GPIO */
    pcl_gpio_config_t *irq_gpio;   /* 中断GPIO */
} pcl_mcu_cfg_t;

#endif /* PCL_MCU_H */
