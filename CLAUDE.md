# CSPD-BSP 项目说明

## 项目概述

**CSPD-BSP** (Compute and Storage Payload Board Support Package) 是为卫星算存载荷设计的板级支持包，作为卫星平台与算存载荷之间的通信桥接和管理中间层。

**系统架构**：
```
卫星平台 <--CAN--> 转接板(CSPD-BSP) <--Ethernet/UART--> 算存载荷
```

## 核心功能

- **CAN通信**：与卫星平台进行CAN总线通信
- **协议转换**：卫星命令 ↔ IPMI/Redfish命令
- **通信冗余**：以太网主通道 + UART备份通道
- **状态监控**：定期查询载荷状态并上报

## 代码结构（4层架构 + 模块化配置）

```
cspd-bsp/
├── osal/                    # 操作系统抽象层 (OSAL)
│   ├── include/             # 接口定义
│   │   └── config/          # OSAL配置（模块独立）
│   │       ├── task_config.h    # 任务配置（栈大小、优先级）
│   │       ├── queue_config.h   # 队列配置
│   │       └── log_config.h     # 日志配置
│   └── src/linux/           # Linux实现
│       ├── os_task.c        # 任务管理（pthread）
│       ├── os_queue.c       # 消息队列
│       ├── os_mutex.c       # 互斥锁（带死锁检测）
│       ├── os_log.c         # 日志系统（带轮转）
│       └── ...
├── hal/                     # 硬件抽象层 (HAL)
│   ├── include/             # 接口定义
│   │   └── config/          # HAL配置（模块独立）
│   │       ├── can_types.h      # CAN帧类型定义
│   │       ├── can_config.h     # CAN硬件配置
│   │       ├── ethernet_config.h # 以太网配置
│   │       └── uart_config.h    # 串口配置
│   └── src/linux/           # Linux驱动
│       ├── hal_can_linux.c  # CAN驱动（SocketCAN）
│       ├── hal_serial_linux.c # 串口驱动
│       └── hal_network_linux.c # 网络驱动
├── pdl/                     # 外设驱动层 (Peripheral Driver Layer)
│   ├── include/             # 接口定义
│   │   ├── peripheral_device.h  # 统一外设接口
│   │   ├── peripherals/         # 外设驱动接口
│   │   ├── pdl_satellite.h      # 卫星平台接口
│   │   ├── pdl_payload_bmc.h    # BMC载荷接口
│   │   ├── pdl_payload_linux.h  # Linux载荷接口
│   │   └── pdl_power.h          # 电源管理接口
│   └── src/
│       ├── peripherals/         # 外设驱动实现
│       │   ├── core/            # 外设管理核心
│       │   ├── mcu/             # MCU外设驱动
│       │   ├── satellite/       # 卫星外设适配器
│       │   ├── payload_bmc/     # BMC外设适配器
│       │   └── payload_linux/   # Linux载荷适配器
│       ├── pdl_core/            # 外设管理核心
│       ├── pdl_mcu/             # MCU外设驱动
│       ├── pdl_satellite/       # 卫星平台服务
│       ├── pdl_payload_bmc/     # BMC载荷服务
│       ├── pdl_payload_os/      # Linux载荷服务
│       └── pdl_power/           # 电源管理服务
├── apps/                    # 应用层
│   ├── can_gateway/         # CAN网关应用
│   │   ├── include/config/  # CAN网关配置（模块独立）
│   │   │   ├── app_config.h     # 应用配置（版本号等）
│   │   │   └── can_protocol.h   # CAN协议定义
│   │   └── src/             # 源代码
│   │       ├── can_gateway.c    # CAN消息处理
│   │       └── main.c           # 主程序入口
│   └── protocol_converter/  # 协议转换应用
│       ├── include/config/  # 协议转换配置（模块独立）
│       │   └── app_config.h     # 应用配置（超时、重试等）
│       └── src/             # 源代码
│           ├── protocol_converter.c # 协议转换逻辑
│           ├── payload_service.c    # 载荷服务封装
│           └── main.c           # 主程序入口
├── output/                  # 构建输出目录
│   ├── build.log            # 构建日志
│   ├── build/               # 编译中间文件
│   └── target/              # 最终产物
│       ├── bin/             # 可执行文件
│       └── lib/             # 静态库
└── tests/                   # 测试框架
    ├── core/                # 测试核心文件
    │   ├── unittest_entry.c     # 统一测试入口
    │   ├── unittest_runner.c    # 测试运行器
    │   ├── unittest_runner.h    # 测试运行器头文件
    │   └── unittest_framework.h # 测试框架
    ├── osal/                # OSAL层测试
    ├── hal/                 # HAL层测试
    ├── service/             # Service层测试
    └── apps/                # Apps层测试
```

