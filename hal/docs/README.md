# HAL - 硬件抽象层

## 概述

HAL (Hardware Abstraction Layer) 是PMC-BSP的硬件抽象层，封装硬件驱动接口，隔离硬件平台差异。HAL层位于OSAL之上，为PDL层和应用层提供统一的硬件访问接口。

## 主要特性

- **平台隔离**：硬件相关代码隔离在`src/linux/`等平台目录
- **统一接口**：提供跨平台的硬件访问API
- **驱动封装**：CAN、串口等硬件驱动的高层封装
- **配置管理**：硬件配置集中在`include/config/`目录
- **OSAL依赖**：必须使用OSAL封装的系统调用，不直接调用系统API

## 支持的硬件

### CAN总线
- **实现**：基于SocketCAN（Linux）
- **特性**：标准帧/扩展帧、ID过滤、统计信息
- **接口**：`hal_can.h`

### 串口 (UART)
- **实现**：基于termios（Linux）
- **特性**：9600-4000000波特率、原始模式、灵活超时
- **接口**：`hal_serial.h`

## 快速开始

### CAN驱动示例

```c
#include "hal_can.h"

int main(void)
{
    hal_can_config_t config = {
        .device = "can0",
        .bitrate = 500000,
        .mode = HAL_CAN_MODE_NORMAL
    };
    
    hal_can_handle_t handle;
    int32_t ret = HAL_CAN_Init(&config, &handle);
    if (ret != OS_SUCCESS) {
        LOG_ERROR("Main", "CAN初始化失败");
        return -1;
    }
    
    /* 发送CAN帧 */
    hal_can_frame_t frame = {
        .can_id = 0x123,
        .can_dlc = 8,
        .data = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}
    };
    HAL_CAN_Send(&handle, &frame, 1000);
    
    /* 接收CAN帧 */
    hal_can_frame_t rx_frame;
    ret = HAL_CAN_Receive(&handle, &rx_frame, 5000);
    if (ret == OS_SUCCESS) {
        LOG_INFO("Main", "接收CAN帧: ID=0x%X", rx_frame.can_id);
    }
    
    HAL_CAN_Deinit(&handle);
    return 0;
}
```

### 串口驱动示例

```c
#include "hal_serial.h"

int main(void)
{
    hal_serial_config_t config = {
        .device = "/dev/ttyS0",
        .baudrate = 115200,
        .data_bits = 8,
        .stop_bits = 1,
        .parity = 'N'
    };
    
    hal_serial_handle_t handle;
    HAL_Serial_Init(&config, &handle);
    
    /* 发送数据 */
    const uint8_t data[] = "Hello UART\n";
    HAL_Serial_Send(&handle, data, sizeof(data), 1000);
    
    /* 接收数据 */
    uint8_t buffer[128];
    int32_t len = HAL_Serial_Receive(&handle, buffer, sizeof(buffer), 5000);
    if (len > 0) {
        LOG_INFO("Main", "接收: %.*s", len, buffer);
    }
    
    HAL_Serial_Deinit(&handle);
    return 0;
}
```

## 文档导航

- **[架构设计](ARCHITECTURE.md)** - HAL内部架构和驱动实现
- **[API参考](API_REFERENCE.md)** - 完整的驱动接口文档
- **[使用指南](USAGE_GUIDE.md)** - 驱动使用示例和最佳实践

## 目录结构

```
hal/
├── include/                    # 公共头文件
│   ├── hal_can.h              # CAN驱动接口
│   ├── hal_serial.h           # 串口驱动接口
│   └── config/                # 硬件配置
│       ├── can_types.h        # CAN帧类型定义
│       ├── can_config.h       # CAN配置
│       └── uart_config.h      # 串口配置
├── src/linux/                 # Linux平台实现
│   ├── hal_can_linux.c        # CAN驱动（SocketCAN）
│   └── hal_serial_linux.c     # 串口驱动（termios）
├── docs/                      # 文档
│   ├── README.md              # 本文件
│   ├── ARCHITECTURE.md        # 架构设计
│   ├── API_REFERENCE.md       # API参考
│   └── USAGE_GUIDE.md         # 使用指南
└── CMakeLists.txt             # 构建配置
```

## 设计原则

1. **平台隔离**：HAL是唯一允许包含硬件平台相关代码的层（除OSAL外）
2. **OSAL依赖**：必须使用OSAL封装的系统调用（`OSAL_socket()`, `OSAL_open()`等）
3. **统一接口**：所有平台实现相同的接口
4. **错误处理**：所有函数返回int32状态码
5. **资源管理**：使用句柄管理硬件资源

## 平台支持

当前支持的平台：
- **Linux** (`src/linux/`) - 基于SocketCAN和termios

计划支持的平台：
- **TI AM6254** (`src/ti_am62/`) - 硬件CAN控制器
- **NXP i.MX8** (`src/nxp_imx8/`) - 硬件CAN控制器

## 依赖关系

```
应用层 (Apps)
    ↓
外设驱动层 (PDL)
    ↓
硬件抽象层 (HAL) ← 你在这里
    ↓
操作系统抽象层 (OSAL)
    ↓
Linux/POSIX 或 RTOS
```

## 配置说明

### CAN配置

配置文件：`include/config/can_config.h`

```c
#define HAL_CAN_DEFAULT_DEVICE   "can0"
#define HAL_CAN_DEFAULT_BITRATE  500000
```

### 串口配置

配置文件：`include/config/uart_config.h`

```c
#define HAL_UART_DEFAULT_DEVICE   "/dev/ttyS0"
#define HAL_UART_DEFAULT_BAUDRATE 115200
```

## 测试

```bash
# 运行HAL层测试
./output/target/bin/unit-test -L HAL

# 运行CAN驱动测试
./output/target/bin/unit-test -m test_hal_can
```

## 相关文档

- [OSAL层文档](../../osal/docs/README.md)
- [PDL层文档](../../pdl/docs/README.md)
- [项目总体架构](../../docs/ARCHITECTURE.md)
- [编码规范](../../docs/CODING_STANDARDS.md)
