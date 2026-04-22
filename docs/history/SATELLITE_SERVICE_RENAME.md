# Service Platform 重命名为 Satellite Service

## 重命名日期
2024-04-22

## 重命名原因

将 `service_platform` 重命名为 `service_satellite`，以更明确地表示这是与**卫星平台**的接口服务，强调"卫星"这一关键概念。

## 重命名范围

### 1. 文件重命名

| 旧文件名 | 新文件名 | 类型 |
|---------|---------|------|
| `service/inc/service_platform.h` | `service/inc/service_satellite.h` | 接口头文件 |
| `service/linux/service_platform.c` | `service/linux/service_satellite.c` | Linux 实现 |

### 2. 类型和函数重命名

#### 类型定义

| 旧名称 | 新名称 |
|-------|-------|
| `platform_service_handle_t` | `satellite_service_handle_t` |
| `platform_service_config_t` | `satellite_service_config_t` |
| `platform_service_context_t` | `satellite_service_context_t` |
| `platform_cmd_callback_t` | `satellite_cmd_callback_t` |

#### 函数接口

| 旧名称 | 新名称 |
|-------|-------|
| `PlatformService_Init()` | `SatelliteService_Init()` |
| `PlatformService_Deinit()` | `SatelliteService_Deinit()` |
| `PlatformService_RegisterCallback()` | `SatelliteService_RegisterCallback()` |
| `PlatformService_SendResponse()` | `SatelliteService_SendResponse()` |
| `PlatformService_SendHeartbeat()` | `SatelliteService_SendHeartbeat()` |
| `PlatformService_GetStats()` | `SatelliteService_GetStats()` |

#### 任务名称

| 旧名称 | 新名称 |
|-------|-------|
| `PLAT_RX` | `SAT_RX` |
| `PLAT_HB` | `SAT_HB` |

### 3. 日志消息更新

| 旧消息 | 新消息 |
|-------|-------|
| "Platform heartbeat task started" | "Satellite heartbeat task started" |
| "Platform heartbeat task stopped" | "Satellite heartbeat task stopped" |
| "Platform CAN RX task started" | "Satellite CAN RX task started" |
| "Platform CAN RX task stopped" | "Satellite CAN RX task stopped" |
| "Platform service initialized" | "Satellite service initialized" |
| "Platform service deinitialized" | "Satellite service deinitialized" |
| "Failed to allocate platform service context" | "Failed to allocate satellite service context" |

### 4. 构建系统更新

**service/CMakeLists.txt**：
```cmake
# 旧配置
set(SERVICE_SOURCES
    ${SERVICE_IMPL_DIR}/service_platform.c
    ...
)

# 新配置
set(SERVICE_SOURCES
    ${SERVICE_IMPL_DIR}/service_satellite.c
    ...
)
```

### 5. 文档更新

更新的文档文件：
- `docs/SERVICE_ARCHITECTURE.md` - Service 层架构文档
- `docs/NAMING_CONVENTION.md` - 文件命名规范
- `service/rtos/README.md` - RTOS 移植指南

更新内容：
- 所有 "Platform Service" → "Satellite Service"
- 所有 "平台接口服务" → "卫星平台接口服务"
- 所有 "平台通信" → "卫星平台通信"
- 所有函数和类型名称引用

## 使用示例对比

### 旧代码示例

```c
#include "service_platform.h"

platform_service_handle_t platform;
platform_service_config_t config = {
    .can_device = "can0",
    .can_bitrate = 500000,
    .heartbeat_interval_ms = 1000
};

PlatformService_Init(&config, &platform);
PlatformService_RegisterCallback(platform, cmd_handler, NULL);
PlatformService_SendResponse(platform, seq, STATUS_OK, result);
```

### 新代码示例

```c
#include "service_satellite.h"

satellite_service_handle_t satellite;
satellite_service_config_t config = {
    .can_device = "can0",
    .can_bitrate = 500000,
    .heartbeat_interval_ms = 1000
};

SatelliteService_Init(&config, &satellite);
SatelliteService_RegisterCallback(satellite, cmd_handler, NULL);
SatelliteService_SendResponse(satellite, seq, STATUS_OK, result);
```

## 迁移指南

如果有外部代码使用了旧的 API，需要进行以下更新：

### 1. 更新头文件包含

```c
// 旧代码
#include "service_platform.h"

// 新代码
#include "service_satellite.h"
```

### 2. 更新类型声明

```c
// 旧代码
platform_service_handle_t handle;
platform_service_config_t config;

// 新代码
satellite_service_handle_t handle;
satellite_service_config_t config;
```

### 3. 更新函数调用

使用查找替换功能：
- `PlatformService_` → `SatelliteService_`
- `platform_service_` → `satellite_service_`
- `platform_cmd_` → `satellite_cmd_`

### 4. 重新编译

```bash
cd cspd-bsp
rm -rf build
./build.sh
```

## 影响范围

### 直接影响
- Service 层的卫星平台接口模块
- 依赖该模块的应用层代码（如 protocol_converter）

### 无影响
- OSAL 层
- HAL 层
- 其他 Service 模块（payload_bmc, payload_linux, power）
- 配置文件

## 验证清单

- [x] 文件重命名完成
- [x] 头文件宏定义更新
- [x] 类型定义更新
- [x] 函数声明和实现更新
- [x] 日志消息更新
- [x] CMakeLists.txt 更新
- [x] 文档更新
- [x] RTOS 移植指南更新

## 总结

此次重命名使服务名称更加明确和具体：
- **旧名称**：Platform Service（平台服务）- 含义较宽泛
- **新名称**：Satellite Service（卫星平台服务）- 明确指向卫星平台

这样的命名更符合项目的实际应用场景，提高了代码的可读性和可维护性。