## 关键设计特点

### 1. 模块化配置（重要）
- **配置下沉**：每个模块的配置文件放在模块内部的 `include/config/` 目录
- **模块独立**：各模块只依赖自己的配置，便于多人协作和多仓库拆分
- **依赖隔离**：
  - OSAL层：只依赖自己的 `osal/include/config/`
  - HAL层：依赖 `hal/include/config/` 和 OSAL接口
  - PDL层：依赖 `pdl/include/config/`、HAL接口、OSAL接口
  - Apps层：依赖 `apps/*/include/config/`、PDL接口、HAL接口、OSAL接口
- **配置文件示例**：
  - `osal/include/config/task_config.h` - 任务栈大小、优先级定义
  - `hal/include/config/can_config.h` - CAN接口、波特率配置
  - `apps/can_gateway/include/config/can_protocol.h` - CAN协议定义

### 2. 分层隔离
- **OSAL层**：封装操作系统API，支持跨平台移植
- **HAL层**：封装硬件驱动，隔离硬件差异
- **PDL层**：外设驱动层，统一管理卫星/载荷/MCU等外设
- **Apps层**：应用逻辑，通过PDL层访问底层

### 2. 任务管理（优雅关闭）
- 文件：`osal/src/linux/os_task.c`
- 配置：`osal/include/config/task_config.h`
- 特点：
  - 使用shutdown标志而非pthread_cancel（避免死锁）
  - 任务通过`OS_TaskShouldShutdown()`检查是否需要退出
  - 使用`pthread_timedjoin_np`等待任务退出（5秒超时）
  - 超时后使用`pthread_detach`而非强制取消

### 4. 日志系统
- 文件：`osal/src/linux/os_log.c`
- 配置：`osal/include/config/log_config.h`
- 功能：
  - 支持多级别日志（DEBUG/INFO/WARN/ERROR/FATAL）
  - 日志轮转（按大小，保留N个文件）
  - 线程安全
- 接口：
  - `OS_printf()` - 简单打印
  - `LOG_INFO(module, ...)` - 带模块名的日志宏
  - `LOG_ERROR(module, ...)` - 错误日志

### 5. 通信冗余
- 主通道：以太网（IPMI over LAN）
- 备份通道：UART（IPMI over Serial）
- 配置：`hal/include/config/ethernet_config.h` 和 `hal/include/config/uart_config.h`
- 自动切换：连续5次失败后切换到备份通道
- 定期恢复：尝试恢复主通道

## 构建系统

### 编译
```bash
./build.sh           # Release模式
./build.sh -d        # Debug模式
./build.sh -c        # 清理后编译
```

### 输出
```
output/
├── build/           # 编译中间文件
└── target/
    ├── bin/         # 可执行文件
    │   ├── can_gateway
    │   ├── protocol_converter
    │   └── unit-test
    └── lib/         # 静态库
        ├── libosal.a
        ├── libhal.a
        └── libpdl.a
```

## 测试系统

### 统一测试入口
```bash
./output/target/bin/unit-test -i    # 交互式菜单
./output/target/bin/unit-test -a    # 运行所有测试
./output/target/bin/unit-test -L OSAL  # 运行OSAL层测试
./output/target/bin/unit-test -m test_os_task  # 运行指定模块
```

