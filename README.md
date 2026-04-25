# PMC-BSP

载荷管理控制器板级支持包 (Payload Management Controller Board Support Package)

## 系统概述

PMC-BSP 是专为卫星算存载荷设计的板级支持包，作为卫星平台与算存载荷之间的通信桥接和管理中间层。

**系统架构**：
```
卫星平台 <--CAN--> 转接板(PMC-BSP) <--Ethernet/UART--> 算存载荷
```

**代码规模**：
- 总代码量：约18,000行
- 生产代码：约14,000行
- 测试代码：约4,000行
- 代码文件：97个（.c/.h）

## 核心功能

### OSAL层（操作系统抽象层）
**设计参考**：NASA cFS OSAL (Core Flight System Operating System Abstraction Layer)

- **任务管理**：基于pthread的任务创建/删除，支持优雅退出机制（避免pthread_cancel死锁）
- **消息队列**：环形缓冲区实现，支持阻塞/非阻塞/超时模式，引用计数保护生命周期
- **互斥锁**：支持超时获取和死锁检测（5秒阈值），避免嵌套锁
- **日志系统**：5级日志（DEBUG/INFO/WARN/ERROR/FATAL），自动轮转（10MB/5个备份），彩色终端输出
- **网络抽象**：Socket封装（TCP/UDP），支持多路复用和超时控制
- **文件操作**：POSIX文件I/O封装，支持非阻塞和I/O控制
- **时间服务**：系统时钟、滴答计数、毫秒级延时
- **信号处理**：Unix信号注册、屏蔽/解除屏蔽
- **内存管理**：堆内存监控、阈值告警（默认80%）
- **错误处理**：37个标准错误码，统一错误描述

**跨平台支持**：
- 当前实现：Linux (pthread, SocketCAN, termios)
- 预留接口：FreeRTOS, VxWorks, RTEMS, Zephyr

### HAL层（硬件抽象层）
**职责**：封装所有硬件相关代码，提供统一的硬件接口

- **CAN驱动**：基于SocketCAN实现，支持标准/扩展帧，CAN ID过滤，统计信息（tx/rx/err计数）
- **串口驱动**：基于termios实现，支持9600-4000000波特率，原始模式（无终端处理），灵活超时控制

**扩展计划**：
- GPIO驱动、SPI驱动、I2C驱动、定时器驱动
- 多平台支持（TI AM62x, STM32, Xilinx Zynq等）

### PDL层（外设驱动层 - Peripheral Driver Layer）
**设计理念**：管理板为核心，卫星/载荷/BMC/MCU统一抽象为外设

- **卫星平台服务**：CAN总线通信，命令回调机制，心跳管理（5秒周期），统计监控
- **BMC载荷服务**：IPMI/Redfish协议，电源控制（上电/下电/重启），状态查询，传感器读取，双通道冗余（以太网+UART）
- **MCU外设服务**：多接口支持（CAN/UART/I2C/SPI），版本查询，状态监控，寄存器读写，固件升级

**外设框架**：
- 统一外设接口（`peripheral_device.h`）
- 支持MCU/卫星/BMC/Linux载荷
- 适配器模式保留传统接口100%兼容

### XConfig层（硬件配置库）
**设计理念**：类似Linux设备树，以外设为单位管理硬件配置，实现配置与代码分离

- **两层配置架构**：
  - 硬件配置层：定义外设硬件接口（MCU/BMC/传感器等如何连接）
  - APP配置层：定义APP使用哪些外设（逻辑名称映射到物理设备）
- **外设配置管理**：以外设为单位（MCU/BMC/传感器等）管理硬件配置
- **接口内嵌**：每个外设配置内嵌其通信接口配置（CAN/UART/I2C/SPI/Ethernet等）
- **APP外设映射**：APP通过外设类型+编号映射到具体硬件，支持可选外设和冗余配置
- **多平台支持**：支持按平台/产品/版本组织配置（嵌套目录结构）
  - TI AM6254平台：H200-100P（100P算力）、H200-32P（32P算力）
  - 演示平台：演示项目（2P算力，用于模拟测试）
