# PDL层架构重构总结

## 重构目标

将PDL层从传统的"业务外设驱动层"重构为"外设驱动层"，以管理板为核心，将卫星平台、载荷（BMC/Linux）、MCU等外部设备统一抽象为外设。

## 新架构设计

### 核心理念

**管理板为核心，外设为卫星/载荷/BMC/MCU**

```
                    管理板 (CSPD-BSP)
                          |
        +----------------+----------------+
        |                |                |
    卫星平台          载荷设备           MCU外设
    (CAN总线)      (BMC/Linux)      (I2C/SPI/UART)
```

### 外设抽象层架构

```
+--------------------------------------------------+
|              应用层 (Apps)                        |
+--------------------------------------------------+
|          外设管理框架 (Peripheral Framework)       |
|  +--------------------------------------------+  |
|  |  统一外设接口 (peripheral_device.h)         |  |
|  +--------------------------------------------+  |
|  |  外设驱动注册表 (Driver Registry)           |  |
|  +--------------------------------------------+  |
+--------------------------------------------------+
|              外设驱动层                           |
|  +------------+  +------------+  +------------+  |
|  | 卫星驱动   |  | BMC驱动    |  | MCU驱动    |  |
|  | (CAN)     |  | (IPMI)    |  | (I2C/SPI)  |  |
|  +------------+  +------------+  +------------+  |
+--------------------------------------------------+
|              硬件抽象层 (HAL)                     |
+--------------------------------------------------+
```

## 目录结构

```
pdl/
├── include/
│   ├── peripheral_device.h          # 外设抽象接口
│   ├── peripherals/
│   │   ├── peripheral_mcu.h         # MCU外设驱动接口
│   │   ├── peripheral_satellite.h   # 卫星外设驱动接口（TODO）
│   │   ├── peripheral_bmc.h         # BMC外设驱动接口（TODO）
│   │   └── peripheral_linux.h       # Linux载荷驱动接口（TODO）
│   ├── config/
│   │   └── watchdog_config.h
│   └── [传统服务接口保留兼容]
└── src/
    ├── peripherals/
    │   ├── core/
    │   │   └── peripheral_core.c    # 外设管理核心实现
    │   ├── mcu/
    │   │   └── peripheral_mcu.c     # MCU外设驱动实现
    │   ├── satellite/               # 卫星外设驱动（TODO）
    │   ├── payload_bmc/             # BMC外设驱动（TODO）
    │   └── payload_linux/           # Linux载荷驱动（TODO）
    └── linux/
        └── [传统服务实现保留兼容]
```

## 核心接口设计

### 1. 外设类型定义

```c
typedef enum
{
    PERIPHERAL_TYPE_SATELLITE = 0,    /* 卫星平台（CAN总线） */
    PERIPHERAL_TYPE_PAYLOAD_BMC = 1,  /* BMC载荷（IPMI/Redfish） */
    PERIPHERAL_TYPE_PAYLOAD_LINUX = 2,/* Linux载荷（SSH/网络） */
    PERIPHERAL_TYPE_MCU = 3,          /* MCU外设（I2C/SPI/UART） */
    PERIPHERAL_TYPE_GENERIC = 4       /* 通用外设（GPIO控制） */
} peripheral_type_t;
```

### 2. 通信接口类型

```c
typedef enum
{
    COMM_INTERFACE_CAN = 0,
    COMM_INTERFACE_ETHERNET = 1,
    COMM_INTERFACE_UART = 2,
    COMM_INTERFACE_I2C = 3,
    COMM_INTERFACE_SPI = 4,
    COMM_INTERFACE_GPIO = 5
} comm_interface_t;
```

### 3. 外设能力标志

