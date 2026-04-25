/**
 * @file xconfig_storage.h
 * @brief 存储设备配置定义
 * @note 对应PDL层的存储设备（预留扩展）
 */

#ifndef XCONFIG_STORAGE_H
#define XCONFIG_STORAGE_H

#include "xconfig_common.h"

/*===========================================================================
 * 存储设备配置
 *===========================================================================*/

/**
 * @brief 存储设备类型
 */
typedef enum {
    STORAGE_TYPE_EMMC = 0,
    STORAGE_TYPE_SD,
    STORAGE_TYPE_NAND,
    STORAGE_TYPE_NOR,
    STORAGE_TYPE_NVME,
    STORAGE_TYPE_SATA
} storage_type_t;

/**
 * @brief 存储设备配置
 */
typedef struct {
    /* 外设基本信息 */
    const char *name;             /* 存储设备名称（如"emmc_storage"） */
    const char *description;      /* 描述信息 */
    storage_type_t type;          /* 存储类型 */
    bool        enabled;          /* 是否启用 */

    /* 设备路径 */
    const char *device_path;      /* 设备路径（如"/dev/mmcblk0"） */

    /* 存储特定配置 */
    uint64 capacity_mb;           /* 容量（MB） */
    uint32 block_size;            /* 块大小（字节） */

    /* GPIO控制（可选） */
    xconfig_gpio_config_t *power_gpio; /* 电源控制GPIO */
} xconfig_storage_cfg_t;

#endif /* XCONFIG_STORAGE_H */
