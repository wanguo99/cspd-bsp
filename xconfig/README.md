# 硬件配置库（XConfig）

## 概述

XConfig是一个类似Linux设备树（Device Tree）的硬件配置管理库，**以外设为单位**描述和管理不同平台、不同产品的硬件配置。

## 设计理念

- **以外设为单位**：配置以MCU、BMC、传感器等外设为核心，而非以接口类型为单位
- **配置与代码分离**：硬件配置以结构体形式定义，与业务代码解耦
- **接口内嵌**：每个外设配置内嵌其通信接口配置（使用联合体节省内存）
- **类型安全**：使用联合体（union）节省内存，同时保持类型安全
- **运行时查询**：支持按平台/产品/版本查询配置
- **易于扩展**：新增平台或产品只需添加配置文件

## 核心概念

### 外设为单位 vs 接口为单位

**传统方式（接口为单位）**：
```
配置CAN接口 → 配置UART接口 → 配置I2C接口
然后在代码中指定哪个接口连接哪个外设
```

**XConfig方式（外设为单位）**：
```c
/* MCU外设配置（包含其通信接口） */
xconfig_mcu_cfg_t mcu_stm32 = {
    .name = "stm32_mcu",
    .interface_type = HW_INTERFACE_UART,  /* 使用UART通信 */
    .interface_cfg.uart = {
        .device = "/dev/ttyS1",
        .baudrate = 115200,
        /* ... */
    },
    /* MCU特定配置 */
    .cmd_timeout_ms = 500,
    .enable_crc = true,
    /* ... */
};
```

**优势**：
- 配置更直观：一个外设的所有配置集中在一起
- 易于理解：从外设角度思考，而非从接口角度
- 便于移植：更换通信接口只需修改外设配置，无需改动代码逻辑

## 目录结构

```
xconfig/
├── include/                           # 头文件
│   ├── xconfig.h                     # 总头文件
│   ├── xconfig_api.h                 # API接口
│   ├── xconfig_common.h              # 通用类型（GPIO、电源域）
│   ├── xconfig_hardware_interface.h  # 硬件接口定义
│   ├── xconfig_mcu.h                 # MCU外设配置
│   ├── xconfig_bmc.h                 # BMC外设配置
│   ├── xconfig_satellite.h           # 卫星平台接口配置
│   ├── xconfig_sensor.h              # 传感器外设配置
│   ├── xconfig_storage.h             # 存储设备配置
│   ├── xconfig_app.h                 # APP配置
│   └── xconfig_board.h               # 板级配置
├── src/                               # 源代码
│   ├── xconfig_api.c                 # API实现
│   └── xconfig_register.c            # 配置注册
├── platform/                          # 平台配置（嵌套目录结构）
│   ├── ti/am625/                     # TI AM625平台
│   │   ├── H200_100P/                # H200-100P产品（100P算力）
│   │   │   ├── h200_100p_payload_base.c  # 基础配置
│   │   │   ├── h200_100p_payload_v1.c    # V1.0配置
│   │   │   └── h200_100p_payload_v2.c    # V2.0配置
│   │   └── H200_32P/                 # H200-32P产品（32P算力）
│   │       ├── h200_32p_payload_base.c   # 基础配置
│   │       ├── h200_32p_payload_v1.c     # V1.0配置
│   │       └── h200_32p_payload_v2.c     # V2.0配置
│   └── platform_demo/                # 演示平台
│       └── project_demo/             # 演示项目（2P算力，演示用）
│           ├── product_demo_base.c       # 基础配置
│           ├── product_demo_v1.c         # V1.0配置
│           └── product_demo_v2.c         # V2.0配置
├── examples/                          # 示例代码
│   └── xconfig_example.c             # 使用示例
└── CMakeLists.txt                     # 构建配置
```

## 支持的平台和产品

### TI AM625平台

**H200-100P系列**（100P算力）：
- `xconfig_h200_100p_base` - 基础配置
- `xconfig_h200_100p_v1` - V1.0（增加冗余MCU和IMU传感器）
- `xconfig_h200_100p_v2` - V2.0（升级1553B、GPS、NVMe）

