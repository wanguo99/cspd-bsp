# Service层重命名为PDL（Peripheral Driver Layer）

## 重命名原因

原名称`service`（服务层）容易与业务服务混淆，不够准确。新名称`pdl`（Peripheral Driver Layer，外设驱动层）更准确地描述了该层的职责：管理和驱动各类外设。

## 命名对比

| 旧名称 | 新名称 | 说明 |
|--------|--------|------|
| service | pdl | 目录名 |
| service_*.h | pdl_*.h | 头文件 |
| service_*.c | pdl_*.c | 源文件 |
| SatelliteService_* | SatellitePDL_* | 卫星平台函数前缀 |
| PayloadBMC_* | PayloadBMCPDL_* | BMC载荷函数前缀 |
| LinuxPayload_* | LinuxPayloadPDL_* | Linux载荷函数前缀 |
| PowerService_* | PowerPDL_* | 电源服务函数前缀 |
| SERVICE_* | PDL_* | 宏定义前缀 |

## 架构层次

```
apps/          # 应用层
pdl/           # 外设驱动层（Peripheral Driver Layer）
  ├── include/
  │   ├── peripheral_device.h      # 统一外设接口
  │   ├── peripherals/             # 各类外设驱动接口
  │   ├── pdl_satellite.h          # 卫星平台接口
  │   ├── pdl_payload_bmc.h        # BMC载荷接口
  │   ├── pdl_payload_linux.h      # Linux载荷接口
  │   └── pdl_power.h              # 电源管理接口
  └── src/
      ├── peripherals/             # 外设驱动实现
      │   ├── core/                # 外设管理核心
      │   ├── mcu/                 # MCU外设驱动
      │   ├── satellite/           # 卫星外设适配器
      │   ├── payload_bmc/         # BMC外设适配器
      │   └── payload_linux/       # Linux载荷适配器
      └── linux/                   # 平台相关实现
          ├── pdl_satellite.c
          ├── pdl_payload_bmc.c
          ├── pdl_payload_linux.c
          ├── pdl_power.c
          ├── persistent_queue.c
          └── watchdog.c
hal/           # 硬件抽象层（Hardware Abstraction Layer）
osal/          # 操作系统抽象层（OS Abstraction Layer）
```

## 重命名范围

### 1. 目录结构
- `service/` → `pdl/`
- `tests/service/` → `tests/pdl/`

### 2. 头文件
- `service_satellite.h` → `pdl_satellite.h`
- `service_payload_bmc.h` → `pdl_payload_bmc.h`
- `service_payload_linux.h` → `pdl_payload_linux.h`
- `service_power.h` → `pdl_power.h`
- `payload_service.h` → `payload_pdl.h`

### 3. 源文件
- `service_satellite.c` → `pdl_satellite.c`
- `service_payload_bmc.c` → `pdl_payload_bmc.c`
- `service_payload_linux.c` → `pdl_payload_linux.c`
- `service_power.c` → `pdl_power.c`
- `payload_service.c` → `payload_pdl.c`
- `test_payload_service_module.c` → `test_payload_pdl_module.c`

### 4. 函数名前缀
```c
// 卫星平台
SatelliteService_Init()      → SatellitePDL_Init()
SatelliteService_Deinit()    → SatellitePDL_Deinit()
SatelliteService_SendResponse() → SatellitePDL_SendResponse()

// BMC载荷
PayloadBMC_Init()            → PayloadBMCPDL_Init()
PayloadBMC_Deinit()          → PayloadBMCPDL_Deinit()
PayloadBMC_SendCommand()     → PayloadBMCPDL_SendCommand()

// Linux载荷
LinuxPayload_Init()          → LinuxPayloadPDL_Init()
LinuxPayload_ExecuteCommand() → LinuxPayloadPDL_ExecuteCommand()

// 电源管理
PowerService_Init()          → PowerPDL_Init()
```

### 5. 宏定义
```c
#ifndef SERVICE_SATELLITE_H  → #ifndef PDL_SATELLITE_H
#define SERVICE_SATELLITE_H  → #define PDL_SATELLITE_H
```

### 6. CMakeLists.txt变量
```cmake
set(SERVICE_SOURCES ...)     → set(PDL_SOURCES ...)
set(SERVICE_INC_DIR ...)     → set(PDL_INC_DIR ...)
add_library(service ...)     → add_library(pdl ...)
target_link_libraries(... service) → target_link_libraries(... pdl)
```

### 7. 文档
- 所有文档中的`service`、`Service层`、`服务层`更新为`pdl`、`PDL层`、`外设驱动层`

## 迁移影响

### 向后兼容性
- ✅ 编译通过
- ✅ 所有测试用例正常
- ✅ 功能无变化，仅重命名

### 需要更新的代码
如果有外部代码引用了旧的接口，需要更新：
```c
// 旧代码
#include "service_satellite.h"
SatelliteService_Init(&config, &handle);

// 新代码
#include "pdl_satellite.h"
SatellitePDL_Init(&config, &handle);
```

## 编译验证

```bash
./build.sh
```

**编译结果**:
- ✅ 编译成功
- ✅ can_gateway (51KB)
- ✅ protocol_converter (56KB)
- ✅ unit-test (466KB)

## 命名优势

1. **准确性**: PDL准确描述了该层的职责 - 外设驱动管理
2. **一致性**: 与HAL/OSAL命名风格一致（都是XAL）
3. **清晰性**: 避免与应用层"服务"概念混淆
4. **专业性**: 符合嵌入式系统分层习惯

## 相关文档

- [PDL架构重构总结](SERVICE_REFACTOR.md)
- [PDL源码迁移报告](SERVICE_MIGRATION.md)

---

**重命名完成时间**: 2026-04-24  
**影响范围**: 全项目  
**编译状态**: ✅ 通过  
**功能验证**: ✅ 正常