- **统一命名规范**：XCONFIG_* (通用) / XCONFIG_HW_* (硬件) / XCONFIG_BMC_* (BMC协议) / XCONFIG_APP_* (APP)
- **配置选择**：支持环境变量/编译选项/默认配置三种选择方式

**使用场景**：仅供PDL层使用，实现硬件配置的集中管理和多平台支持

### Apps层（应用层）
- **CAN网关**：三任务并发架构（RX/TX/Heartbeat），消息队列缓冲（深度10），原子统计，CAN协议封装（8字节固定帧）
- **协议转换器**：CAN命令→IPMI命令映射，智能响应解析（温度/电压/状态），通道故障自动切换（连续5次失败），线程安全保护

## 架构设计

### 5层分层架构

```
┌─────────────────────────────────────────────────────────┐
│  Apps层 (应用层)                                         │
│  ├── can_gateway        - CAN网关应用                    │
│  └── protocol_converter - 协议转换应用                   │
├─────────────────────────────────────────────────────────┤
│  PDL层 (外设驱动层 - Peripheral Driver Layer)            │
│  ├── pdl_satellite      - 卫星平台服务                   │
│  ├── pdl_bmc            - BMC载荷服务 (IPMI/Redfish)     │
│  └── pdl_mcu            - MCU外设服务                    │
├─────────────────────────────────────────────────────────┤
│  XConfig层 (硬件配置库)                                  │
│  └── 以外设为单位的硬件配置管理 (类似设备树)             │
├─────────────────────────────────────────────────────────┤
│  HAL层 (硬件抽象层)                                      │
│  ├── hal_can            - CAN驱动 (SocketCAN)           │
│  └── hal_serial         - 串口驱动 (termios)            │
├─────────────────────────────────────────────────────────┤
│  OSAL层 (操作系统抽象层)                                 │
│  ├── 任务管理 (优雅退出)  ├── 消息队列 (引用计数)        │
│  ├── 互斥锁 (死锁检测)    ├── 日志系统 (轮转机制)        │
│  ├── 网络抽象             └── 文件操作                   │
└─────────────────────────────────────────────────────────┘
```

### 模块化配置

每个模块的配置文件独立管理，位于模块内部的 `include/config/` 目录：

- **OSAL配置**：`osal/include/config/`
  - `task_config.h` - 任务栈大小、优先级
  - `queue_config.h` - 队列深度配置
  - `log_config.h` - 日志级别、文件路径
- **HAL配置**：`hal/include/config/`
  - `can_types.h` - CAN帧类型定义
  - `can_config.h` - CAN接口、波特率
  - `uart_config.h` - 串口设备、波特率
- **XConfig配置**：`xconfig/platform/<vendor>/<chip>/<product>/`
  - 以外设为单位的硬件配置（MCU/BMC/传感器等）
  - 支持多平台、多产品、多版本配置管理
  - 嵌套目录结构：`platform/ti/am625/H200_100P/*.c`
  - 详见 [xconfig/README.md](xconfig/README.md)
- **Apps配置**：`apps/*/include/config/`
  - `can_gateway/include/config/can_protocol.h` - CAN协议定义
  - `protocol_converter/include/config/app_config.h` - 超时、重试配置

## 快速开始

### 1. 系统要求

- Linux系统 (推荐Ubuntu 20.04+)
- CMake 3.10+
- GCC编译器
- CAN工具 (can-utils)
- Root权限 (访问CAN设备)

### 2. 安装依赖

```bash
sudo apt-get update
sudo apt-get install build-essential cmake can-utils
```

### 3. 编译

```bash
# 使用自动构建脚本（推荐）
./build.sh           # Release模式
./build.sh -d        # Debug模式
./build.sh -c        # 清理后编译
```

编译产物位于 `output/target/` 目录：
```
output/target/
├── bin/
│   ├── can_gateway          # CAN网关应用
│   ├── protocol_converter   # 协议转换应用
│   └── unit-test            # 统一测试程序
└── lib/
    ├── libosal.a            # OSAL静态库
    ├── libhal.a             # HAL静态库
    └── libpdl.a             # PDL静态库
```

