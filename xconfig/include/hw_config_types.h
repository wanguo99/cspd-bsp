/************************************************************************
 * 硬件配置类型定义（以外设为单位）
 *
 * 设计理念：
 * - 以外设为配置单位（MCU、BMC、传感器等）
 * - 每个外设包含其通信接口配置
 * - 参考Linux设备树的设计思想
 ************************************************************************/

#ifndef XCONFIG_TYPES_H
#define XCONFIG_TYPES_H

#include "osal_types.h"

/*===========================================================================
 * 通用接口类型定义
 *===========================================================================*/

/**
 * @brief 接口类型枚举
 */
typedef enum {
    XCONFIG_INTERFACE_NONE = 0,
    XCONFIG_INTERFACE_CAN,
    XCONFIG_INTERFACE_UART,
    XCONFIG_INTERFACE_I2C,
    XCONFIG_INTERFACE_SPI,
    XCONFIG_INTERFACE_ETHERNET,
    XCONFIG_INTERFACE_USB,
    XCONFIG_INTERFACE_SPACEWIRE,
    XCONFIG_INTERFACE_1553B,
    XCONFIG_INTERFACE_MAX
} xconfig_interface_type_t;

/**
 * @brief GPIO配置
 */
typedef struct {
    uint32 gpio_num;              /* GPIO编号 */
    uint32 pin_mux;               /* 引脚复用配置 */
    bool   active_low;            /* 低电平有效 */
    bool   pull_up;               /* 上拉使能 */
    bool   pull_down;             /* 下拉使能 */
} xconfig_gpio_config_t;

/*===========================================================================
 * 接口配置（内嵌在外设配置中）
 *===========================================================================*/

/* CAN接口配置 */
typedef struct {
    const char *device;           /* 设备名（如"can0"） */
    uint32      bitrate;          /* 波特率（bps） */
    uint32      tx_id;            /* 发送CAN ID */
    uint32      rx_id;            /* 接收CAN ID */
} xconfig_can_cfg_t;

/* UART接口配置 */
typedef struct {
    const char *device;           /* 设备名（如"/dev/ttyS0"） */
    uint32      baudrate;         /* 波特率 */
    uint8       data_bits;        /* 数据位（5-8） */
    uint8       stop_bits;        /* 停止位（1-2） */
    char        parity;           /* 校验位（'N'/'E'/'O'） */
} xconfig_uart_cfg_t;

/* I2C接口配置 */
typedef struct {
    const char *device;           /* 设备名（如"/dev/i2c-0"） */
    uint8       slave_addr;       /* 从设备地址（7位） */
    uint32      speed_hz;         /* 速度（Hz） */
} xconfig_i2c_cfg_t;

/* SPI接口配置 */
typedef struct {
    const char *device;           /* 设备名（如"/dev/spidev0.0"） */
    uint32      speed_hz;         /* 时钟频率（Hz） */
    uint8       mode;             /* SPI模式（0-3） */
    uint8       bits_per_word;    /* 每字位数 */
} xconfig_spi_cfg_t;

/* Ethernet接口配置 */
typedef struct {
    const char *interface;        /* 接口名（如"eth0"） */
    const char *ip_addr;          /* IP地址 */
    uint16      port;             /* 端口号 */
} xconfig_ethernet_cfg_t;

/* SpaceWire接口配置 */
typedef struct {
    const char *device;           /* 设备名 */
    uint32      link_speed;       /* 链路速度（Mbps） */
} xconfig_spacewire_cfg_t;

/* 1553B接口配置 */
typedef struct {
    const char *device;           /* 设备名 */
    uint8       rt_address;       /* RT地址（0-31） */
} xconfig_1553b_cfg_t;

/*===========================================================================
 * MCU外设配置
 *===========================================================================*/

/**
 * @brief MCU外设配置
 *
 * MCU作为一个完整的外设单元，包含其通信接口配置
 */
typedef struct {
    /* 外设基本信息 */
    const char *name;             /* MCU名称（如"stm32_mcu"） */
    const char *description;      /* 描述信息 */
    bool        enabled;          /* 是否启用 */

    /* 通信接口配置（使用联合体） */
    xconfig_interface_type_t interface_type;
    union {
        xconfig_can_cfg_t       can;
        xconfig_uart_cfg_t      uart;
        xconfig_i2c_cfg_t       i2c;
        xconfig_spi_cfg_t       spi;
    } interface_cfg;

    /* MCU特定配置 */
    uint32 cmd_timeout_ms;        /* 命令超时（ms） */
    uint32 retry_count;           /* 重试次数 */
    bool   enable_crc;            /* 启用CRC校验 */

    /* GPIO控制（可选） */
    xconfig_gpio_config_t *reset_gpio; /* 复位GPIO */
    xconfig_gpio_config_t *irq_gpio;   /* 中断GPIO */
} xconfig_mcu_cfg_t;

