# Service 层架构设计

## 概述

Service 层位于 HAL 和 Apps 之间，提供业务逻辑封装和高级服务接口。它将底层硬件操作抽象为面向业务的服务，使应用层无需关心硬件细节。

## 架构分层

```
┌─────────────────────────────────────────┐
│          Apps (应用层)                   │
│  - CAN Gateway                          │
│  - Protocol Converter                   │
└─────────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────────┐
│        Service (服务层)                  │
│  - Satellite Service (卫星平台接口)     │
│  - BMC Payload Service (BMC载荷)        │
│  - Linux Payload Service (Linux载荷)    │
│  - Power Service (电源管理)             │
└─────────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────────┐
│          HAL (硬件抽象层)                │
│  - CAN Driver                           │
│  - Network Driver                       │
│  - Serial Driver                        │
└─────────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────────┐
│          OSAL (操作系统抽象层)           │
│  - Task, Queue, Mutex, etc.            │
└─────────────────────────────────────────┘
```

## Service 模块

### 1. Satellite Service (卫星平台接口服务)

**职责**：
- 封装与卫星平台的 CAN 总线通信
- 处理卫星平台命令请求和响应
- 管理心跳和状态上报
- 提供统一的卫星平台交互接口

**接口**：`service/inc/service_satellite.h`

**实现**：
- Linux: `service/linux/service_satellite.c`
- RTOS: `service/rtos/service_satellite.c` (待实现)

**主要功能**：
- `SatelliteService_Init()` - 初始化服务
- `SatelliteService_RegisterCallback()` - 注册命令回调
- `SatelliteService_SendResponse()` - 发送响应
- `SatelliteService_SendHeartbeat()` - 发送心跳

**使用场景**：
```c
// 应用层使用示例
satellite_service_handle_t satellite;
satellite_service_config_t config = {
    .can_device = "can0",
    .can_bitrate = 500000,
    .heartbeat_interval_ms = 1000
};

SatelliteService_Init(&config, &satellite);
SatelliteService_RegisterCallback(satellite, cmd_handler, NULL);
```

### 2. BMC Payload Service (BMC载荷服务)

**职责**：
- 与带 BMC 的载荷通信（IPMI/Redfish 协议）
- 支持多通道（网络/串口）自动切换
- 电源控制、状态查询、传感器读取
- 故障检测和自动恢复

**接口**：`service/inc/service_payload_bmc.h`

**实现**：
- Linux: `service/linux/service_payload_bmc.c`
- RTOS: `service/rtos/service_payload_bmc.c` (待实现)

**主要功能**：
- `BMCPayload_Init()` - 初始化服务
- `BMCPayload_PowerOn/Off/Reset()` - 电源控制
- `BMCPayload_GetPowerState()` - 查询电源状态
- `BMCPayload_ReadSensors()` - 读取传感器
- `BMCPayload_ExecuteCommand()` - 执行原始 IPMI 命令
- `BMCPayload_SwitchChannel()` - 切换通信通道

**使用场景**：
```c
// 应用层使用示例
bmc_payload_handle_t bmc;
bmc_payload_config_t config = {
    .network = {
        .ip_addr = "192.168.1.100",
        .port = 623,
        .username = "admin",
        .password = "admin"
    },
    .auto_switch = true
};

BMCPayload_Init(&config, &bmc);
BMCPayload_PowerOn(bmc);

bmc_power_state_t state;
BMCPayload_GetPowerState(bmc, &state);
```

### 3. Linux Payload Service (通用Linux载荷服务)

**职责**：
- 与不带 BMC 但运行 Linux 系统的载荷通信
- 支持 SSH、HTTP API、串口终端
- 系统状态查询、进程管理、文件传输
- 远程命令执行

**接口**：`service/inc/service_payload_linux.h`

**实现**：
- Linux: `service/linux/service_payload_linux.c` (框架)
- RTOS: `service/rtos/service_payload_linux.c` (待实现)

**主要功能**：
- `LinuxPayload_Init()` - 初始化服务
- `LinuxPayload_ExecuteCommand()` - 执行远程命令
- `LinuxPayload_GetSystemStatus()` - 获取系统状态
- `LinuxPayload_GetProcessList()` - 获取进程列表
- `LinuxPayload_StartProcess/StopProcess()` - 进程管理
- `LinuxPayload_UploadFile/DownloadFile()` - 文件传输
- `LinuxPayload_Reboot/Shutdown()` - 系统控制

**使用场景**：
```c
// 应用层使用示例
linux_payload_handle_t linux_pl;
linux_payload_config_t config = {
    .ssh = {
        .ip_addr = "192.168.1.101",
        .port = 22,
        .username = "root",
        .password = "password"
    },
    .primary_comm = LINUX_COMM_SSH
};

LinuxPayload_Init(&config, &linux_pl);

char output[1024];
int32 exit_code;
LinuxPayload_ExecuteCommand(linux_pl, "uptime", output, sizeof(output), &exit_code);
```

### 4. Power Service (电源管理服务)