### 测试覆盖
- **OSAL层**：6个模块（task, queue, mutex, file, network, signal）
- **HAL层**：1个模块（CAN驱动）
- **PDL层**：1个模块（payload pdl）
- **Apps层**：2个模块（CAN gateway, protocol converter）

### 交互式菜单特点
- 三级选择：层级 → 模块 → 测试用例
- 使用序号选择，无需输入完整名称
- 支持单个测试、模块测试、层级测试

## 编码规范

### 日志接口
- **禁止**直接使用`printf`/`fprintf`
- **使用**OSAL日志接口：
  - `OS_printf()` - 简单输出
  - `LOG_INFO("MODULE", "message")` - 信息日志
  - `LOG_ERROR("MODULE", "message")` - 错误日志

### 错误处理
- 所有函数返回`int32`状态码
- 成功返回`OS_SUCCESS`
- 失败返回`OS_ERROR`或具体错误码
- 关键操作必须检查返回值

### 内存管理
- 关键路径（中断、DMA）使用预分配内存池
- 避免在中断上下文中使用`malloc`
- 所有`malloc`必须检查返回值

### 任务编写规范
```c
static void my_task_entry(void *arg)
{
    osal_id_t task_id = OS_TaskGetId();
    
    while (!OS_TaskShouldShutdown())  // 检查退出标志
    {
        // 执行任务逻辑
        do_work();
        
        // 延时
        OS_TaskDelay(100);
    }
    
    // 清理资源
    cleanup();
}
```

## 关键配置

### 模块配置文件位置
- **OSAL配置**：`osal/include/config/`
  - `task_config.h` - 任务栈大小、优先级定义
  - `queue_config.h` - 队列大小配置
  - `log_config.h` - 日志级别、文件路径
- **HAL配置**：`hal/include/config/`
  - `can_types.h` - CAN帧类型定义
  - `can_config.h` - CAN接口、波特率
  - `ethernet_config.h` - 以太网IP、端口
  - `uart_config.h` - 串口设备、波特率
- **Apps配置**：`apps/*/include/config/`
  - `apps/can_gateway/include/config/app_config.h` - 系统版本号
  - `apps/can_gateway/include/config/can_protocol.h` - CAN协议定义
  - `apps/protocol_converter/include/config/app_config.h` - 超时、重试配置

### CAN配置
- 文件：`hal/include/config/can_config.h`
- 接口：`can0`
- 波特率：500Kbps
- 协议：自定义8字节协议（见`apps/can_gateway/include/config/can_protocol.h`）

### 载荷通信
- 主通道：以太网（192.168.1.100:623）
- 备份通道：UART（/dev/ttyS0, 115200）
- 配置文件：`hal/include/config/ethernet_config.h` 和 `hal/include/config/uart_config.h`
- 协议：IPMI/Redfish

### 看门狗配置
- 文件：`pdl/include/config/watchdog_config.h`
- 检查间隔：100ms
- 任务超时：500ms
- 最大重启次数：3次
- 超时后进入安全模式

## 常见问题

### 1. 编译警告处理
- 项目使用`-Werror`，所有警告视为错误
- 必须修复所有警告才能编译通过

### 2. 测试失败（硬件相关）
- CAN测试失败：需要CAN设备（can0）
- 串口测试失败：需要串口设备（/dev/ttyS0）
- 这些失败是正常的，不影响代码逻辑

### 3. 任务优雅退出
- 任务循环必须检查`OS_TaskShouldShutdown()`
- 不要使用无限循环`while(1)`
- 退出前清理资源

## 最近重构（2026-04-24）