/*===========================================================================
 * BMC外设配置
 *===========================================================================*/

/**
 * @brief BMC外设配置
 *
 * BMC作为一个完整的外设单元，包含主备通道配置
 */
typedef struct {
    /* 外设基本信息 */
    const char *name;             /* BMC名称（如"payload_bmc"） */
    const char *description;      /* 描述信息 */
    bool        enabled;          /* 是否启用 */

    /* 主通道配置（以太网） */
    struct {
        xconfig_interface_type_t type; /* 必须是XCONFIG_INTERFACE_ETHERNET */
        xconfig_ethernet_cfg_t   cfg;
    } primary_channel;

    /* 备份通道配置（串口） */
    struct {
        xconfig_interface_type_t type; /* 必须是XCONFIG_INTERFACE_UART */
        xconfig_uart_cfg_t       cfg;
    } backup_channel;

    /* BMC特定配置 */
    uint32 cmd_timeout_ms;        /* 命令超时（ms） */
    uint32 retry_count;           /* 重试次数 */
    uint32 failover_threshold;    /* 故障切换阈值（连续失败次数） */

    /* GPIO控制（可选） */
    xconfig_gpio_config_t *power_gpio; /* 电源控制GPIO */
    xconfig_gpio_config_t *reset_gpio; /* 复位GPIO */
} xconfig_bmc_cfg_t;

/*===========================================================================
 * 卫星平台接口配置
 *===========================================================================*/

/**
 * @brief 卫星平台接口配置
 *
 * 卫星平台作为一个外设单元，通常使用CAN或1553B
 */
typedef struct {
    /* 外设基本信息 */
    const char *name;             /* 名称（如"satellite_platform"） */
    const char *description;      /* 描述信息 */
    bool        enabled;          /* 是否启用 */

    /* 通信接口配置（使用联合体） */
    xconfig_interface_type_t interface_type;
    union {
        xconfig_can_cfg_t      can;
        xconfig_1553b_cfg_t    bus_1553b;
        xconfig_spacewire_cfg_t spacewire;
    } interface_cfg;

    /* 卫星平台特定配置 */
    uint32 heartbeat_interval_ms; /* 心跳间隔（ms） */
    uint32 cmd_timeout_ms;        /* 命令超时（ms） */
} xconfig_satellite_cfg_t;

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

    /* 通信接口配置（使用联合体） */
    xconfig_interface_type_t interface_type;
    union {
        xconfig_i2c_cfg_t  i2c;
        xconfig_spi_cfg_t  spi;
        xconfig_uart_cfg_t uart;
    } interface_cfg;

    /* 传感器特定配置 */
    uint32 sample_rate;           /* 采样率（Hz） */
    uint32 resolution;            /* 分辨率（位） */

    /* GPIO控制（可选） */
    xconfig_gpio_config_t *power_gpio; /* 电源控制GPIO */
    xconfig_gpio_config_t *irq_gpio;   /* 中断GPIO */
} xconfig_sensor_cfg_t;

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

/*===========================================================================
 * 电源域配置
 *===========================================================================*/

/**
 * @brief 电源域配置
 */
typedef struct {
    const char *name;             /* 电源域名称 */
    xconfig_gpio_config_t *enable_gpio; /* 使能GPIO */
    uint32      voltage_mv;       /* 电压（mV） */
    uint32      current_ma;       /* 电流限制（mA） */
    uint32      startup_delay_ms; /* 启动延时（ms） */
} xconfig_power_domain_t;

/*===========================================================================
 * APP配置（应用层外设映射）
 *===========================================================================*/

/**
 * @brief 外设功能类型
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
    uint32 device_id;             /* 外设编号（第几个） */
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
    uint32 mapping_count;

    /* APP功能参数 */
    struct {
        uint32 heartbeat_interval_ms;
        uint32 cmd_timeout_ms;
        uint32 retry_count;
        uint32 queue_depth;
        uint32 failover_threshold;
    } params;
} xconfig_app_config_t;

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
    uint32                    mcu_count;

    xconfig_bmc_cfg_t       **bmcs;         /* BMC外设列表 */
    uint32                    bmc_count;

    xconfig_satellite_cfg_t **satellites;   /* 卫星平台接口列表 */
    uint32                    satellite_count;

    xconfig_sensor_cfg_t    **sensors;      /* 传感器外设列表 */
    uint32                    sensor_count;

    xconfig_storage_cfg_t   **storages;     /* 存储设备列表 */
    uint32                    storage_count;

    xconfig_power_domain_t  **power_domains; /* 电源域列表 */
    uint32                    power_domain_count;

    /* APP配置列表（新增） */
    xconfig_app_config_t    **apps;         /* APP配置列表 */
    uint32                    app_count;

    /* 扩展配置（预留） */
    void *private_data;           /* 私有数据指针 */
} xconfig_board_config_t;

#endif /* XCONFIG_TYPES_H */
