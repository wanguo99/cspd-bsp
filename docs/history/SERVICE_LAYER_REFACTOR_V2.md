# Service 层重构记录

## 日期
2024-04-22

## 背景

原架构中只有一个 `payload_service`，职责不清晰，将所有载荷相关的功能混在一起。用户提出应该按照业务领域划分多个独立的服务模块。

## 用户需求

> 在我的设想中，应该是增加一个service层，然后在service层中，增加类似service_payload_bmc.c（与带BMC的载荷交互）、service_payload_generic_linux.c（与不带BMC但是运行Linux系统的载荷交互）、service_power_manager.c（电源管理服务）、service_site_interface.c（与卫星平台交互）的服务

## 重构方案

### 1. Service 层模块划分

将单一的 `payload_service` 拆分为 4 个独立服务：

#### service_platform (平台接口服务)
- **职责**：与卫星平台的 CAN 总线通信
- **功能**：
  - 封装 CAN 协议
  - 处理平台命令
  - 心跳管理
  - 状态上报

#### service_payload_bmc (BMC载荷服务)
- **职责**：与带 BMC 的载荷通信
- **功能**：
  - IPMI/Redfish 协议
  - 多通道（网络/串口）
  - 电源控制
  - 传感器读取
  - 自动故障切换

#### service_payload_linux (Linux载荷服务)
- **职责**：与通用 Linux 载荷通信
- **功能**：
  - SSH 远程命令执行
  - HTTP API 调用
  - 串口终端
  - 系统状态查询
  - 进程管理
  - 文件传输

#### service_power (电源管理服务)
- **职责**：统一的电源管理
- **功能**：
  - 支持多种载荷类型
  - 电源开关控制
  - 状态监控
  - 定时任务
  - 故障保护

### 2. 跨平台支持

Service 层也需要支持 Linux 和 RTOS 平台：

```
service/
├── inc/                    # 接口定义（平台无关）
├── linux/                  # Linux 实现
└── rtos/                   # RTOS 实现（预留）
```

### 3. 目录结构

**新结构**：
```
service/
├── inc/
│   ├── service_platform.h
│   ├── service_payload_bmc.h
│   ├── service_payload_linux.h
│   └── service_power.h
├── linux/
│   ├── service_platform.c
│   ├── service_payload_bmc.c
│   ├── service_payload_linux.c
│   └── service_power.c
├── rtos/
│   └── README.md
└── CMakeLists.txt
```

**旧结构**（已删除）：
```
service/
├── inc/
│   └── payload_service.h
└── linux/
    └── payload_service_linux.c
```

## 实现细节

### 1. service_platform

**接口设计**：
```c
typedef void* platform_service_handle_t;

int32 PlatformService_Init(const platform_service_config_t *config,
                           platform_service_handle_t *handle);
int32 PlatformService_RegisterCallback(platform_service_handle_t handle,
                                       platform_cmd_callback_t callback,
                                       void *user_data);
int32 PlatformService_SendResponse(platform_service_handle_t handle,
                                   uint32 seq_num,
                                   can_status_t status,
                                   uint32 result);
int32 PlatformService_SendHeartbeat(platform_service_handle_t handle,
                                    can_status_t status);
```

**实现要点**：
- 封装 HAL_CAN 接口
- 创建 CAN 接收任务
- 创建心跳任务
- 命令回调机制

### 2. service_payload_bmc

**接口设计**：
```c
typedef void* bmc_payload_handle_t;

int32 BMCPayload_Init(const bmc_payload_config_t *config,
                      bmc_payload_handle_t *handle);
int32 BMCPayload_PowerOn(bmc_payload_handle_t handle);
int32 BMCPayload_PowerOff(bmc_payload_handle_t handle);
int32 BMCPayload_PowerReset(bmc_payload_handle_t handle);
int32 BMCPayload_GetPowerState(bmc_payload_handle_t handle,
                               bmc_power_state_t *state);
int32 BMCPayload_ReadSensors(bmc_payload_handle_t handle,
                             bmc_sensor_type_t type,
                             bmc_sensor_reading_t *readings,
                             uint32 max_count,
                             uint32 *actual_count);
int32 BMCPayload_ExecuteCommand(bmc_payload_handle_t handle,
                                const char *cmd,
                                char *response,
                                uint32 resp_size);
int32 BMCPayload_SwitchChannel(bmc_payload_handle_t handle,
                               bmc_channel_t channel);
```

**实现要点**：
- 使用 HAL_Network 和 HAL_Serial
- 多通道管理
- 自动故障切换
- IPMI 命令封装

### 3. service_payload_linux

**接口设计**：
```c
typedef void* linux_payload_handle_t;

int32 LinuxPayload_Init(const linux_payload_config_t *config,
                        linux_payload_handle_t *handle);
int32 LinuxPayload_ExecuteCommand(linux_payload_handle_t handle,
                                  const char *command,
                                  char *output,
                                  uint32 output_size,
                                  int32 *exit_code);
int32 LinuxPayload_GetSystemStatus(linux_payload_handle_t handle,
                                   linux_system_status_t *status);
int32 LinuxPayload_GetProcessList(linux_payload_handle_t handle,
                                  linux_process_info_t *processes,
                                  uint32 max_count,
                                  uint32 *actual_count);
int32 LinuxPayload_StartProcess(linux_payload_handle_t handle,
                                const char *command,
                                uint32 *pid);
int32 LinuxPayload_StopProcess(linux_payload_handle_t handle,
                               uint32 pid,
                               bool force);
int32 LinuxPayload_UploadFile(linux_payload_handle_t handle,
                              const char *local_path,
                              const char *remote_path);
int32 LinuxPayload_DownloadFile(linux_payload_handle_t handle,
                                const char *remote_path,
                                const char *local_path);
int32 LinuxPayload_Reboot(linux_payload_handle_t handle);
int32 LinuxPayload_Shutdown(linux_payload_handle_t handle);
```