**职责**：
- 统一的电源控制接口
- 支持多种载荷类型（BMC/Linux/Generic）
- 电源状态监控和记录
- 电源策略管理（定时开关机、节能模式）
- 故障保护（过流、过温保护）

**接口**：`service/inc/service_power.h`

**实现**：
- Linux: `service/linux/service_power.c`
- RTOS: `service/rtos/service_power.c` (待实现)

**主要功能**：
- `PowerService_Init()` - 初始化服务
- `PowerService_PowerOn/Off()` - 电源开关
- `PowerService_Reset()` - 电源复位
- `PowerService_Standby()` - 待机模式
- `PowerService_GetStatus()` - 获取电源状态
- `PowerService_SchedulePowerOn/Off()` - 定时开关机
- `PowerService_EnableProtection()` - 启用保护功能

**使用场景**：
```c
// 应用层使用示例
power_service_handle_t power;
power_service_config_t config = {
    .type = PAYLOAD_TYPE_BMC,
    .payload_handle = bmc_handle,
    .protection = {
        .max_current_a = 10.0f,
        .max_temp_c = 85.0f
    },
    .auto_recovery = true
};

PowerService_Init(&config, &power);
PowerService_PowerOn(power);

// 定时关机
PowerService_SchedulePowerOff(power, 3600);  // 1小时后关机
```

## 跨平台支持

### 目录结构

```
service/
├── inc/                          # 接口定义（平台无关）
│   ├── service_satellite.h
│   ├── service_payload_bmc.h
│   ├── service_payload_linux.h
│   └── service_power.h
├── linux/                        # Linux 实现
│   ├── service_satellite.c
│   ├── service_payload_bmc.c
│   ├── service_payload_linux.c
│   └── service_power.c
├── rtos/                         # RTOS 实现（预留）
│   └── README.md
└── CMakeLists.txt
```

### 实现原则

1. **接口统一**：所有平台使用相同的头文件接口
2. **实现分离**：不同平台的实现放在各自目录
3. **依赖抽象**：只依赖 OSAL 和 HAL，不直接调用系统 API
4. **行为一致**：不同平台实现相同的功能和行为

## 设计原则

### 1. 单一职责
每个服务只负责一个明确的业务领域：
- Satellite Service → 卫星平台通信
- BMC Payload Service → BMC 载荷管理
- Linux Payload Service → Linux 载荷管理
- Power Service → 电源管理

### 2. 依赖倒置
Service 层依赖 HAL 抽象接口，不依赖具体实现：
```c
// 正确：依赖 HAL 接口
#include "hal_network.h"
HAL_Network_Send(handle, data, len);

// 错误：直接调用系统 API
send(sockfd, data, len, 0);
```

### 3. 开闭原则
对扩展开放，对修改关闭：
- 添加新服务：创建新的头文件和实现
- 添加新平台：在新目录实现相同接口
- 不修改现有代码

### 4. 接口隔离
每个服务提供最小必要接口：
- 不暴露内部实现细节
- 使用 opaque handle（void*）隐藏上下文
- 只提供业务相关的函数

## 与其他层的交互

### Service → HAL
Service 层调用 HAL 层的硬件驱动：
```c
// Satellite Service 使用 CAN 驱动
HAL_CAN_Send(can_handle, &frame);

// BMC Payload Service 使用网络和串口驱动
HAL_Network_Send(net_handle, data, len);
HAL_Serial_Write(serial_handle, data, len);
```

### Apps → Service
应用层调用 Service 层的业务接口：
```c
// Protocol Converter 使用 Satellite Service
SatelliteService_SendResponse(satellite, seq, status, result);

// Protocol Converter 使用 BMC Payload Service
BMCPayload_PowerOn(bmc);
```

### Service → OSAL
Service 层使用 OSAL 的系统服务：
```c
// 任务管理
OS_TaskCreate(&task_id, "SVC_TASK", task_func, arg, ...);

// 同步机制
OS_MutexLock(mutex);
OS_MutexUnlock(mutex);

// 日志输出
LOG_INFO("Service initialized");
```

## 扩展指南

### 添加新服务

1. **定义接口**：在 `service/inc/` 创建头文件
2. **实现 Linux 版本**：在 `service/linux/` 创建实现
3. **预留 RTOS 版本**：在 `service/rtos/` 添加说明
4. **更新 CMakeLists.txt**：添加源文件
5. **编写文档**：更新本文档

### 移植到新平台

1. **创建平台目录**：如 `service/freertos/`
2. **复制接口实现**：从 Linux 版本开始
3. **替换系统调用**：使用 OSAL/HAL 接口
4. **测试验证**：确保行为一致
5. **更新构建系统**：修改 CMakeLists.txt

## 总结

Service 层是 CSPD-BSP 框架的核心业务逻辑层，它：
- 封装复杂的业务逻辑
- 提供简洁的高级接口
- 支持多平台移植
- 便于测试和维护

通过合理的分层和抽象，Service 层使应用开发更加简单，同时保持了良好的可扩展性和可维护性。