### 4. 配置CAN接口

```bash
# 设置CAN波特率为500Kbps
sudo ip link set can0 type can bitrate 500000

# 启动CAN接口
sudo ip link set can0 up

# 验证CAN接口
ip -details link show can0
```

### 5. 运行应用

```bash
# 运行CAN网关
sudo ./output/target/bin/can_gateway

# 运行协议转换器
sudo ./output/target/bin/protocol_converter
```

### 6. 运行测试

```bash
# 交互式菜单（推荐）
./output/target/bin/unit-test -i

# 运行所有测试
./output/target/bin/unit-test -a

# 运行指定层测试
./output/target/bin/unit-test -L OSAL

# 运行指定模块测试
./output/target/bin/unit-test -m test_osal_task

# 列出所有测试
./output/target/bin/unit-test -l
```

## 目录结构

```
pmc-bsp/
├── osal/                    # 操作系统抽象层 (OSAL)
│   ├── include/             # 接口定义
│   │   └── config/          # OSAL配置（模块独立）
│   └── src/linux/           # Linux实现
│       ├── osal_task.c      # 任务管理（优雅退出）
│       ├── osal_queue.c     # 消息队列（引用计数）
│       ├── osal_mutex.c     # 互斥锁（死锁检测）
│       ├── osal_log.c       # 日志系统（轮转机制）
│       └── ...
├── hal/                     # 硬件抽象层 (HAL)
│   ├── include/             # 接口定义
│   │   └── config/          # HAL配置（模块独立）
│   └── src/linux/           # Linux驱动
│       ├── hal_can.c        # CAN驱动（SocketCAN）
│       └── hal_serial.c     # 串口驱动（termios）
├── pdl/                     # 外设驱动层 (PDL)
│   ├── include/             # 接口定义
│   │   ├── pdl_satellite.h  # 卫星平台接口
│   │   ├── pdl_bmc.h        # BMC载荷接口
│   │   └── pdl_mcu.h        # MCU外设接口
│   └── src/
│       ├── pdl_satellite/   # 卫星平台服务
│       ├── pdl_bmc/         # BMC载荷服务（IPMI/Redfish）
│       └── pdl_mcu/         # MCU外设服务
├── xconfig/                     # 硬件配置库（XConfig）
│   ├── include/                 # 头文件
│   │   ├── xconfig.h           # 总头文件
│   │   ├── xconfig_common.h    # 通用类型（GPIO、电源域）
│   │   ├── xconfig_hardware_interface.h  # 硬件接口定义
│   │   ├── xconfig_mcu.h       # MCU外设配置
│   │   ├── xconfig_bmc.h       # BMC外设配置（IPMI/Redfish协议）
│   │   ├── xconfig_satellite.h # 卫星平台接口配置
│   │   ├── xconfig_sensor.h    # 传感器外设配置
│   │   ├── xconfig_storage.h   # 存储设备配置
│   │   ├── xconfig_app.h       # APP配置
│   │   └── xconfig_board.h     # 板级配置
│   ├── src/                     # 源代码
│   │   ├── xconfig_api.c       # API实现
│   │   └── xconfig_register.c  # 配置注册
│   └── platform/                # 平台配置（嵌套目录结构）
│       ├── ti/am6254/           # TI AM6254平台
│       │   ├── H200_100P/      # H200-100P产品（100P算力）
│       │   │   ├── h200_100p_base.c  # 基础配置
│       │   │   ├── h200_100p_v1.c    # V1.0配置
│       │   │   └── h200_100p_v2.c    # V2.0配置
│       │   └── H200_32P/       # H200-32P产品（32P算力）
│       │       ├── h200_32p_base.c   # 基础配置
│       │       ├── h200_32p_v1.c     # V1.0配置
│       │       └── h200_32p_v2.c     # V2.0配置
│       └── vendor_demo/         # 演示厂商
│           └── platform_demo/   # 演示平台
│               └── project_demo/   # 演示项目（2P算力，演示用）
│                   ├── product_demo_base.c       # 基础配置
│                   ├── product_demo_v1.c         # V1.0配置
│                   └── product_demo_v2.c         # V2.0配置
├── apps/                    # 应用层
│   ├── can_gateway/         # CAN网关应用
│   │   ├── include/config/  # CAN网关配置
│   │   └── src/             # 源代码
│   └── protocol_converter/  # 协议转换应用
│       ├── include/config/  # 协议转换配置
│       └── src/             # 源代码
├── output/                  # 构建输出目录
│   ├── build.log            # 构建日志
│   ├── build/               # 编译中间文件
│   └── target/              # 最终产物
└── docs/                    # 文档目录
    ├── ARCHITECTURE.md      # 架构设计文档
    ├── MODULE_ANALYSIS.md   # 模块源码分析
    ├── TESTING.md           # 测试框架文档
    └── ...
```

