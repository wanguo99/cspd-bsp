# PDL - 外设驱动层

## 概述

PDL (Peripheral Driver Layer) 是PMC-BSP的外设驱动层，统一管理卫星平台、BMC载荷、MCU等外设。PDL层将这些设备抽象为"外设"，提供统一的服务接口。

## 主要特性

- **统一外设管理**：卫星/载荷/MCU统一抽象为外设
- **多通道冗余**：支持主备双通道（如BMC的以太网+串口）
- **自动故障切换**：5次连续失败自动切换通道
- **心跳机制**：卫星平台5秒心跳检测
- **协议支持**：IPMI、Redfish、自定义协议

## 支持的外设服务

### 卫星平台服务 (Satellite PDL)
- **通信方式**：CAN总线或1553B
- **功能**：命令接收、遥测上报、心跳检测
- **接口**：`pdl_satellite.h`

### BMC载荷服务 (BMC PDL)
- **通信方式**：以太网（主）+ 串口（备）
- **协议**：IPMI over LAN、Redfish
- **功能**：电源管理、传感器读取、固件升级
- **接口**：`pdl_bmc.h`

### MCU外设服务 (MCU PDL)
- **通信方式**：CAN、UART、I2C、SPI
- **功能**：版本查询、固件升级、GPIO控制
- **接口**：`pdl_mcu.h`

## 快速开始

### 卫星平台服务示例

```c
#include "pdl_satellite.h"

/* 命令回调 */
void cmd_callback(uint32_t cmd_id, const uint8_t *data, uint32_t len)
{
    LOG_INFO("Satellite", "收到命令: 0x%X", cmd_id);
}

int main(void)
{
    satellite_pdl_config_t config = {
        .can_device = "can0",
        .can_bitrate = 500000,
        .heartbeat_interval_ms = 5000,
        .cmd_callback = cmd_callback
    };
    
    SatellitePDL_Init(&config);
    
    /* 发送遥测 */
    uint8_t telemetry[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    SatellitePDL_SendTelemetry(0x200, telemetry, sizeof(telemetry));
    
    /* 运行... */
    OSAL_TaskDelay(60000);
    
    SatellitePDL_Deinit();
    return 0;
}
```

## 文档导航

- **[架构设计](ARCHITECTURE.md)** - PDL内部架构和服务实现
- **[API参考](API_REFERENCE.md)** - 完整的服务接口文档
- **[使用指南](USAGE_GUIDE.md)** - 服务使用示例和最佳实践

## 目录结构

```
pdl/
├── include/                        # 公共头文件
│   ├── pdl_satellite.h            # 卫星平台服务接口
│   ├── pdl_bmc.h                  # BMC载荷服务接口
│   └── pdl_mcu.h                  # MCU外设服务接口
├── src/                           # 源代码
│   ├── pdl_satellite/             # 卫星平台服务实现
│   ├── pdl_bmc/                   # BMC载荷服务实现
│   └── pdl_mcu/                   # MCU外设服务实现
├── docs/                          # 文档
│   ├── README.md                  # 本文件
│   ├── ARCHITECTURE.md            # 架构设计
│   ├── API_REFERENCE.md           # API参考
│   └── USAGE_GUIDE.md             # 使用指南
└── CMakeLists.txt                 # 构建配置
```

## 设计原则

1. **平台无关**：PDL层必须保持完全平台无关，通过HAL/OSAL访问底层
2. **外设抽象**：将卫星/载荷/MCU统一抽象为外设
3. **冗余设计**：关键外设支持多通道冗余
4. **自动恢复**：故障自动切换，无需人工干预

## 依赖关系

```
应用层 (Apps)
    ↓
外设驱动层 (PDL) ← 你在这里
    ↓
硬件配置层 (XConfig)
    ↓
硬件抽象层 (HAL)
    ↓
操作系统抽象层 (OSAL)
```

## 测试

```bash
# 运行PDL层测试
./output/target/bin/unit-test -L PDL

# 运行卫星平台服务测试
./output/target/bin/unit-test -m test_pdl_satellite
```

## 相关文档

- [HAL层文档](../../hal/docs/README.md)
- [XConfig层文档](../../xconfig/docs/README.md)
- [项目总体架构](../../docs/ARCHITECTURE.md)