**H200-32P系列**（32P算力）：
- `xconfig_h200_32p_base` - 基础配置
- `xconfig_h200_32p_v1` - V1.0（增加冗余MCU和IMU传感器）
- `xconfig_h200_32p_v2` - V2.0（升级1553B、GPS、NVMe）

### 演示平台

**演示项目系列**（2P算力，演示/模拟测试用）：
- `xconfig_demo_base` - 基础配置
- `xconfig_demo_v1` - V1.0（增加冗余MCU和IMU传感器）
- `xconfig_demo_v2` - V2.0（升级1553B、GPS、NVMe）

> **注意**：产品名称中的"P"表示算力（Computing Power），如100P表示100 PFLOPS算力，而非PCIe通道数。

## 支持的外设类型

### 1. MCU外设（xconfig_mcu_cfg_t）

MCU作为一个完整的外设单元，包含其通信接口配置。

```c
typedef struct {
    const char *name;                 /* MCU名称 */
    bool enabled;                     /* 是否启用 */
    
    /* 通信接口配置（使用联合体） */
    xconfig_interface_type_t interface_type;
    union {
        xconfig_can_cfg_t  can;
        xconfig_uart_cfg_t uart;
        xconfig_i2c_cfg_t  i2c;
        xconfig_spi_cfg_t  spi;
    } interface_cfg;
    
    /* MCU特定配置 */
    uint32 cmd_timeout_ms;
    uint32 retry_count;
    bool enable_crc;
    
    /* GPIO控制 */
    xconfig_gpio_config_t *reset_gpio;
    xconfig_gpio_config_t *irq_gpio;
} xconfig_mcu_cfg_t;
```

**示例**：
```c
static xconfig_mcu_cfg_t mcu_stm32 = {
    .name = "stm32_mcu",
    .enabled = true,
    .interface_type = HW_INTERFACE_UART,
    .interface_cfg.uart = {
        .device = "/dev/ttyS1",
        .baudrate = 115200,
        .data_bits = 8,
        .stop_bits = 1,
        .parity = 'N'
    },
    .cmd_timeout_ms = 500,
    .retry_count = 3,
    .enable_crc = true,
    .reset_gpio = &gpio_mcu_reset,
    .irq_gpio = &gpio_mcu_irq
};
```

### 2. BMC外设（xconfig_bmc_cfg_t）

BMC作为一个完整的外设单元，包含主备双通道配置。

```c
typedef struct {
    const char *name;
    bool enabled;
    
    /* 主通道配置（以太网） */
    struct {
        xconfig_interface_type_t type;
        xconfig_ethernet_cfg_t cfg;
    } primary_channel;
    
    /* 备份通道配置（串口） */
    struct {
        xconfig_interface_type_t type;
        xconfig_uart_cfg_t cfg;
    } backup_channel;
    
    /* BMC特定配置 */
    uint32 cmd_timeout_ms;
    uint32 failover_threshold;
    
    /* GPIO控制 */
    xconfig_gpio_config_t *power_gpio;
    xconfig_gpio_config_t *reset_gpio;
} xconfig_bmc_cfg_t;
```

**示例**：
```c
static xconfig_bmc_cfg_t bmc_payload = {
    .name = "payload_bmc",
    .enabled = true,
    .primary_channel = {
        .type = HW_INTERFACE_ETHERNET,
        .cfg = {
            .interface = "eth0",
            .ip_addr = "192.168.1.100",
            .port = 623
        }
    },
    .backup_channel = {
        .type = HW_INTERFACE_UART,
        .cfg = {
            .device = "/dev/ttyS2",
            .baudrate = 115200,
            .data_bits = 8,
            .stop_bits = 1,
            .parity = 'N'
        }
    },
    .cmd_timeout_ms = 2000,
    .failover_threshold = 5
};
```

### 3. 卫星平台接口（xconfig_satellite_cfg_t）

