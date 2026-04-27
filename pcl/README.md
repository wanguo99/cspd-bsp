# PCL - 硬件配置库

## 概述

PCL是一个类似Linux设备树的硬件配置管理库，以外设为单位描述和管理不同平台、不同产品的硬件配置。

## 主要特性

- **以外设为单位**：配置以MCU、BMC、传感器等外设为核心
- **接口内嵌**：每个外设配置内嵌其通信接口配置
- **多平台支持**：支持TI AM6254、演示平台等
- **运行时查询**：支持按平台/产品/版本查询配置
- **纯数据结构**：平台无关，无系统调用

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

**PCL方式（外设为单位）**：
```c
/* MCU外设配置（包含其通信接口） */
pcl_mcu_cfg_t mcu_stm32 = {
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

## 编译说明

### 快速开始

```bash
# 在项目根目录编译整个项目（包含PCL）
./build.sh              # Release模式
./build.sh -d           # Debug模式
```

### 单独编译PCL模块

```bash
# 方法1: 使用CMake直接编译
mkdir -p output/build && cd output/build
cmake ../.. -DCMAKE_BUILD_TYPE=Release
make pcl -j$(nproc)
cd ../..

# 方法2: 在已配置的构建目录中编译
cd output/build
make pcl -j$(nproc)
cd ../..
```

### 支持的编译参数

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `CMAKE_BUILD_TYPE` | STRING | Release | 编译类型：Release/Debug |
| `BUILD_XCONFIG_EXAMPLES` | BOOL | OFF | 是否编译示例程序 |

### 配置编译参数

**编译示例程序**：
```bash
cd output/build
cmake ../.. -DBUILD_XCONFIG_EXAMPLES=ON
make pcl_example -j$(nproc)
cd ../..

# 运行示例
./output/target/bin/pcl_example
```

**Debug模式编译**：
```bash
cd output/build
cmake ../.. -DCMAKE_BUILD_TYPE=Debug
make pcl -j$(nproc)
cd ../..
```

### 编译输出

```
output/
├── build/
│   └── lib/
│       └── libpcl.a       # PCL静态库
└── target/
    └── bin/
        └── pcl_example    # 示例程序（可选）
```

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
#include "pcl_api.h"

int main(void)
{
    /* 初始化 */
    HW_Config_Init();
    HW_Config_RegisterAll();
    
    /* 选择配置 */
    const pcl_board_config_t *board = HW_Config_SelectDefault();
    
    /* 查找MCU外设 */
    const pcl_mcu_cfg_t *mcu = XCONFIG_HW_FindMCU(board, "stm32_mcu");
    if (mcu != NULL) {
        LOG_INFO("Main", "MCU接口: %s", 
                 mcu->interface_type == HW_INTERFACE_UART ? "UART" : "CAN");
    }
    
    return 0;
}
```

## 文档导航

- **[完整文档](docs/README.md)** - 详细的PCL文档
- **[架构设计](docs/ARCHITECTURE.md)** - 设计理念和核心概念
- **[API参考](docs/API_REFERENCE.md)** - API接口文档
- **[使用指南](docs/USAGE_GUIDE.md)** - 使用示例和最佳实践

## 目录结构

```
pcl/
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

PCL采用**外设为单位**的设计，每个外设配置包含其通信接口配置，更符合硬件设计思维。

**传统方式**（接口为单位）：
```
配置CAN接口 → 配置UART接口 → 在代码中指定哪个接口连接哪个外设
```

**PCL方式**（外设为单位）：
```c
pcl_mcu_cfg_t mcu = {
    .name = "stm32_mcu",
    .interface_type = HW_INTERFACE_UART,  /* 接口内嵌 */
    .interface_cfg.uart = {
        .device = "/dev/ttyS1",
        .baudrate = 115200
    }
};
```

## 相关文档

- [PDL层文档](../pdl/docs/README.md) - PDL层使用PCL配置
- [项目总体架构](../docs/ARCHITECTURE.md)
