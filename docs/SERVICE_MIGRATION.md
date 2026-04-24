# PDL层源码迁移完成报告

## 迁移目标

将现有的传统服务实现（pdl_satellite、pdl_payload_bmc、pdl_payload_linux）迁移到统一的外设框架，通过适配器模式实现兼容。

## 迁移策略

采用**适配器模式**：
- 保留现有服务实现（src/linux/pdl_*.c）作为底层驱动
- 创建外设驱动适配器（src/peripherals/*/peripheral_*.c）包装现有服务
- 适配器实现统一的peripheral_ops_t接口
- 上层应用可选择使用传统接口或新的外设框架接口

## 迁移架构

```
┌─────────────────────────────────────────────────────────┐
│              应用层 (Apps)                               │
│  可选择使用传统接口或外设框架接口                          │
└─────────────────────────────────────────────────────────┘
                          │
        ┌─────────────────┴─────────────────┐
        │                                   │
┌───────▼──────────┐            ┌──────────▼──────────┐
│  传统服务接口     │            │  外设框架接口        │
│  pdl_*.h     │            │  peripheral_device.h │
└───────┬──────────┘            └──────────┬──────────┘
        │                                   │
        │                       ┌───────────▼───────────┐
        │                       │  外设驱动适配器        │
        │                       │  peripheral_*.c       │
        │                       └───────────┬───────────┘
        │                                   │
        └───────────────┬───────────────────┘
                        │
            ┌───────────▼───────────┐
            │  传统服务实现（底层）   │
            │  pdl_*.c          │
            └───────────┬───────────┘
                        │
            ┌───────────▼───────────┐
            │  HAL层                │
            └───────────────────────┘
```

## 已完成的迁移

### 1. 卫星平台外设驱动适配器

**文件**: `pdl/src/peripherals/satellite/peripheral_satellite.c`

**功能映射**:
- `satellite_driver_init` → `SatelliteService_Init`
- `satellite_driver_deinit` → `SatelliteService_Deinit`
- `satellite_driver_get_status` → `SatelliteService_GetStats`
- `satellite_driver_execute_command` → `SatelliteService_SendResponse`
- `satellite_driver_get_stats` → `SatelliteService_GetStats`

**外设能力**:
- ✅ 状态查询 (PERIPHERAL_CAP_STATUS_QUERY)
- ✅ 命令执行 (PERIPHERAL_CAP_COMMAND_EXEC)
- ✅ 心跳机制 (PERIPHERAL_CAP_HEARTBEAT)
- ❌ 电源控制 (卫星平台不支持)
- ❌ 复位 (卫星平台不支持)

**注册函数**:
```c
int32 Satellite_RegisterDriver(void);
int32 Satellite_Create(const satellite_pdl_config_t *config, peripheral_handle_t *handle);
int32 Satellite_Destroy(peripheral_handle_t handle);
```

### 2. BMC载荷外设驱动适配器

**文件**: `pdl/src/peripherals/payload_bmc/peripheral_bmc.c`

**功能映射**:
- `bmc_driver_init` → `PayloadBMC_Init`
- `bmc_driver_deinit` → `PayloadBMC_Deinit`
- `bmc_driver_power_on` → `BMCPayload_PowerOn`
- `bmc_driver_power_off` → `BMCPayload_PowerOff`
- `bmc_driver_reset` → `BMCPayload_PowerReset`
- `bmc_driver_get_status` → `PayloadBMC_GetStatus`
- `bmc_driver_execute_command` → `PayloadBMC_SendCommand`
- `bmc_driver_get_stats` → `PayloadBMC_GetStats`

**外设能力**:
- ✅ 电源控制 (PERIPHERAL_CAP_POWER_CONTROL)
- ✅ 复位 (PERIPHERAL_CAP_RESET)
- ✅ 状态查询 (PERIPHERAL_CAP_STATUS_QUERY)
- ✅ 传感器读取 (PERIPHERAL_CAP_SENSOR_READ)
- ✅ 命令执行 (PERIPHERAL_CAP_COMMAND_EXEC)

**注册函数**:
```c
int32 BMC_RegisterDriver(void);
int32 BMC_Create(const bmc_payload_config_t *config, peripheral_handle_t *handle);
int32 BMC_Destroy(peripheral_handle_t handle);
```

**特殊处理**:
- 实现文件中混用了`PayloadBMC_*`和`BMCPayload_*`两种命名
- 适配器中添加了extern声明以使用内部函数

### 3. Linux载荷外设驱动适配器

**文件**: `pdl/src/peripherals/payload_linux/peripheral_linux.c`

**功能映射**:
- `linux_driver_init` → `LinuxPayload_Init`
- `linux_driver_deinit` → `LinuxPayload_Deinit`
- `linux_driver_power_off` → `LinuxPayload_Shutdown`
- `linux_driver_reset` → `LinuxPayload_Reboot`
- `linux_driver_get_status` → `LinuxPayload_GetSystemStatus`
- `linux_driver_execute_command` → `LinuxPayload_ExecuteCommand`
- `linux_driver_get_stats` → `LinuxPayload_GetStats`

**外设能力**:
- ✅ 复位 (PERIPHERAL_CAP_RESET)
- ✅ 状态查询 (PERIPHERAL_CAP_STATUS_QUERY)
- ✅ 命令执行 (PERIPHERAL_CAP_COMMAND_EXEC)
- ❌ 电源控制 (通常由BMC控制)

**注册函数**:
```c
int32 Linux_RegisterDriver(void);
int32 Linux_Create(const linux_payload_config_t *config, peripheral_handle_t *handle);
int32 Linux_Destroy(peripheral_handle_t handle);
```

**特殊处理**:
- `linux_system_status_t`不包含温度字段，适配器中设置为0.0f

## 目录结构

```
pdl/
├── include/
│   ├── peripheral_device.h              # 外设抽象接口
│   ├── peripherals/
│   │   └── peripheral_mcu.h             # MCU外设驱动接口
│   ├── pdl_satellite.h              # 传统卫星服务接口（保留）
│   ├── pdl_payload_bmc.h            # 传统BMC服务接口（保留）
│   ├── pdl_payload_linux.h          # 传统Linux服务接口（保留）
│   └── ...
└── src/
    ├── peripherals/
    │   ├── core/
    │   │   └── peripheral_core.c        # 外设管理核心
    │   ├── mcu/
    │   │   └── peripheral_mcu.c         # MCU外设驱动
    │   ├── satellite/
    │   │   └── peripheral_satellite.c   # 卫星外设适配器 ✅
    │   ├── payload_bmc/
    │   │   └── peripheral_bmc.c         # BMC外设适配器 ✅
    │   └── payload_linux/
    │       └── peripheral_linux.c       # Linux载荷适配器 ✅
    └── linux/
        ├── pdl_satellite.c          # 卫星服务实现（底层）
        ├── pdl_payload_bmc.c        # BMC服务实现（底层）
        ├── pdl_payload_linux.c      # Linux服务实现（底层）
        ├── pdl_power.c              # 电源服务实现
        ├── persistent_queue.c           # 持久化队列
        └── watchdog.c                   # 看门狗
```

## CMakeLists.txt更新

```cmake
set(SERVICE_SOURCES
    # 外设管理核心
    ${CMAKE_CURRENT_SOURCE_DIR}/src/peripherals/core/peripheral_core.c

    # 外设驱动适配器
    ${CMAKE_CURRENT_SOURCE_DIR}/src/peripherals/mcu/peripheral_mcu.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/peripherals/satellite/peripheral_satellite.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/peripherals/payload_bmc/peripheral_bmc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/peripherals/payload_linux/peripheral_linux.c

    # 传统服务实现（底层）
    ${CMAKE_CURRENT_SOURCE_DIR}/src/linux/pdl_satellite.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/linux/pdl_payload_bmc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/linux/pdl_payload_linux.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/linux/pdl_power.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/linux/persistent_queue.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/linux/watchdog.c
)
```

## 使用示例

### 方式1：使用传统接口（向后兼容）

```c
/* 传统方式 - 继续工作 */
satellite_pdl_config_t sat_cfg = {...};
satellite_pdl_handle_t sat_handle;
SatelliteService_Init(&sat_cfg, &sat_handle);
SatelliteService_SendResponse(sat_handle, seq, status, result);
SatelliteService_Deinit(sat_handle);
```

### 方式2：使用外设框架接口（推荐）

```c
/* 注册所有外设驱动 */
Satellite_RegisterDriver();
BMC_RegisterDriver();
Linux_RegisterDriver();
MCU_RegisterDriver();

