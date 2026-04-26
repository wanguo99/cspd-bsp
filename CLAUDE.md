# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

**PMC-BSP** (Payload Management Controller Board Support Package) 是为卫星算存载荷设计的板级支持包，作为卫星平台与算存载荷之间的通信桥接和管理中间层。

**系统架构**：
```
卫星平台 <--CAN--> 转接板(PMC-BSP) <--Ethernet/UART--> 算存载荷
```

## 快速命令参考

### 编译
```bash
# 使用构建脚本（推荐）
./build.sh              # Release模式编译
./build.sh -d           # Debug模式编译
./build.sh -c           # 清理构建目录
./build.sh --target can_gateway  # 仅编译CAN网关

# 直接使用CMake（高级用户）
mkdir -p output/build && cd output/build
cmake ../.. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
cd ../..
```

### 测试
```bash
./output/target/bin/unit-test -i    # 交互式菜单（推荐）
./output/target/bin/unit-test -a    # 运行所有测试
./output/target/bin/unit-test -L OSAL  # 运行OSAL层测试
./output/target/bin/unit-test -m test_osal_task  # 运行指定模块
./output/target/bin/unit-test -l    # 列出所有测试

# 编译单个测试文件（快速迭代）
cd output/build
make test_osal_task  # 仅编译指定测试模块
cd ../..
```

### 运行应用
```bash
sudo ./output/target/bin/can_gateway      # CAN网关
sudo ./output/target/bin/protocol_converter  # 协议转换器
```

### CAN调试
```bash
# 配置CAN接口
sudo ip link set can0 type can bitrate 500000
sudo ip link set can0 up

# 发送测试消息
cansend can0 100#0110000100000000  # 上电命令
candump can0                        # 监控CAN消息
```

## 核心功能

- **CAN通信**：与卫星平台进行CAN总线通信
- **协议转换**：卫星命令 ↔ IPMI/Redfish命令
- **通信冗余**：以太网主通道 + UART备份通道
- **状态监控**：定期查询载荷状态并上报

## 代码结构（5层架构 + 模块化配置）