## 支持的平台和产品

### TI AM6254平台

**H200-100P系列**（100P算力）：
- 基础配置：`xconfig_h200_100p_base`
- V1.0：`xconfig_h200_100p_v1`（增加冗余MCU和IMU传感器）
- V2.0：`xconfig_h200_100p_v2`（升级1553B、GPS、NVMe）

**H200-32P系列**（32P算力）：
- 基础配置：`xconfig_h200_32p_base`
- V1.0：`xconfig_h200_32p_v1`（增加冗余MCU和IMU传感器）
- V2.0：`xconfig_h200_32p_v2`（升级1553B、GPS、NVMe）

### 演示平台

**演示项目系列**（2P算力，演示/模拟测试用）：
- 基础配置：`xconfig_demo_base`
- V1.0：`xconfig_demo_v1`（增加冗余MCU和IMU传感器）
- V2.0：`xconfig_demo_v2`（升级1553B、GPS、NVMe）

> **注意**：产品名称中的"P"表示算力（Computing Power），如100P表示100 PFLOPS算力，而非PCIe通道数。

## CAN协议

### 消息格式 (8字节)

| Byte | 说明 |
|------|------|
| 0 | 消息类型 |
| 1 | 命令类型/状态码 |
| 2-3 | 序列号（大端序） |
| 4-7 | 数据/参数（大端序） |

### CAN ID定义

- `0x100`：卫星平台 → 管理板
- `0x200`：管理板 → 卫星平台

### 消息类型

- `0x01`：命令请求 (CMD_REQ)
- `0x02`：命令响应 (CMD_RESP)
- `0x03`：状态查询 (STATUS_QUERY)
- `0x04`：状态上报 (STATUS_REPORT)
- `0x05`：心跳 (HEARTBEAT)
- `0x06`：错误报告 (ERROR)

### 命令类型

**电源控制**：
- `0x10`：载荷上电 (POWER_ON)
- `0x11`：载荷下电 (POWER_OFF)
- `0x12`：载荷重启 (RESET)

**状态查询**：
- `0x20`：查询状态 (QUERY_STATUS)
- `0x21`：查询温度 (QUERY_TEMP)
- `0x22`：查询电压 (QUERY_VOLTAGE)

**配置管理**：
- `0x30`：设置配置 (SET_CONFIG)
- `0x31`：获取配置 (GET_CONFIG)

## 关键设计特点

### 1. 航天级可靠性设计

**优雅退出机制（OSAL任务）**
- 使用 `shutdown_requested` 标志而非 `pthread_cancel`（避免死锁）
- 任务循环检查 `OSAL_TaskShouldShutdown()`
- 使用 `pthread_timedjoin_np` 等待5秒超时
- 超时后使用 `pthread_detach` 而非强制取消

**引用计数保护（OSAL队列）**
- 使用 `atomic_int` 引用计数防止 use-after-free
- 删除时先标记 `valid=false`，引用计数降为0时才释放
- 双重保护机制：`valid` 标志 + `ref_count`
- 防止队列删除时其他线程正在使用