/* 创建外设实例 */
satellite_pdl_config_t sat_cfg = {...};
peripheral_handle_t sat_peripheral;
Satellite_Create(&sat_cfg, &sat_peripheral);

bmc_payload_config_t bmc_cfg = {...};
peripheral_handle_t bmc_peripheral;
BMC_Create(&bmc_cfg, &bmc_peripheral);

/* 统一的外设操作 */
peripheral_status_t status;
Peripheral_GetStatus(sat_peripheral, &status);
Peripheral_GetStatus(bmc_peripheral, &status);

peripheral_info_t info;
Peripheral_GetInfo(sat_peripheral, &info);
printf("Device: %s, Type: %d\n", info.name, info.type);

/* 统一的电源控制 */
Peripheral_PowerOn(bmc_peripheral);
Peripheral_PowerOff(bmc_peripheral);
Peripheral_Reset(bmc_peripheral);

/* 销毁外设 */
Satellite_Destroy(sat_peripheral);
BMC_Destroy(bmc_peripheral);
```

### 方式3：混合使用

```c
/* 可以混合使用两种接口 */
peripheral_handle_t bmc;
BMC_Create(&bmc_cfg, &bmc);

/* 使用外设框架接口 */
Peripheral_PowerOn(bmc);

/* 也可以获取底层句柄使用传统接口 */
bmc_peripheral_context_t *ctx = (bmc_peripheral_context_t *)bmc;
BMCPayload_ReadSensors(ctx->pdl_handle, ...);
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