**实现要点**：
- SSH 客户端（需要 libssh）
- HTTP API 客户端
- 串口终端
- 当前为框架实现，预留接口

### 4. service_power

**接口设计**：
```c
typedef void* power_service_handle_t;

int32 PowerService_Init(const power_service_config_t *config,
                        power_service_handle_t *handle);
int32 PowerService_PowerOn(power_service_handle_t handle);
int32 PowerService_PowerOff(power_service_handle_t handle, bool force);
int32 PowerService_Reset(power_service_handle_t handle);
int32 PowerService_Standby(power_service_handle_t handle);
int32 PowerService_GetStatus(power_service_handle_t handle,
                             power_status_t *status);
int32 PowerService_RegisterCallback(power_service_handle_t handle,
                                    power_event_callback_t callback,
                                    void *user_data);
int32 PowerService_SchedulePowerOn(power_service_handle_t handle,
                                   uint32 delay_sec);
int32 PowerService_SchedulePowerOff(power_service_handle_t handle,
                                    uint32 delay_sec);
int32 PowerService_CancelSchedule(power_service_handle_t handle);
int32 PowerService_EnableProtection(power_service_handle_t handle,
                                    bool enable);
int32 PowerService_ClearFault(power_service_handle_t handle);
```

**实现要点**：
- 支持多种载荷类型（BMC/Linux/Generic）
- 统一的电源控制接口
- 定时任务管理
- 故障保护机制
- 事件回调

## 文件变更

### 新增文件

**接口定义**：
- `service/inc/service_platform.h` (2.8KB)
- `service/inc/service_payload_bmc.h` (5.2KB)
- `service/inc/service_payload_linux.h` (5.8KB)
- `service/inc/service_power.h` (4.6KB)

**Linux 实现**：
- `service/linux/service_platform.c` (6.2KB)
- `service/linux/service_payload_bmc.c` (5.8KB)
- `service/linux/service_payload_linux.c` (6.4KB, 框架)
- `service/linux/service_power.c` (9.2KB)

**文档**：
- `service/rtos/README.md` - RTOS 实现指南
- `docs/SERVICE_ARCHITECTURE.md` - Service 层架构文档
- `docs/history/SERVICE_LAYER_REFACTOR_V2.md` - 本文档

### 删除文件

- `service/inc/payload_service.h`
- `service/linux/payload_service_linux.c`

### 修改文件

- `service/CMakeLists.txt` - 更新源文件列表

## 设计优势

### 1. 职责清晰
每个服务只负责一个明确的业务领域，符合单一职责原则。

### 2. 易于扩展
- 添加新服务：创建新的头文件和实现
- 添加新功能：在对应服务中扩展
- 不影响其他服务

### 3. 便于测试
每个服务可以独立测试，降低测试复杂度。

### 4. 跨平台支持
接口统一，实现分离，便于移植到不同平台。

### 5. 代码复用
服务可以在不同项目中复用。

## 应用层适配

应用层需要根据实际需求选择使用哪些服务：

```c
// 示例：Protocol Converter 使用多个服务

// 1. 初始化平台服务
platform_service_handle_t platform;
PlatformService_Init(&platform_cfg, &platform);
PlatformService_RegisterCallback(platform, cmd_handler, NULL);

// 2. 初始化 BMC 载荷服务
bmc_payload_handle_t bmc;
BMCPayload_Init(&bmc_cfg, &bmc);

// 3. 初始化电源服务
power_service_handle_t power;
power_cfg.type = PAYLOAD_TYPE_BMC;
power_cfg.payload_handle = bmc;
PowerService_Init(&power_cfg, &power);

// 4. 处理命令
void cmd_handler(can_cmd_type_t cmd, uint32 param, void *data)
{
    switch (cmd) {
        case CMD_TYPE_POWER_ON:
            PowerService_PowerOn(power);
            break;
        case CMD_TYPE_POWER_OFF:
            PowerService_PowerOff(power, false);
            break;
        // ...
    }
}
```

## 后续工作

### 短期
1. 完善 BMC Payload Service 的 IPMI 命令实现
2. 实现 Linux Payload Service 的 SSH 客户端（需要 libssh）
3. 完善 Power Service 的保护功能
4. 更新应用层代码使用新的服务接口

### 中期
1. 添加单元测试
2. 实现 RTOS 平台版本
3. 优化性能和内存使用
4. 添加更多传感器类型支持

### 长期
1. 支持更多载荷类型
2. 实现更智能的故障检测和恢复
3. 添加日志记录和诊断功能
4. 支持远程升级

## 总结

通过将单一的 `payload_service` 拆分为 4 个独立的服务模块，实现了：
- 更清晰的职责划分
- 更好的代码组织
- 更强的可扩展性
- 更容易的维护

这次重构使 Service 层真正成为一个业务逻辑层，为应用层提供了丰富而清晰的服务接口。
