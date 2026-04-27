/************************************************************************
 * XConfig卫星平台配置
 *
 * 功能：
 * - 卫星平台配置类型定义
 * - 对应PDL层的pdl_satellite.h
 *
 * 说明：
 * - 卫星平台作为硬件外设，使用物理层硬件接口通信
 * - 支持SpaceWire/1553B/CAN等航天专用接口
 ************************************************************************/

#ifndef PCL_SATELLITE_H
#define PCL_SATELLITE_H

#include "pcl_common.h"
#include "pcl_hardware_interface.h"

/*===========================================================================
 * 卫星平台配置
 *===========================================================================*/

/**
 * @brief 卫星平台配置
 *
 * 卫星平台作为硬件外设，使用物理层硬件接口通信
 */
typedef struct {
    /* 外设基本信息 */
    const char *name;             /* 卫星平台名称（如"satellite_bus"） */
    const char *description;      /* 描述信息 */
    bool        enabled;          /* 是否启用 */

    /* 硬件通信接口配置（使用联合体） */
    pcl_hw_interface_type_t interface_type;
    union {
        pcl_spacewire_cfg_t spacewire;
        pcl_1553b_cfg_t     mil1553b;
        pcl_can_cfg_t       can;
        pcl_uart_cfg_t      uart;
    } interface_cfg;

    /* 卫星平台特定配置 */
    uint32_t cmd_timeout_ms;        /* 命令超时（ms） */
    uint32_t retry_count;           /* 重试次数 */
    bool   enable_telemetry;      /* 启用遥测 */

    /* GPIO控制（可选） */
    pcl_gpio_config_t *power_gpio; /* 电源控制GPIO */
    pcl_gpio_config_t *reset_gpio; /* 复位GPIO */
} pcl_satellite_cfg_t;

#endif /* PCL_SATELLITE_H */