### Service层重命名为PDL层（最新）
- **重命名原因**：原名称`service`容易与业务服务混淆，新名称`pdl`（Peripheral Driver Layer，外设驱动层）更准确
- **架构理念**：管理板为核心，卫星/载荷/BMC/MCU统一抽象为外设
- **命名变更**：
  - 目录：`service/` → `pdl/`
  - 头文件：`service_*.h` → `pdl_*.h`
  - 源文件：`service_*.c` → `pdl_*.c`
  - 函数前缀：`SatelliteService_*` → `SatellitePDL_*`
  - 宏定义：`SERVICE_*` → `PDL_*`
- **外设框架**：新增统一外设接口（`peripheral_device.h`），支持MCU/卫星/BMC/Linux载荷
- **适配器模式**：保留传统接口100%兼容，通过适配器包装到外设框架
- **详细文档**：见 `docs/PDL_RENAME.md`、`docs/SERVICE_REFACTOR.md`、`docs/SERVICE_MIGRATION.md`

### 目录结构标准化
- **inc → include**：统一使用 `include/` 目录存放头文件
- **linux → src/linux**：统一使用 `src/linux/` 目录存放Linux平台实现
- **config 移入 include**：配置文件统一放在 `include/config/` 目录
- **apps 源码分离**：应用层源码移至 `src/` 目录，头文件在 `include/`
- **标准化结构**：
  ```
  module/
  ├── include/           # 头文件
  │   └── config/        # 配置文件
  └── src/linux/         # Linux平台实现
  ```
- **引用规范**：配置文件使用 `#include "config/xxx_config.h"` 引用

### 模块化配置重构
- **配置下沉**：将全局 `config/` 目录拆分，配置文件下沉到各模块内部
- **目录结构**：
  - `osal/include/config/` - OSAL层配置（task_config.h, queue_config.h, log_config.h）
  - `hal/include/config/` - HAL层配置（can_types.h, can_config.h, ethernet_config.h, uart_config.h）
  - `pdl/include/config/` - PDL层配置（watchdog_config.h）
  - `apps/can_gateway/include/config/` - CAN网关配置（app_config.h, can_protocol.h）
  - `apps/protocol_converter/include/config/` - 协议转换配置（app_config.h）
- **依赖隔离**：各模块只依赖自己的配置，便于多人协作和多仓库拆分
- **构建日志**：`build.log` 生成到 `output/` 目录，所有构建产物统一管理
- **删除**：移除全局 `config/` 目录和 `system_config.h`

### 测试框架重构
- 重命名：`test_framework.h` → `unittest_framework.h`
- 重命名：`test_runner.c/h` → `unittest_runner.c/h`
- 重命名：`test_main_unified.c` → `unittest_entry.c`
- 统一日志：所有`printf`替换为`OS_printf`
- 目录重组：测试核心文件移至`tests/core/`目录

### 清理遗留代码
- 删除：`tests/osal/test_main.c`（已被统一入口替代）
- 删除：`tests/apps/CMakeLists.txt`（独立测试配置）
- 删除：`tests/pdl/CMakeLists.txt`（独立测试配置）
- 删除：`examples/`目录（示例代码）
- 删除：`osal/freertos/`目录（FreeRTOS示例）
- 删除：`config/`目录（全局配置，已下沉到各模块）
- 删除：看门狗和持久化队列模块（暂不需要）

## 开发建议

1. **遵循分层架构**：不要跨层直接调用
2. **模块独立性**：各模块只依赖自己的 `include/config/` 目录，不要引用其他模块的配置
3. **使用OSAL接口**：不要直接使用pthread/socket等系统API
4. **优雅退出**：任务循环检查`OS_TaskShouldShutdown()`
5. **错误处理**：所有返回值必须检查
6. **日志规范**：使用`LOG_*`宏，不要用`printf`
7. **测试驱动**：新功能必须编写单元测试
8. **配置管理**：修改配置时只修改对应模块的 `include/config/` 目录

## 性能指标

- CAN消息延迟：< 10ms
- 命令处理时间：< 100ms
- 内存占用：< 128MB
- CPU占用（空闲）：< 5%
