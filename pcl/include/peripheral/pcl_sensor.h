/**
 * @file pcl_sensor.h
 * @brief 传感器外设配置定义
 * @note 对应PDL层的传感器外设（预留扩展）
 */

#ifndef PCL_SENSOR_H
#define PCL_SENSOR_H

#include "pcl_common.h"
#include "pcl_hardware_interface.h"

/*===========================================================================
 * 传感器外设配置
 *===========================================================================*/

/**
 * @brief 传感器类型
 */
typedef enum {
    SENSOR_TYPE_TEMPERATURE = 0,
    SENSOR_TYPE_PRESSURE,
    SENSOR_TYPE_HUMIDITY,
    SENSOR_TYPE_ACCELEROMETER,
    SENSOR_TYPE_GYROSCOPE,
    SENSOR_TYPE_MAGNETOMETER,
    SENSOR_TYPE_GPS,
    SENSOR_TYPE_CAMERA,
    SENSOR_TYPE_CUSTOM
} sensor_type_t;

/**
 * @brief 传感器外设配置
 */
typedef struct {
    /* 外设基本信息 */
    const char *name;             /* 传感器名称（如"board_temp"） */
    const char *description;      /* 描述信息 */
    sensor_type_t type;           /* 传感器类型 */
    bool        enabled;          /* 是否启用 */

    /* 硬件通信接口配置（使用联合体） */
    pcl_hw_interface_type_t interface_type;
    union {
        pcl_i2c_cfg_t  i2c;
        pcl_spi_cfg_t  spi;
        pcl_uart_cfg_t uart;
    } interface_cfg;

    /* 传感器特定配置 */
    uint32_t sample_rate;           /* 采样率（Hz） */
    uint32_t resolution;            /* 分辨率（位） */

    /* GPIO控制（可选） */
    pcl_gpio_config_t *power_gpio; /* 电源控制GPIO */
    pcl_gpio_config_t *irq_gpio;   /* 中断GPIO */
} pcl_sensor_cfg_t;

#endif /* PCL_SENSOR_H */