卫星平台作为一个外设单元，通常使用CAN或1553B。

```c
typedef struct {
    const char *name;
    bool enabled;
    
    /* 通信接口配置 */
    xconfig_interface_type_t interface_type;
    union {
        xconfig_can_cfg_t can;
        xconfig_1553b_cfg_t bus_1553b;
        xconfig_spacewire_cfg_t spacewire;
    } interface_cfg;
    
    /* 卫星平台特定配置 */
    uint32 heartbeat_interval_ms;
    uint32 cmd_timeout_ms;
} xconfig_satellite_cfg_t;
```

### 4. 传感器外设（xconfig_sensor_cfg_t）

传感器作为一个外设单元，包含其通信接口和采样配置。

```c
typedef struct {
    const char *name;
    sensor_type_t type;
    bool enabled;
    
    /* 通信接口配置 */
    xconfig_interface_type_t interface_type;
    union {
        xconfig_i2c_cfg_t i2c;
        xconfig_spi_cfg_t spi;
        xconfig_uart_cfg_t uart;
    } interface_cfg;
    
    /* 传感器特定配置 */
    uint32 sample_rate;
    uint32 resolution;
    
    /* GPIO控制 */
    xconfig_gpio_config_t *power_gpio;
} xconfig_sensor_cfg_t;
```

### 5. 存储设备（xconfig_storage_cfg_t）

存储设备配置。

```c
typedef struct {
    const char *name;
    storage_type_t type;
    bool enabled;
    const char *device_path;
    uint64 capacity_mb;
    uint32 block_size;
    xconfig_gpio_config_t *power_gpio;
} xconfig_storage_cfg_t;
```

## 板级配置结构

顶层配置结构，以外设为单位描述整个板子：

```c
typedef struct {
    /* 板级信息 */
    const char *platform;
    const char *product;
    const char *version;
    const char *description;
    
    /* 外设配置列表（以外设为单位） */
    xconfig_mcu_cfg_t **mcus;
    uint32 mcu_count;
    
    xconfig_bmc_cfg_t **bmcs;
    uint32 bmc_count;
    
    xconfig_satellite_cfg_t **satellites;
    uint32 satellite_count;
    
    xconfig_sensor_cfg_t **sensors;
    uint32 sensor_count;
    
    xconfig_storage_cfg_t **storages;
    uint32 storage_count;
    
    xconfig_power_domain_t **power_domains;
    uint32 power_domain_count;
} xconfig_board_config_t;
```

## API接口

### 初始化和注册

```c
/* 初始化配置库 */
int32 HW_Config_Init(void);

/* 注册所有配置 */
int32 HW_Config_RegisterAll(void);

/* 选择默认配置 */
const xconfig_board_config_t* HW_Config_SelectDefault(void);
```

### 外设配置查询（以外设为单位）

```c
/* 查找MCU外设配置 */
const xconfig_mcu_cfg_t* XCONFIG_HW_FindMCU(const xconfig_board_config_t *board,
                                       const char *name);

/* 查找BMC外设配置 */
const xconfig_bmc_cfg_t* XCONFIG_HW_FindBMC(const xconfig_board_config_t *board,
                                       const char *name);

/* 查找卫星平台接口配置 */
const xconfig_satellite_cfg_t* XCONFIG_HW_FindSatellite(const xconfig_board_config_t *board,
                                                    const char *name);

/* 查找传感器外设配置 */
const xconfig_sensor_cfg_t* XCONFIG_HW_FindSensor(const xconfig_board_config_t *board,
                                             const char *name);

/* 查找存储设备配置 */
const xconfig_storage_cfg_t* XCONFIG_HW_FindStorage(const xconfig_board_config_t *board,
                                                const char *name);
```

## 使用示例

### 1. 基本使用

```c
#include "xconfig_api.h"

int main(void)
{
    /* 初始化 */
    HW_Config_Init();
    HW_Config_RegisterAll();
    
    /* 选择配置 */
    const xconfig_board_config_t *board = HW_Config_SelectDefault();
    
    /* 打印配置 */
    HW_Config_Print(board);
    
    return 0;
}
```

