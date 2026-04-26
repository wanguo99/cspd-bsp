# XConfig - 硬件配置库

## 概述

XConfig是一个类似Linux设备树的硬件配置管理库，以外设为单位描述和管理不同平台、不同产品的硬件配置。

## 主要特性

- **以外设为单位**：配置以MCU、BMC、传感器等外设为核心
- **接口内嵌**：每个外设配置内嵌其通信接口配置
- **多平台支持**：支持TI AM6254、演示平台等
- **运行时查询**：支持按平台/产品/版本查询配置
- **纯数据结构**：平台无关，无系统调用

## 支持的平台

- **TI AM6254**: H200-100P（100P算力）、H200-32P（32P算力）
- **演示平台**: 演示项目（2P算力，用于模拟测试）

## 支持的外设类型

- **MCU外设** - 微控制器单元
- **BMC外设** - 基板管理控制器
- **卫星平台接口** - 卫星平台通信接口
- **传感器外设** - 各类传感器
- **存储设备** - eMMC、NVMe等存储设备

## 快速开始

```c
#include "xconfig_api.h"

int main(void)
{
    /* 初始化 */
    HW_Config_Init();
    HW_Config_RegisterAll();
    
    /* 选择配置 */
    const xconfig_board_config_t *board = HW_Config_SelectDefault();
    
    /* 查找MCU外设 */
    const xconfig_mcu_cfg_t *mcu = XCONFIG_HW_FindMCU(board, "stm32_mcu");
    if (mcu != NULL) {
        LOG_INFO("Main", "MCU接口: %s", 
                 mcu->interface_type == HW_INTERFACE_UART ? "UART" : "CAN");
    }
    
    return 0;
}
```

## 文档导航

- **[完整文档](docs/README.md)** - 详细的XConfig文档
- **[架构设计](docs/ARCHITECTURE.md)** - 设计理念和核心概念
- **[API参考](docs/API_REFERENCE.md)** - API接口文档
- **[使用指南](docs/USAGE_GUIDE.md)** - 使用示例和最佳实践

## 目录结构

```
xconfig/
├── include/                    # 头文件
│   ├── api/                   # 对外API
│   ├── internal/              # 内部头文件
│   └── peripheral/            # 外设配置头文件
├── src/                       # 源代码
├── platform/                  # 平台配置
│   ├── ti/am6254/            # TI AM6254平台
│   └── vendor_demo/          # 演示平台
├── docs/                      # 文档
│   ├── README.md             # 完整文档
│   ├── ARCHITECTURE.md       # 架构设计
│   ├── API_REFERENCE.md      # API参考
│   └── USAGE_GUIDE.md        # 使用指南
└── CMakeLists.txt            # 构建配置
```

## 设计理念

XConfig采用**外设为单位**的设计，每个外设配置包含其通信接口配置，更符合硬件设计思维。

**传统方式**（接口为单位）：
```
配置CAN接口 → 配置UART接口 → 在代码中指定哪个接口连接哪个外设
```

**XConfig方式**（外设为单位）：
```c
xconfig_mcu_cfg_t mcu = {
    .name = "stm32_mcu",
    .interface_type = HW_INTERFACE_UART,  /* 接口内嵌 */
    .interface_cfg.uart = {
        .device = "/dev/ttyS1",
        .baudrate = 115200
    }
};
```

## 相关文档

- [PDL层文档](../pdl/docs/README.md) - PDL层使用XConfig配置
- [项目总体架构](../docs/ARCHITECTURE.md)