**死锁检测（OSAL互斥锁）**
- 使用 `pthread_mutex_timedlock()` 实现超时（默认5秒）
- 超时后触发回调函数，记录锁名称和等待时间
- 避免嵌套锁：查找锁和获取锁操作分离

### 2. 通信冗余与故障恢复

**双通道架构（PDL BMC）**
- 主通道：以太网（IPMI over LAN，端口623）
- 备份通道：UART（IPMI over Serial，115200bps）
- 自动切换：连续5次失败后切换到备份通道
- 故障恢复：成功后重置失败计数，允许手动切换回主通道

**连接状态机**
```
DISCONNECTED → CONNECTING → CONNECTED → ERROR
     ↑              ↓            ↓         ↓
     └──────────────┴────────────┴─────────┘
```

### 3. 协议封装与转换

**CAN协议（8字节固定帧）**
- 帧结构：`[msg_type][cmd_type][seq_num(2B)][data(4B)]`
- CAN ID：0x100（卫星→管理板）、0x200（管理板→卫星）
- 消息类型：CMD_REQ、CMD_RESP、STATUS_QUERY、STATUS_REPORT、HEARTBEAT、ERROR
- 序列号管理：互斥锁保护，防止竞态条件

**IPMI协议封装**
- 命令映射：CAN命令 → IPMI命令（Chassis Control、Get Status、Sensor Reading）
- 智能解析：字符串匹配提取温度（℃）、电压（mV）、电源状态
- 错误分类：超时、通信错误、无效命令、载荷离线

**MCU协议（串口）**
- 帧格式：`[0xAA][0x55][cmd][len][data...][crc16_h][crc16_l]`
- CRC校验：MODBUS CRC16（多项式0xA001）
- 防御性编程：帧头校验、CRC校验、长度检查

### 4. 日志与监控

**日志轮转（OSAL日志）**
- 按大小轮转（默认10MB）
- 保留N个备份（默认5个）
- 轮转逻辑：`log → log.1 → log.2 → ... → log.5`（删除最旧）
- 双输出：彩色终端 + 纯文本文件

**统计监控**
- CAN网关：原子计数器（tx_count、rx_count、err_count）
- 协议转换：命令计数、成功/失败/超时次数
- BMC服务：通道切换次数、命令成功率
- 每30秒自动打印统计信息

### 5. 线程安全设计

**原子操作**
- 统计计数器使用 C11 `atomic_uint`
- 引用计数使用 `atomic_int`
- 无锁读取，避免性能损失

**互斥锁保护**
- 队列操作：`pthread_mutex` + `pthread_cond`
- 通道切换：互斥锁保护状态机
- 序列号递增：互斥锁防止竞态

**线程安全函数**
- 使用 `localtime_r` 替代 `localtime`
- 使用 `inet_ntop` 替代 `inet_ntoa`

### 6. 防御性编程

**边界检查**
- CAN DLC限制：最大8字节，防止越界拷贝
- 队列深度限制：≤10000，消息大小≤64KB
- 字符串截断保护：`strncpy` + 手动添加 `\0`

**超时保护**
- 所有阻塞操作带超时（避免死锁）
- CAN接收：1000ms超时
- Socket连接：5秒超时
- 任务退出：5秒超时

**错误处理**
- 所有函数返回 `int32` 状态码
- 区分错误类型：超时、中断、阻塞、参数错误
- 关键操作失败后记录详细日志

## 测试覆盖

| 层级 | 模块数 | 测试用例数 | 覆盖范围 |
|------|--------|-----------|---------|
| OSAL | 6 | 60 | 任务、队列、互斥锁、网络、文件、信号 |
| HAL | 1 | 3 | CAN驱动 |
| PDL | 1 | 2 | 卫星平台服务 |
| Apps | 2 | 5 | CAN网关、协议转换器 |
| **总计** | **10** | **70** | **完整的5层架构** |

### 测试特点

- **双模式支持**：统一测试 + 独立测试
- **真实场景模拟**：多线程、网络、超时测试
- **交互式菜单**：三级选择（层级 → 模块 → 测试用例）
- **精确超时验证**：±50ms误差，符合实时系统要求