```
pmc-bsp/
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
├── xconfig/                 # 硬件配置库 (XConfig)
│   ├── include/             # 接口定义
│   │   ├── xconfig.h       # 总头文件
│   │   ├── xconfig_common.h # 通用类型（GPIO、电源域）
│   │   ├── xconfig_hardware_interface.h # 硬件接口定义
│   │   ├── xconfig_mcu.h   # MCU外设配置
│   │   ├── xconfig_bmc.h   # BMC外设配置
│   │   ├── xconfig_satellite.h # 卫星平台接口配置
│   │   ├── xconfig_sensor.h # 传感器外设配置
│   │   ├── xconfig_storage.h # 存储设备配置
│   │   ├── xconfig_app.h   # APP配置
│   │   └── xconfig_board.h # 板级配置
│   ├── src/                 # 源代码
│   │   ├── xconfig_api.c   # API实现
│   │   └── xconfig_register.c # 配置注册
│   └── platform/            # 平台配置（嵌套目录结构）
│       ├── ti/am6254/        # TI AM6254平台
│       │   ├── H200_100P/  # H200-100P产品（100P算力）
│       │   │   ├── h200_100p_base.c
│       │   │   ├── h200_100p_v1.c
│       │   │   └── h200_100p_v2.c
│       │   └── H200_32P/   # H200-32P产品（32P算力）
│       │       ├── h200_32p_base.c
│       │       ├── h200_32p_v1.c
│       │       └── h200_32p_v2.c
│       └── vendor_demo/     # 演示厂商
│           └── platform_demo/   # 演示平台
│               └── project_demo/   # 演示项目（2P算力，演示用）
│                   ├── product_demo_base.c
│                   ├── product_demo_v1.c
│                   └── product_demo_v2.c
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

### 2. 分层隔离与系统调用封装（关键）
- **OSAL层**：封装操作系统API，支持跨平台移植
  - **唯一允许直接调用系统调用的层**
  - 提供两类接口：
    1. **高层业务接口**：如 `OSAL_TaskCreate()`, `OSAL_SocketOpen()` - 带资源管理和业务逻辑
    2. **原始系统调用封装**：如 `OSAL_socket()`, `OSAL_bind()`, `OSAL_open()`, `OSAL_close()` - 1:1映射系统调用，仅做跨平台适配
  - 参考Linux uapi形式，按模块划分（socket/unistd/fcntl等）
- **HAL层**：封装硬件驱动，隔离硬件差异
  - **必须使用OSAL封装的系统调用**，禁止直接调用 `socket()`, `bind()`, `open()`, `close()` 等
  - 允许使用硬件特定的ioctl操作（如CAN的SIOCGIFINDEX）
- **PDL层**：外设驱动层，统一管理卫星/载荷/MCU等外设
  - **必须通过HAL层或OSAL接口访问底层**
- **Apps层**：应用逻辑，通过PDL层访问底层
  - **严格禁止任何系统调用**

### 2.1 OSAL系统调用封装架构（重要）

**设计原则**：参考Linux uapi，按功能模块划分系统调用封装，便于RTOS移植

**模块划分**（规划中）：
```
osal/include/
├── osal_socket.h      # Socket操作：OSAL_socket, OSAL_bind, OSAL_connect, OSAL_setsockopt
├── osal_unistd.h      # 文件I/O：OSAL_open, OSAL_close, OSAL_read, OSAL_write
├── osal_fcntl.h       # 文件控制：OSAL_fcntl
├── osal_ioctl.h       # 设备控制：OSAL_ioctl
├── osal_select.h      # I/O多路复用：OSAL_select
└── osal_termios.h     # 串口控制：OSAL_tcgetattr, OSAL_tcsetattr
```

**封装原则**：
1. **1:1映射**：接口名称、参数、返回值与系统调用保持一致
2. **无业务逻辑**：不引入资源管理、ID分配等额外逻辑
3. **跨平台适配**：仅做必要的类型转换和平台差异处理
4. **便于移植**：RTOS移植时只需实现对应模块的封装

**使用示例**（HAL层CAN驱动）：
```c
/* ✅ 正确：使用OSAL原始系统调用封装 */
int32 HAL_CAN_Init(const hal_can_config_t *config, hal_can_handle_t *handle)
{
    int32 sockfd;
    struct sockaddr_can addr;
    struct ifreq ifr;
    
    /* 创建socket（使用OSAL封装） */
    sockfd = OSAL_socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (sockfd < 0) {
        return OS_ERROR;
    }
    
    /* 获取接口索引（硬件操作，允许使用ioctl） */
    OSAL_ioctl(sockfd, SIOCGIFINDEX, &ifr);
    
    /* 绑定接口（使用OSAL封装） */
    OSAL_bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    
    return OS_SUCCESS;
}

/* ❌ 错误：直接调用系统调用 */
int32 HAL_CAN_Init(const hal_can_config_t *config, hal_can_handle_t *handle)
{
    int sockfd = socket(PF_CAN, SOCK_RAW, CAN_RAW);  // ❌ 禁止
    bind(sockfd, ...);  // ❌ 禁止
}
```

**关键规则**：
- ✅ HAL/PDL/Apps层必须使用 `OSAL_socket()` 而不是 `socket()`
- ✅ HAL/PDL/Apps层必须使用 `OSAL_open()` 而不是 `open()`
- ✅ HAL/PDL/Apps层必须使用 `OSAL_close()` 而不是 `close()`
- ✅ 所有标准库函数（memcpy/strlen等）必须使用OSAL封装
- ❌ 除OSAL层外，任何层都不允许 `#include <unistd.h>` 或 `#include <sys/socket.h>`

### 3. XConfig层（硬件配置库）
- **两层配置架构**：硬件配置层（定义外设硬件接口）+ APP配置层（定义APP使用哪些外设）
- **外设为单位**：以外设为单位（MCU/BMC/传感器等）管理硬件配置，类似Linux设备树
- **接口内嵌**：每个外设配置内嵌其通信接口配置（CAN/UART/I2C/SPI/Ethernet等）
- **多平台支持**：嵌套目录结构 `platform/<vendor>/<chip>/<product>/`
  - TI AM6254: H200-100P（100P算力）、H200-32P（32P算力）
  - 演示平台: 演示项目（2P算力，用于模拟测试）
- **配置选择**：支持环境变量/编译选项/默认配置三种方式

### 4. 任务管理（优雅关闭）
- 文件：`osal/src/linux/os_task.c`
- 配置：`osal/include/config/task_config.h`
- 特点：
  - 使用shutdown标志而非pthread_cancel（避免死锁）
  - 任务通过`OS_TaskShouldShutdown()`检查是否需要退出
  - 使用`pthread_timedjoin_np`等待任务退出（5秒超时）
  - 超时后使用`pthread_detach`而非强制取消

