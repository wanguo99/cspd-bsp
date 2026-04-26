/************************************************************************
 * XConfig BMC外设配置
 *
 * 功能：
 * - BMC外设配置类型定义
 * - 对应PDL层的pdl_bmc.h
 *
 * 说明：
 * - BMC作为软件层管理控制器，使用IPMI/Redfish等管理协议
 * - 主通道：IPMI over LAN（以太网）
 * - 备份通道：IPMI over Serial（串口）
 ************************************************************************/

#ifndef XCONFIG_BMC_H
#define XCONFIG_BMC_H

#include "xconfig_common.h"

/*===========================================================================
 * BMC协议类型枚举
 *===========================================================================*/

/**
 * @brief BMC协议类型
 */
typedef enum {
    XCONFIG_BMC_PROTOCOL_NONE = 0,
    XCONFIG_BMC_PROTOCOL_IPMI,      /* IPMI协议 */
    XCONFIG_BMC_PROTOCOL_REDFISH,   /* Redfish协议 */
    XCONFIG_BMC_PROTOCOL_MAX
} xconfig_bmc_protocol_t;

/*===========================================================================
 * BMC通道配置
 *===========================================================================*/

/**
 * @brief IPMI over LAN配置（以太网通道）
 */
typedef struct {
    const char *interface;        /* 网络接口名（如"eth0"） */
    const char *ip_addr;          /* BMC IP地址 */
    uint16      port;             /* IPMI端口（默认623） */
    const char *username;         /* 用户名 */
    const char *password;         /* 密码 */
} xconfig_bmc_ipmi_lan_cfg_t;

/**
 * @brief IPMI over Serial配置（串口通道）
 */
typedef struct {
    const str_t *device;          /* 串口设备（如"/dev/ttyS0"） */
    uint32       baudrate;        /* 波特率 */
    uint8        data_bits;       /* 数据位 */
    uint8        stop_bits;       /* 停止位 */
    int8         parity;          /* 校验位 */
} xconfig_bmc_ipmi_serial_cfg_t;

/**
 * @brief Redfish配置（基于HTTP/HTTPS）
 */
typedef struct {
    const char *base_url;         /* Redfish服务基础URL */
    const char *username;         /* 用户名 */
    const char *password;         /* 密码 */
    bool        use_https;        /* 是否使用HTTPS */
} xconfig_bmc_redfish_cfg_t;

/*===========================================================================
 * BMC外设配置
 *===========================================================================*/

/**
 * @brief BMC外设配置
 *
 * BMC作为软件层管理控制器，支持IPMI/Redfish等管理协议
 */
typedef struct {
    /* 外设基本信息 */
    const char *name;             /* BMC名称（如"payload_bmc"） */
    const char *description;      /* 描述信息 */
    bool        enabled;          /* 是否启用 */

    /* 主通道配置 */
    struct {
        xconfig_bmc_protocol_t protocol; /* 协议类型 */
        union {
            xconfig_bmc_ipmi_lan_cfg_t  ipmi_lan;
            xconfig_bmc_redfish_cfg_t   redfish;
        } cfg;
    } primary_channel;

    /* 备份通道配置（通常是IPMI over Serial） */
    struct {
        xconfig_bmc_protocol_t protocol; /* 协议类型 */
        xconfig_bmc_ipmi_serial_cfg_t cfg;
    } backup_channel;

    /* BMC特定配置 */
    uint32 cmd_timeout_ms;        /* 命令超时（ms） */
    uint32 retry_count;           /* 重试次数 */
    uint32 failover_threshold;    /* 故障切换阈值（连续失败次数） */

    /* GPIO控制（可选） */
    xconfig_gpio_config_t *power_gpio; /* 电源控制GPIO */
    xconfig_gpio_config_t *reset_gpio; /* 复位GPIO */
} xconfig_bmc_cfg_t;

#endif /* XCONFIG_BMC_H */