```c
typedef enum
{
    PERIPHERAL_CAP_POWER_CONTROL = (1 << 0),  /* 支持电源控制 */
    PERIPHERAL_CAP_RESET = (1 << 1),          /* 支持复位 */
    PERIPHERAL_CAP_STATUS_QUERY = (1 << 2),   /* 支持状态查询 */
    PERIPHERAL_CAP_SENSOR_READ = (1 << 3),    /* 支持传感器读取 */
    PERIPHERAL_CAP_COMMAND_EXEC = (1 << 4),   /* 支持命令执行 */
    PERIPHERAL_CAP_FIRMWARE_UPDATE = (1 << 5),/* 支持固件升级 */
    PERIPHERAL_CAP_HEARTBEAT = (1 << 6)       /* 支持心跳机制 */
} peripheral_capability_t;
```

### 4. 外设操作接口（虚函数表）

```c
typedef struct
{
    int32 (*init)(void *config, peripheral_handle_t *handle);
    int32 (*deinit)(peripheral_handle_t handle);
    int32 (*power_on)(peripheral_handle_t handle);
    int32 (*power_off)(peripheral_handle_t handle);
    int32 (*reset)(peripheral_handle_t handle);
    int32 (*get_status)(peripheral_handle_t handle, peripheral_status_t *status);
    int32 (*get_info)(peripheral_handle_t handle, peripheral_info_t *info);
    int32 (*execute_command)(peripheral_handle_t handle, ...);
    int32 (*read_data)(peripheral_handle_t handle, ...);
    int32 (*write_data)(peripheral_handle_t handle, ...);
    int32 (*register_callback)(peripheral_handle_t handle, ...);
    int32 (*get_stats)(peripheral_handle_t handle, peripheral_stats_t *stats);
    int32 (*reset_stats)(peripheral_handle_t handle);
} peripheral_ops_t;
```

## 统一API接口

### 外设生命周期管理

```c
/* 注册外设驱动 */
int32 Peripheral_RegisterDriver(peripheral_type_t type, const peripheral_ops_t *ops);

/* 创建外设实例 */
int32 Peripheral_Create(peripheral_type_t type, void *config, peripheral_handle_t *handle);

/* 销毁外设实例 */
int32 Peripheral_Destroy(peripheral_handle_t handle);
```

### 外设控制接口

```c
/* 电源控制 */
int32 Peripheral_PowerOn(peripheral_handle_t handle);
int32 Peripheral_PowerOff(peripheral_handle_t handle);
int32 Peripheral_Reset(peripheral_handle_t handle);

/* 状态查询 */
int32 Peripheral_GetStatus(peripheral_handle_t handle, peripheral_status_t *status);
int32 Peripheral_GetInfo(peripheral_handle_t handle, peripheral_info_t *info);

/* 命令执行 */
int32 Peripheral_ExecuteCommand(peripheral_handle_t handle, ...);

/* 数据读写 */
int32 Peripheral_ReadData(peripheral_handle_t handle, ...);
int32 Peripheral_WriteData(peripheral_handle_t handle, ...);
```

## MCU外设驱动

### 支持的MCU通信接口

- **I2C**: 适用于低速传感器MCU、电源管理MCU
- **SPI**: 适用于高速数据采集MCU
- **UART**: 适用于调试接口、简单通信

### MCU配置示例

```c
mcu_config_t mcu_cfg = {
    .name = "PowerMCU",
    .interface = MCU_INTERFACE_I2C,
    .i2c = {
        .device = "/dev/i2c-0",
        .slave_addr = 0x50,
        .speed_hz = 100000
    },
    .cmd_timeout_ms = 1000,
    .retry_count = 3,
    .enable_crc = true,
    .heartbeat_interval_ms = 5000
};

peripheral_handle_t mcu_handle;
MCU_Init(&mcu_cfg, &mcu_handle);
```

### MCU命令类型

```c
typedef enum
{
    MCU_CMD_GET_VERSION = 0x01,       /* 获取版本 */
    MCU_CMD_GET_STATUS = 0x02,        /* 获取状态 */
    MCU_CMD_RESET = 0x03,             /* 复位 */
    MCU_CMD_POWER_ON = 0x10,          /* 上电 */
    MCU_CMD_POWER_OFF = 0x11,         /* 下电 */
    MCU_CMD_READ_SENSOR = 0x20,       /* 读取传感器 */
    MCU_CMD_WRITE_GPIO = 0x30,        /* 写GPIO */
    MCU_CMD_READ_GPIO = 0x31,         /* 读GPIO */
    MCU_CMD_FIRMWARE_UPDATE = 0xF0,   /* 固件升级 */
    MCU_CMD_CUSTOM = 0xFF             /* 自定义命令 */
} mcu_cmd_type_t;
```