### 5. 日志系统
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

### 6. 通信冗余
- 主通道：以太网（IPMI over LAN）
- 备份通道：UART（IPMI over Serial）
- 配置：`hal/include/config/ethernet_config.h` 和 `hal/include/config/uart_config.h`
- 自动切换：连续5次失败后切换到备份通道
- 定期恢复：尝试恢复主通道

## 开发工作流

### 添加新功能
1. 在对应层的 `include/` 目录添加接口定义
2. 在 `src/` 目录实现功能
3. 在 `tests/` 目录添加单元测试
4. 运行 `./build.sh -d` 验证编译
5. 运行 `./output/target/bin/unit-test -a` 验证测试

### 修改配置
1. 找到对应模块的 `include/config/` 目录
2. 修改配置文件（如 `can_config.h`）
3. 重新编译：`./build.sh`

### 添加新测试
1. 在 `tests/src/<layer>/` 创建测试文件
2. 使用 `TEST_MODULE_BEGIN/END` 宏注册测试模块
3. 在 `tests/src/test_entry.c` 添加模块引用
4. 重新编译：`./build.sh -d`
5. 运行测试：`./output/target/bin/unit-test -m <module_name>`

### 调试
```bash
# Debug模式编译
./build.sh -d

# 使用GDB
sudo gdb ./output/target/bin/can_gateway
(gdb) run
(gdb) bt  # 查看调用栈

# 查看日志
tail -f /var/log/pmc-bsp.log

# 查看统计信息（程序每30秒自动打印）
grep "Statistics" /var/log/pmc-bsp.log

# 单步调试特定测试
sudo gdb --args ./output/target/bin/unit-test -m test_osal_task
```

## 构建输出