## 迁移优势

### 1. 向后兼容
- 现有代码无需修改，继续使用传统接口
- 渐进式迁移，降低风险

### 2. 统一抽象
- 所有外设使用统一的peripheral_ops_t接口
- 简化上层应用开发

### 3. 灵活性
- 可以选择使用传统接口或外设框架接口
- 可以混合使用两种接口

### 4. 可扩展性
- 新增外设类型只需实现适配器
- 不影响现有代码

### 5. 易于测试
- 适配器层可独立测试
- 底层服务实现不变，测试用例继续有效

## 后续工作

### 短期
- [ ] 为适配器添加单元测试
- [ ] 完善错误处理和日志记录
- [ ] 添加外设热插拔支持

### 中期
- [ ] 逐步将上层应用迁移到外设框架接口
- [ ] 优化适配器性能
- [ ] 添加外设状态监控和告警

### 长期
- [ ] 考虑将底层服务实现重构为纯外设驱动
- [ ] 移除传统接口（当所有应用迁移完成后）
- [ ] 外设驱动独立成库

## 注意事项

1. **函数命名不一致**: BMC服务实现中混用了`PayloadBMC_*`和`BMCPayload_*`两种命名，适配器中使用extern声明解决

2. **结构体字段差异**: `linux_system_status_t`不包含温度字段，适配器中设置默认值

3. **能力限制**: 
   - 卫星平台不支持电源控制和复位
   - Linux载荷通常不支持直接电源控制（由BMC控制）

4. **性能考虑**: 适配器增加了一层间接调用，但开销可忽略

5. **线程安全**: 底层服务实现已有互斥锁保护，适配器无需额外处理

---

**迁移完成时间**: 2026-04-24  
**迁移方式**: 适配器模式  
**兼容性**: 100%向后兼容  
**编译状态**: ✅ 通过