## 使用示例

### 1. 注册MCU驱动

```c
/* 在系统初始化时注册 */
MCU_RegisterDriver();
```

### 2. 创建MCU外设实例

```c
mcu_config_t config = {
    .name = "SensorMCU",
    .interface = MCU_INTERFACE_I2C,
    .i2c.device = "/dev/i2c-1",
    .i2c.slave_addr = 0x48,
    .i2c.speed_hz = 400000
};

peripheral_handle_t mcu;
if (MCU_Init(&config, &mcu) == OS_SUCCESS)
{
    /* MCU初始化成功 */
}
```

### 3. 控制MCU外设

```c
/* 上电 */
Peripheral_PowerOn(mcu);

/* 获取状态 */
peripheral_status_t status;
Peripheral_GetStatus(mcu, &status);

/* 读取传感器 */
mcu_cmd_frame_t cmd = {
    .cmd = MCU_CMD_READ_SENSOR,
    .len = 1,
    .data = {0x01}  /* 传感器ID */
};
mcu_resp_frame_t resp;
MCU_SendCommand(mcu, &cmd, &resp, 1000);

/* 下电 */
Peripheral_PowerOff(mcu);
```

### 4. 统一的外设管理

```c
/* 所有外设使用统一接口 */
peripheral_handle_t devices[] = {satellite, bmc, linux_payload, mcu};

for (int i = 0; i < 4; i++)
{
    peripheral_info_t info;
    Peripheral_GetInfo(devices[i], &info);
    
    peripheral_status_t status;
    Peripheral_GetStatus(devices[i], &status);
    
    printf("Device: %s, Type: %d, State: %d\n", 
           info.name, info.type, status.state);
}
```

## 架构优势

### 1. 统一抽象
- 所有外设使用统一的接口，简化上层应用开发
- 新增外设类型只需实现标准接口

### 2. 模块化设计
- 每种外设驱动独立开发、测试、维护
- 驱动可插拔，支持动态注册

### 3. 易于扩展
- 新增MCU外设支持（I2C/SPI/UART）
- 未来可扩展：传感器、执行器、显示屏等

### 4. 多人协作友好
- 按外设类型划分目录，职责清晰
- 不同团队可独立开发不同外设驱动

### 5. 多仓库拆分就绪
- 外设驱动可独立成库
- 核心框架与具体驱动解耦

## 后续工作

### 短期（已完成）
- ✅ 设计外设抽象接口
- ✅ 实现外设管理核心
- ✅ 实现MCU外设驱动框架
- ✅ 编译验证通过

### 中期（TODO）
- [ ] 将现有卫星服务迁移到外设框架
- [ ] 将现有BMC服务迁移到外设框架
- [ ] 将现有Linux载荷服务迁移到外设框架
- [ ] 实现I2C/SPI HAL层接口
- [ ] 完善MCU驱动实现（I2C/SPI/UART）

### 长期（规划）
- [ ] 添加传感器外设支持
- [ ] 添加执行器外设支持
- [ ] 实现外设热插拔
- [ ] 实现外设固件升级框架
- [ ] 外设驱动独立成库

## 兼容性说明

**保留传统接口**：现有的`pdl_satellite.h`、`pdl_payload_bmc.h`等接口保持不变，确保现有代码继续工作。新代码推荐使用外设框架接口。

**渐进式迁移**：可以逐步将传统服务迁移到外设框架，无需一次性重写。

## 编译验证

```bash
./build.sh
```

编译成功，生成：
- can_gateway (51KB)
- protocol_converter (56KB)
- unit-test (466KB)

---

**重构完成时间**: 2026-04-24
**架构设计**: 外设抽象层 + 统一管理框架
**新增功能**: MCU外设驱动支持（I2C/SPI/UART）