```
output/
├── build.log        # 构建日志
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

## 测试覆盖

| 层级 | 模块数 | 测试用例数 | 覆盖范围 |
|------|--------|-----------|---------|
| OSAL | 6 | 60 | 任务、队列、互斥锁、网络、文件、信号 |
| HAL | 1 | 3 | CAN驱动 |
| PDL | 1 | 2 | 卫星平台服务 |
| Apps | 2 | 5 | CAN网关、协议转换器 |
| **总计** | **10** | **70** | **完整的5层架构** |

### 交互式菜单特点
- 三级选择：层级 → 模块 → 测试用例
- 使用序号选择，无需输入完整名称
- 支持单个测试、模块测试、层级测试

## 编码规范

**详细规范请参考**：[docs/CODING_STANDARDS.md](docs/CODING_STANDARDS.md)

### 系统调用封装（最重要）
- **禁止**HAL/PDL/Apps/Tests层直接使用系统调用
- **必须**使用OSAL封装的接口：
  - 文件操作：`OSAL_open()`, `OSAL_close()`, `OSAL_read()`, `OSAL_write()`
  - Socket操作：`OSAL_socket()`, `OSAL_bind()`, `OSAL_connect()`, `OSAL_setsockopt()`
  - 内存操作：`OSAL_Memset()`, `OSAL_Memcpy()`, `OSAL_Malloc()`, `OSAL_Free()`
  - 字符串操作：`OSAL_Strlen()`, `OSAL_Strcmp()`, `OSAL_Strcpy()`, `OSAL_Snprintf()`
- **违规示例**：直接使用 `socket()`, `bind()`, `open()`, `close()`, `memcpy()`, `strlen()` 等

### 日志接口
- **禁止**直接使用`printf`/`fprintf`
- **使用**OSAL日志接口：
  - `OSAL_Printf()` - 简单输出（无格式）
  - `OSAL_INFO("MODULE", "message")` - 信息日志
  - `OSAL_ERROR("MODULE", "message")` - 错误日志
- **禁止**直接调用底层函数：`OSAL_LogInfo()`, `OSAL_LogError()` 等（仅供宏内部使用）

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

### 编译警告处理
- 项目使用`-Werror`，所有警告视为错误
- 必须修复所有警告才能编译通过

### 测试失败（硬件相关）
- CAN测试失败：需要CAN设备（can0）
- 串口测试失败：需要串口设备（/dev/ttyS0）
- 这些失败是正常的，不影响代码逻辑

### 任务优雅退出
- 任务循环必须检查`OS_TaskShouldShutdown()`
- 不要使用无限循环`while(1)`
- 退出前清理资源

### CAN接口无法启动
检查CAN驱动是否加载：
```bash
lsmod | grep can
sudo modprobe can
sudo modprobe can_raw
sudo modprobe vcan
```

### 权限不足
需要root权限访问CAN设备：
```bash
sudo ./output/target/bin/can_gateway
```

### 载荷连接失败
检查网络连接和IP配置：
```bash
ping 192.168.1.100
telnet 192.168.1.100 623
```

## 最近重构

### 系统调用封装重构（2026-04-25）
- **目标**：实现完整的系统调用封装，支持RTOS移植
- **架构设计**：参考Linux uapi，按模块划分（socket/unistd/fcntl/ioctl等）
- **封装原则**：1:1映射系统调用，不引入业务逻辑，仅做跨平台适配
- **实施范围**：
  - 创建OSAL原始系统调用封装接口（规划中）
  - 修改HAL层使用OSAL封装（进行中）
  - 修改PDL/Apps/Tests层使用OSAL封装（待完成）
- **关键变更**：
  - HAL层CAN驱动：`socket()` → `OSAL_socket()`, `bind()` → `OSAL_bind()`
  - HAL层串口驱动：`open()` → `OSAL_open()`, `close()` → `OSAL_close()`
  - 所有层：`memcpy()` → `OSAL_Memcpy()`, `strlen()` → `OSAL_Strlen()`

### Service层重命名为PDL层（2026-04-24）
- 原名称`service`容易与业务服务混淆，新名称`pdl`（Peripheral Driver Layer，外设驱动层）更准确
- 架构理念：管理板为核心，卫星/载荷/BMC/MCU统一抽象为外设
- 命名变更：`service/` → `pdl/`, `service_*.h` → `pdl_*.h`, `SatelliteService_*` → `SatellitePDL_*`
- 外设框架：新增统一外设接口（`peripheral_device.h`），支持MCU/卫星/BMC/Linux载荷
- 适配器模式：保留传统接口100%兼容，通过适配器包装到外设框架

### 目录结构标准化
- `inc` → `include`：统一使用 `include/` 目录存放头文件
- `linux` → `src/linux`：统一使用 `src/linux/` 目录存放Linux平台实现
- `config` 移入 `include`：配置文件统一放在 `include/config/` 目录
- 标准化结构：`module/include/` + `module/src/linux/` + `module/include/config/`

### 模块化配置重构
- 配置下沉：将全局 `config/` 目录拆分，配置文件下沉到各模块内部
- 依赖隔离：各模块只依赖自己的配置，便于多人协作和多仓库拆分
- 构建日志：`build.log` 生成到 `output/` 目录，所有构建产物统一管理

### 测试框架重构
- 重命名：`test_framework.h` → `unittest_framework.h`
- 重命名：`test_runner.c/h` → `unittest_runner.c/h`
- 重命名：`test_main_unified.c` → `unittest_entry.c`
- 统一日志：所有`printf`替换为`OS_printf`
- 目录重组：测试核心文件移至`tests/core/`目录

## 开发建议

1. **系统调用封装（最重要）**：HAL/PDL/Apps/Tests层严禁直接调用系统调用，必须使用OSAL封装
2. **遵循分层架构**：不要跨层直接调用
3. **模块独立性**：各模块只依赖自己的 `include/config/` 目录，不要引用其他模块的配置
4. **使用OSAL接口**：不要直接使用pthread/socket/open/close等系统API
5. **优雅退出**：任务循环检查`OS_TaskShouldShutdown()`
6. **错误处理**：所有返回值必须检查
7. **日志规范**：使用`OSAL_INFO/ERROR`宏，不要用`printf`或直接调用`OSAL_LogInfo`
8. **测试驱动**：新功能必须编写单元测试
9. **配置管理**：修改配置时只修改对应模块的 `include/config/` 目录

## 快速开发技巧

### 快速编译单个目标
```bash
cd output/build && make can_gateway -j$(nproc) && cd ../..
```

### 快速测试单个模块
```bash
./build.sh -d && ./output/target/bin/unit-test -m test_osal_task
```

### 监控日志并运行应用
```bash
tail -f /var/log/pmc-bsp.log &
sudo ./output/target/bin/can_gateway
```

### 检查代码风格（使用clang-format）
```bash
find . -name "*.c" -o -name "*.h" | xargs clang-format -i --style=file
```

## 性能指标

- CAN消息延迟：< 10ms
- 命令处理时间：< 100ms
- 内存占用：< 128MB
- CPU占用（空闲）：< 5%