## 性能指标

| 指标 | 目标值 |
|------|--------|
| CAN消息延迟 | < 10ms |
| 命令处理时间 | < 100ms |
| 内存占用 | < 128MB |
| CPU占用(空闲) | < 5% |

## 模拟测试

### 模拟CAN发送

```bash
# 发送上电命令（序列号=1）
cansend can0 100#0110000100000000

# 发送查询状态命令（序列号=2）
cansend can0 100#0120000200000000

# 发送心跳
cansend can0 100#0500000000000000
```

### 监控CAN消息

```bash
# 实时监控
candump can0

# 过滤特定ID
candump can0,100:7FF

# 记录到文件
candump -l can0
```

## 故障处理

### 以太网故障
- 自动检测连续5次失败
- 自动切换到UART通道
- 定期尝试恢复以太网

### 载荷离线
- 返回离线状态码
- 记录错误日志
- 定期重试连接

### CAN总线故障
- 错误计数统计
- 自动重试（1次）
- 日志记录详细错误信息

## 开发指南

### 添加新命令

1. 在 `apps/can_gateway/include/config/can_protocol.h` 添加命令类型
2. 在 `apps/protocol_converter/src/protocol_converter.c` 的 `execute_ipmi_command()` 添加处理逻辑
3. 重新编译

### 修改配置

1. 找到对应模块的 `include/config/` 目录
2. 修改配置文件（如 `can_config.h`）
3. 重新编译

### 添加新测试

1. 在 `tests/src/<layer>/` 创建测试文件
2. 使用 `TEST_MODULE_BEGIN/END` 宏注册测试模块
3. 在 `tests/src/test_entry.c` 添加模块引用
4. 重新编译

## 调试

### 查看日志

```bash
# 实时查看
tail -f /var/log/pmc-bsp.log

# 查看统计信息（程序每30秒自动打印）
grep "Statistics" /var/log/pmc-bsp.log
```

### 调试模式

在对应模块的 `include/config/log_config.h` 设置：

```c
#define LOG_LEVEL  LOG_LEVEL_DEBUG
```

### 使用GDB调试

```bash
# Debug模式编译
./build.sh -d

# 使用GDB
sudo gdb ./output/target/bin/can_gateway
(gdb) run
(gdb) bt  # 查看调用栈
```

## 常见问题

### Q: CAN接口无法启动

A: 检查CAN驱动是否加载：
```bash
lsmod | grep can
sudo modprobe can
sudo modprobe can_raw
sudo modprobe vcan
```

### Q: 权限不足

A: 需要root权限访问CAN设备：
```bash
sudo ./output/target/bin/can_gateway
```

### Q: 载荷连接失败

A: 检查网络连接和IP配置：
```bash
ping 192.168.1.100
telnet 192.168.1.100 623
```

### Q: 编译警告

A: 项目使用 `-Werror`，所有警告视为错误，必须修复所有警告才能编译通过。

### Q: 测试失败（硬件相关）

A: CAN/串口测试需要硬件设备，失败是正常的，不影响代码逻辑。

## 文档

详细文档请查看 `docs/` 目录：

- **[架构设计](docs/ARCHITECTURE.md)** - 系统架构和设计思想
- **[模块分析](docs/MODULE_ANALYSIS.md)** - 各模块源码详细分析
- **[测试框架](docs/TESTING.md)** - 测试框架使用指南
- **[PDL重构](docs/PDL_RENAME.md)** - Service层重命名为PDL层的说明
- **[迁移指南](docs/SERVICE_MIGRATION.md)** - 代码迁移指南

## 版本历史

### v1.0.0 (2026-04-24)
- 完成5层架构设计（OSAL/HAL/XConfig/PDL/Apps）
- 实现CAN网关和协议转换应用
- 完成70个测试用例覆盖
- 模块化配置重构
- Service层重命名为PDL层

## 许可证

Apache 2.0

## 联系方式

- 项目主页：https://github.com/your-org/pmc-bsp
- 问题反馈：https://github.com/your-org/pmc-bsp/issues