### 2. 查找MCU外设并初始化

```c
/* 获取板级配置 */
const xconfig_board_config_t *board = HW_Config_GetBoard();

/* 查找MCU外设配置 */
const xconfig_mcu_cfg_t *mcu_cfg = XCONFIG_HW_FindMCU(board, "stm32_mcu");
if (mcu_cfg == NULL) {
    printf("MCU not found\n");
    return -1;
}

/* 根据接口类型初始化 */
if (mcu_cfg->interface_type == HW_INTERFACE_UART) {
    /* 使用UART配置初始化MCU */
    uart_init(mcu_cfg->interface_cfg.uart.device,
              mcu_cfg->interface_cfg.uart.baudrate);
} else if (mcu_cfg->interface_type == HW_INTERFACE_CAN) {
    /* 使用CAN配置初始化MCU */
    can_init(mcu_cfg->interface_cfg.can.device,
             mcu_cfg->interface_cfg.can.bitrate);
}
```

### 3. 查找BMC外设并初始化双通道

```c
/* 查找BMC外设配置 */
const xconfig_bmc_cfg_t *bmc_cfg = XCONFIG_HW_FindBMC(board, "payload_bmc");

/* 初始化主通道（以太网） */
if (bmc_cfg->primary_channel.type == HW_INTERFACE_ETHERNET) {
    eth_init(bmc_cfg->primary_channel.cfg.interface,
             bmc_cfg->primary_channel.cfg.ip_addr,
             bmc_cfg->primary_channel.cfg.port);
}

/* 初始化备份通道（串口） */
if (bmc_cfg->backup_channel.type == HW_INTERFACE_UART) {
    uart_init(bmc_cfg->backup_channel.cfg.device,
              bmc_cfg->backup_channel.cfg.baudrate);
}
```

### 4. 遍历所有传感器外设

```c
const xconfig_board_config_t *board = HW_Config_GetBoard();

for (uint32 i = 0; i < board->sensor_count; i++) {
    const xconfig_sensor_cfg_t *sensor = board->sensors[i];
    
    if (!sensor->enabled) {
        continue;
    }
    
    printf("Sensor: %s\n", sensor->name);
    printf("  Type: %d\n", sensor->type);
    printf("  Interface: %s\n",
           sensor->interface_type == HW_INTERFACE_I2C ? "I2C" :
           sensor->interface_type == HW_INTERFACE_SPI ? "SPI" : "Unknown");
    
    /* 根据接口类型初始化传感器 */
    if (sensor->interface_type == HW_INTERFACE_I2C) {
        i2c_sensor_init(sensor->interface_cfg.i2c.device,
                        sensor->interface_cfg.i2c.slave_addr);
    }
}
```

## 配置文件示例

完整的配置文件示例（`h200_100p_payload_base.c`）：

