/************************************************************************
 * XConfig板级配置
 *
 * 功能：
 * - 板级配置类型定义（顶层）
 * - 聚合所有外设配置（MCU、BMC、卫星平台等）
 *
 * 说明：
 * - 这是顶层配置结构，以外设为单位描述整个板子的硬件配置
 * - 包含板级信息、外设列表、APP配置列表
 ************************************************************************/

#ifndef XCONFIG_BOARD_H
#define XCONFIG_BOARD_H

#include "xconfig_mcu.h"
#include "xconfig_bmc.h"
#include "xconfig_satellite.h"
#include "xconfig_sensor.h"
#include "xconfig_storage.h"
#include "xconfig_app.h"

/*===========================================================================
 * 板级配置（顶层）
 *===========================================================================*/

/**
 * @brief 板级硬件配置
 *
 * 这是顶层配置结构，以外设为单位描述整个板子的硬件配置
 */
typedef struct {
    /* 板级信息 */
    const char *platform;         /* 平台名称（如"ti/am625"） */
    const char *product;          /* 产品名称（如"h200_payload"） */
    const char *version;          /* 版本号（如"v1.0"） */
    const char *description;      /* 描述信息 */

    /* 外设配置列表（以外设为单位） */
    xconfig_mcu_cfg_t       **mcus;         /* MCU外设列表 */
    uint32_t                    mcu_count;

    xconfig_bmc_cfg_t       **bmcs;         /* BMC外设列表 */
    uint32_t                    bmc_count;

    xconfig_satellite_cfg_t **satellites;   /* 卫星平台接口列表 */
    uint32_t                    satellite_count;

    xconfig_sensor_cfg_t    **sensors;      /* 传感器列表 */
    uint32_t                    sensor_count;

    xconfig_storage_cfg_t   **storages;     /* 存储设备列表 */
    uint32_t                    storage_count;

    xconfig_power_domain_t  **power_domains; /* 电源域列表 */
    uint32_t                    power_domain_count;

    /* APP配置列表 */
    xconfig_app_config_t    **apps;         /* APP配置列表 */
    uint32_t                    app_count;

    /* 扩展配置（预留） */
    void *private_data;           /* 私有数据指针 */
} xconfig_board_config_t;

#endif /* XCONFIG_BOARD_H */
