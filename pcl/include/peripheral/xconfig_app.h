/************************************************************************
 * XConfig APP配置
 *
 * 功能：
 * - APP配置类型定义
 * - APP外设映射（定义APP使用哪些外设）
 *
 * 说明：
 * - APP配置定义某个APP在特定产品上使用哪些外设
 * - 通过外设类型+编号映射到具体硬件
 ************************************************************************/

#ifndef XCONFIG_APP_H
#define XCONFIG_APP_H

#include "xconfig_common.h"

/*===========================================================================
 * APP配置
 *===========================================================================*/

/**
 * @brief 外设类型枚举
 */
typedef enum {
    XCONFIG_DEV_SATELLITE = 0,     /* 卫星平台接口 */
    XCONFIG_DEV_MCU,               /* MCU外设 */
    XCONFIG_DEV_BMC,               /* BMC外设 */
    XCONFIG_DEV_SENSOR,            /* 传感器 */
    XCONFIG_DEV_STORAGE,           /* 存储设备 */
    XCONFIG_DEV_MAX
} xconfig_device_type_t;

/**
 * @brief APP外设映射
 *
 * 定义APP使用哪个外设（类型+编号）
 */
typedef struct {
    const char *function;         /* 功能名称（如"satellite_comm"） */
    xconfig_device_type_t device_type; /* 外设类型 */
    uint32_t device_id;             /* 外设编号（第几个） */
    bool required;                /* 是否必需 */
} xconfig_app_device_mapping_t;

/**
 * @brief APP配置
 *
 * 定义某个APP在特定产品上使用哪些外设
 */
typedef struct {
    const char *app_name;         /* APP名称（如"can_gateway"） */
    const char *description;      /* 描述 */

    /* 外设映射列表 */
    xconfig_app_device_mapping_t *device_mappings;
    uint32_t mapping_count;

    /* APP功能参数 */
    struct {
        uint32_t heartbeat_interval_ms;
        uint32_t cmd_timeout_ms;
        uint32_t retry_count;
        uint32_t queue_depth;
        uint32_t failover_threshold;
    } params;
} xconfig_app_config_t;

#endif /* XCONFIG_APP_H */