```c
#include "xconfig.h"

/* GPIO定义 */
static xconfig_gpio_config_t gpio_mcu_reset = {
    .gpio_num = 42,
    .active_low = true,
    .pull_up = true
};

/* MCU外设配置 */
static xconfig_mcu_cfg_t mcu_stm32 = {
    .name = "stm32_mcu",
    .enabled = true,
    .interface_type = XCONFIG_HW_INTERFACE_UART,
    .interface_cfg.uart = {
        .device = "/dev/ttyS1",
        .baudrate = 115200,
        .data_bits = 8,
        .stop_bits = 1,
        .parity = 'N'
    },
    .cmd_timeout_ms = 500,
    .retry_count = 3,
    .enable_crc = true,
    .reset_gpio = &gpio_mcu_reset
};

/* BMC外设配置 */
static xconfig_bmc_cfg_t bmc_payload = {
    .name = "payload_bmc",
    .enabled = true,
    .primary_channel = {
        .protocol = XCONFIG_BMC_PROTOCOL_IPMI,
        .cfg.ipmi_lan = {
            .interface = "eth0",
            .ip_addr = "192.168.1.100",
            .port = 623,
            .username = "admin",
            .password = NULL
        }
    },
    .backup_channel = {
        .protocol = XCONFIG_BMC_PROTOCOL_IPMI,
        .cfg = {
            .device = "/dev/ttyS2",
            .baudrate = 115200,
            .data_bits = 8,
            .stop_bits = 1,
            .parity = 'N'
        }
    },
    .cmd_timeout_ms = 2000,
    .failover_threshold = 5
};

/* 外设列表 */
static xconfig_mcu_cfg_t *mcu_list[] = { &mcu_stm32 };
static xconfig_bmc_cfg_t *bmc_list[] = { &bmc_payload };

/* 板级配置 */
const xconfig_board_config_t xconfig_h200_100p_base = {
    .platform = "ti/am625",
    .product = "H200_100P",
    .version = "base",
    .description = "H200-100P Payload Adapter Board (100P Computing Power) - Base Configuration",
    
    .mcus = mcu_list,
    .mcu_count = 1,
    
    .bmcs = bmc_list,
    .bmc_count = 1,
    
    /* ... */
};
```

## 添加新外设配置

### 步骤1：定义外设配置

```c
/* 新增MCU外设 */
static xconfig_mcu_cfg_t mcu_new = {
    .name = "new_mcu",
    .enabled = true,
    .interface_type = HW_INTERFACE_CAN,  /* 使用CAN通信 */
    .interface_cfg.can = {
        .device = "can1",
        .bitrate = 500000,
        .tx_id = 0x400,
        .rx_id = 0x500
    },
    .cmd_timeout_ms = 500,
    .retry_count = 3,
    .enable_crc = true
};
```

### 步骤2：添加到外设列表

```c
static xconfig_mcu_cfg_t *mcu_list[] = {
    &mcu_stm32,
    &mcu_new  /* 添加新MCU */
};
```

### 步骤3：更新板级配置

```c
const xconfig_board_config_t xconfig_xxx = {
    /* ... */
    .mcus = mcu_list,
    .mcu_count = sizeof(mcu_list) / sizeof(mcu_list[0]),
    /* ... */
};
```

## 设计优势

1. **外设为中心**：从外设角度思考配置，更符合硬件设计思维
2. **配置集中**：一个外设的所有配置（接口+参数）集中在一起
3. **内存高效**：使用联合体，每个外设只占用最大接口类型的内存
4. **类型安全**：编译时检查配置类型
5. **易于维护**：修改外设通信接口只需修改配置，无需改动代码
6. **支持冗余**：BMC等外设原生支持主备双通道配置
7. **航天适配**：原生支持SpaceWire、1553B等航天接口

## 与传统配置方式对比

| 特性 | 传统方式（接口为单位） | XConfig（外设为单位） |
|------|----------------------|---------------------|
| 配置组织 | 按接口类型分组 | 按外设分组 |
| 配置查找 | 先找接口，再关联外设 | 直接查找外设 |
| 接口变更 | 需修改多处配置 | 只修改外设配置 |
| 代码可读性 | 需理解接口-外设映射 | 直观清晰 |
| 冗余支持 | 需额外设计 | 原生支持（如BMC双通道） |

## 注意事项

1. **配置不可变**：所有配置使用 `const` 修饰，运行时只读
2. **指针生命周期**：配置中的字符串指针必须指向静态存储区
3. **GPIO编号**：GPIO编号与芯片相关，需参考芯片手册
4. **接口设备名**：设备名（如"/dev/ttyS0"）需与实际系统匹配
5. **联合体使用**：访问联合体前必须先检查 `interface_type` 字段

## 未来扩展

- [ ] 支持配置继承和合并（base + 版本差异）
- [ ] 支持配置序列化/反序列化（保存到文件）
- [ ] 支持配置热更新（不重启切换配置）
- [ ] 增加配置可视化工具
- [ ] 支持更多平台（Xilinx Zynq, NXP i.MX等）
