# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

**PMC-BSP** (Payload Management Controller Board Support Package) 是为算存载荷管理控制器设计的板级支持包，PMC是作为卫星平台与算存载荷之间的通信桥接和管理中间层。

**系统架构**：
```
卫星平台 <--CAN--> 算存载荷管理控制器(PMC) <--Ethernet/UART/CAN--> 算存载荷
```

## 数据类型使用规范

### 类型定义（定义在 `osal/include/osal_types.h`）

```c
/* 字符串类型 */
str_t                           // 字符串类型（底层是char，与标准C库兼容）

/* 数值类型 - 明确大小 */
int8, int16, int32, int64       // 有符号整数
uint8, uint16, uint32, uint64   // 无符号整数

/* 布尔类型 */
bool                            // true/false

/* OSAL特定类型 */
osal_id_t                       // 对象ID（uint32）
osal_size_t                     // 大小类型（平台相关）
osal_ssize_t                    // 有符号大小类型
```

### 使用规则

**1. 字符串和文本数据 - 使用 `str_t`**
```c
str_t device_name[64];          // ✅ 设备名称
str_t log_message[256];         // ✅ 日志消息
str_t version_string[32];       // ✅ 版本字符串
const str_t *interface;         // ✅ 字符串指针
str_t parity;                   // ✅ 校验位字符 ('N', 'E', 'O')
```

**2. 字节数据和二进制数据 - 使用 `uint8` / `int8`**
```c
uint8 can_data[8];              // ✅ CAN数据字节
uint8 mac_address[6];           // ✅ MAC地址
int8 temperature_offset;        // ✅ 温度偏移（有符号字节）
uint8 register_value;           // ✅ 寄存器值
```

**3. 数值数据 - 使用固定大小类型**
```c
uint32 baudrate;                // ✅ 波特率
int32 temperature;              // ✅ 温度值
uint16 port;                    // ✅ 端口号
int16 voltage_mv;               // ✅ 电压（毫伏）
```

**4. 禁止使用的类型**
```c
char device_name[64];           // ❌ 应使用 str_t
char can_data[8];               // ❌ 应使用 uint8（如果是二进制数据）
int size;                       // ❌ 应使用 int32（明确大小）
unsigned int count;             // ❌ 应使用 uint32
```

### 设计原则

- **str_t 用于字符串**：与标准C库（strcpy, strlen, fopen等）完全兼容
- **int8/uint8 用于字节**：明确表示二进制数据，避免与字符串混淆
- **固定大小类型**：确保跨平台一致性（32位/64位系统）
- **参考标准**：NASA cFS OSAL、MISRA C、Linux Kernel风格


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
./output/target/bin/unit-test -L HAL   # 运行HAL层测试
./output/target/bin/unit-test -L PDL   # 运行PDL层测试
./output/target/bin/unit-test -m test_osal_task  # 运行指定模块
./output/target/bin/unit-test -l    # 列出所有测试

# 编译单个测试模块（快速迭代）
cd output/build
make test_osal_task  # 仅编译指定测试模块
cd ../..

# 完整的测试工作流
./build.sh -d                        # Debug模式编译
./output/target/bin/unit-test -i    # 交互式运行测试
```

### 运行应用
```bash
./output/target/bin/sample_app      # 示例应用（展示OSAL基本用法）
```

## 核心功能

PMC-BSP采用5层分层架构：

### OSAL - 操作系统抽象层
跨平台的操作系统抽象接口，提供任务、队列、互斥锁、日志等基础服务，所有标准库函数、系统调用都应该在此进行封装。

**特性**：用户态库设计、线程安全、优雅关闭、死锁检测、日志轮转

**文档**: [osal/README.md](osal/README.md) | [详细文档](osal/docs/)

### HAL - 硬件抽象层
硬件驱动封装，提供CAN、串口等硬件接口，只允许PDL访问该库。

**特性**：平台隔离、统一接口、驱动封装、配置管理

**文档**: [hal/README.md](hal/README.md) | [详细文档](hal/docs/)

### PDL - 外设驱动层
统一管理卫星平台、BMC载荷、MCU等外设服务，对应用提供访问外设的统一接口，只允许APP和TEST层访问该库的接口。

**特性**：统一外设管理、多通道冗余、自动故障切换、心跳机制

**文档**: [pdl/README.md](pdl/README.md) | [详细文档](pdl/docs/)

### PCL - 外设配置库
参考设备树架构，以外设为单位的硬件配置库，只允许PDL访问该库。

**特性**：外设为单位、配置与代码分离、接口内嵌、运行时查询

**文档**: [pcl/README.md](pcl/README.md) | [详细文档](pcl/docs/)

### Apps - 应用层
暂未加入业务应用，只有示例应用，用于后期扩展使用参考。

**特性**：平台无关、使用抽象接口、优雅退出、错误处理

**文档**: [apps/README.md](apps/README.md) | [详细文档](apps/docs/)

### 架构依赖关系

**正确的依赖链：**
```
Apps
  ↓
PDL (外设服务)
  ↓
HAL (硬件驱动：CAN/串口/I2C/SPI等)
  ↓
OSAL (操作系统抽象：任务/队列/互斥锁/socket/文件等)
  ↓
Linux系统调用
```

**重要说明**：
- **OSAL是所有层的基础**：PCL、HAL、PDL、Apps、Tests都可以直接使用OSAL接口
- **HAL封装硬件设备**：CAN控制器、串口芯片、I2C/SPI总线等特定硬件
- **网络socket属于OSAL**：socket是操作系统提供的通用接口，不是特定硬件设备
- **PDL调用OSAL_socket()是正确的**：不存在跨层调用问题

## 核心功能详解

- **OSAL抽象层**：提供跨平台的操作系统抽象接口（任务、队列、互斥锁、网络、文件等）
- **HAL硬件层**：封装硬件驱动（CAN、串口、网络等）
- **PDL外设层**：统一管理卫星/载荷/MCU等外设
- **PCL配置**：硬件配置库，支持多平台多产品配置

## 代码结构（5层架构 + 模块化配置）

```
pmc-bsp/
├── osal/                    # 操作系统抽象层 (OSAL)
│   ├── include/             # 接口定义
│   │   ├── osal.h           # 总头文件
│   │   ├── osal_types.h     # 类型定义
│   │   ├── sys/             # 系统调用封装（任务、互斥锁、信号等）
│   │   ├── ipc/             # 进程间通信（队列、共享内存等）
│   │   ├── net/             # 网络抽象（socket封装）
│   │   ├── lib/             # 标准库封装（字符串、内存等）
│   │   └── util/            # 工具函数（日志、版本等）
│   └── src/posix/           # POSIX实现
│       ├── ipc/             # 进程间通信实现
│       │   ├── osal_task.c      # 任务管理（pthread）
│       │   ├── osal_queue.c     # 消息队列
│       │   ├── osal_mutex.c     # 互斥锁（带死锁检测）
│       │   └── osal_atomic.c    # 原子操作
│       ├── sys/             # 系统调用实现
│       │   ├── osal_clock.c     # 时钟
│       │   ├── osal_signal.c    # 信号处理
│       │   ├── osal_file.c      # 文件操作
│       │   ├── osal_select.c    # I/O多路复用
│       │   ├── osal_env.c       # 环境变量
│       │   └── osal_time.c      # 时间操作
│       ├── net/             # 网络实现
│       │   ├── osal_socket.c    # Socket封装
│       │   └── osal_termios.c   # 串口控制
│       ├── lib/             # 标准库封装
│       │   ├── osal_heap.c      # 内存管理
│       │   ├── osal_string.c    # 字符串操作
│       │   └── osal_errno.c     # 错误处理
│       └── util/            # 工具实现
│           ├── osal_version.c   # 版本信息
│           └── osal_log.c       # 日志系统（带轮转）
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
├── pcl/                 # 硬件配置库 (PCL)
│   ├── include/             # 接口定义
│   │   ├── pcl.h       # 总头文件
│   │   ├── pcl_common.h # 通用类型（GPIO、电源域）
│   │   ├── pcl_hardware_interface.h # 硬件接口定义
│   │   ├── pcl_mcu.h   # MCU外设配置
│   │   ├── pcl_bmc.h   # BMC外设配置
│   │   ├── pcl_satellite.h # 卫星平台接口配置
│   │   ├── pcl_sensor.h # 传感器外设配置
│   │   ├── pcl_storage.h # 存储设备配置
│   │   ├── pcl_app.h   # APP配置
│   │   └── pcl_board.h # 板级配置
│   ├── src/                 # 源代码
│   │   ├── pcl_api.c   # API实现
│   │   └── pcl_register.c # 配置注册
│   └── platform/            # 平台配置（嵌套目录结构）
│       ├── ti/am6254/        # TI AM6254平台
│       │   └── H200_100P/  # H200-100P产品（100P算力）
│       │       ├── h200_100p_base.c
│       │       ├── h200_100p_v1.c
│       │       └── h200_100p_v2.c
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
│   └── sample_app/          # 示例应用
│       ├── README.md        # 应用说明文档
│       └── src/             # 源代码
│           └── main.c       # 主程序（展示OSAL基本用法）
├── output/                  # 构建输出目录
│   ├── build.log            # 构建日志
│   ├── build/               # 编译中间文件
│   └── target/              # 最终产物
│       ├── bin/             # 可执行文件
│       └── lib/             # 静态库
└── tests/                   # 测试框架
    ├── include/             # 测试框架头文件
    │   ├── test_framework.h     # 测试框架宏定义
    │   └── test_runner.h        # 测试运行器接口
    ├── src/                 # 测试源代码
    │   ├── test_entry.c         # 统一测试入口
    │   ├── test_runner.c        # 测试运行器实现
    │   ├── osal/                # OSAL层测试
    │   └── hal/                 # HAL层测试
    └── docs/                # 测试文档
```

## 关键设计特点

### 1. 模块化配置（重要）
- **配置分布**：配置文件分布在各模块的 `include/config/` 目录
- **HAL配置**：`hal/include/config/` - CAN、串口等硬件配置
- **PCL配置**：`pcl/platform/` - 以外设为单位的硬件配置
- **Apps配置**：`apps/*/include/config/` - 应用层协议和参数配置
- **依赖隔离**：
  - OSAL层：提供基础抽象接口
  - HAL层：依赖OSAL接口
  - PDL层：依赖HAL接口和OSAL接口
  - Apps层：依赖PDL接口、HAL接口、OSAL接口

### 2. 分层隔离与平台无关性（关键）
- **平台相关代码隔离原则**：
  - **OSAL层**：唯一允许包含操作系统相关代码的层（Linux/RTOS/Windows等）
  - **HAL层**：唯一允许包含硬件平台相关代码的层（ARM/x86/RISC-V等，SocketCAN/硬件CAN等）
  - **PCL/PDL/Apps/Tests层**：必须保持完全平台无关，可无修改移植到任何平台
  
- **OSAL层**：封装操作系统API，支持跨平台移植
  - **唯一允许直接调用系统调用的层**
  - 提供两类接口：
    1. **高层业务接口**：如 `OSAL_TaskCreate()`, `OSAL_SocketOpen()` - 带资源管理和业务逻辑
    2. **原始系统调用封装**：如 `OSAL_socket()`, `OSAL_bind()`, `OSAL_open()`, `OSAL_close()` - 1:1映射系统调用，仅做跨平台适配
  - 参考Linux uapi形式，按模块划分（socket/unistd/fcntl等）
  - 平台相关实现放在 `src/linux/`, `src/freertos/`, `src/vxworks/` 等目录
  
- **HAL层**：封装硬件驱动，隔离硬件差异
  - **必须使用OSAL封装的系统调用**，禁止直接调用 `socket()`, `bind()`, `open()`, `close()` 等
  - **唯一允许包含硬件平台相关代码的层**（如SocketCAN、硬件寄存器操作等）
  - 平台相关实现放在 `src/linux/`, `src/ti_am62/`, `src/nxp_imx8/` 等目录
  
- **PCL层**：硬件配置库，纯数据配置
  - **必须保持完全平台无关**，只包含配置数据结构
  - 禁止包含任何系统头文件（`<unistd.h>`, `<sys/socket.h>` 等）
  - 禁止调用任何系统API，只能使用OSAL接口
  
- **PDL层**：外设驱动层，统一管理卫星/载荷/MCU等外设
  - **必须保持完全平台无关**，通过HAL层或OSAL接口访问底层
  - 禁止包含任何系统头文件或硬件相关头文件
  
- **Apps层**：应用逻辑，通过PDL层访问底层
  - **必须保持完全平台无关**，严格禁止任何系统调用
  - 禁止包含任何系统头文件
  
- **Tests层**：测试代码
  - **必须保持完全平台无关**，只能使用OSAL接口
  - 禁止包含任何系统头文件

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

### 3. PCL层（硬件配置库）
- **两层配置架构**：硬件配置层（定义外设硬件接口）+ APP配置层（定义APP使用哪些外设）
- **外设为单位**：以外设为单位（MCU/BMC/传感器等）管理硬件配置，类似Linux设备树
- **接口内嵌**：每个外设配置内嵌其通信接口配置（CAN/UART/I2C/SPI/Ethernet等）
- **多平台支持**：嵌套目录结构 `platform/<vendor>/<chip>/<product>/`
  - TI AM6254: H200-100P（100P算力）
  - 演示平台: 演示项目（2P算力，用于模拟测试）
- **配置选择**：支持环境变量/编译选项/默认配置三种方式

### 4. OSAL接口设计（用户态库）
- 文件：`osal/src/posix/util/osal_version.c`
- 头文件：`osal/include/osal.h`
- 特点：
  - **无需显式初始化**：OSAL作为用户态库，使用静态初始化，无需调用Init/Teardown
  - **无空闲循环**：用户态应用由操作系统调度，不需要OS_IdleLoop
  - **简洁接口**：仅保留必要的业务接口，删除了OS_API_Init/OS_API_Teardown/OS_IdleLoop
  - **版本信息**：通过`OS_GetVersionString()`获取OSAL版本

### 5. 任务管理（优雅关闭）
- 文件：`osal/src/posix/ipc/osal_task.c`
- 头文件：`osal/include/ipc/osal_task.h`
- 特点：
  - 使用shutdown标志而非pthread_cancel（避免死锁）
  - 任务通过`OSAL_TaskShouldShutdown()`检查是否需要退出
  - 使用`pthread_timedjoin_np`等待任务退出（5秒超时）
  - 超时后使用`pthread_detach`而非强制取消

### 6. 日志系统
- 文件：`osal/src/posix/util/osal_log.c`
- 头文件：`osal/include/util/osal_log.h`
- 功能：
  - 支持多级别日志（DEBUG/INFO/WARN/ERROR/FATAL）
  - 日志轮转（按大小，保留N个文件）
  - 线程安全
- 接口：
  - `OSAL_Printf()` - 简单打印
  - `LOG_INFO(module, ...)` - 带模块名的日志宏
  - `LOG_ERROR(module, ...)` - 错误日志

### 7. 错误处理统一
- 文件：`osal/src/posix/lib/osal_errno.c`
- 头文件：`osal/include/lib/osal_errno.h`
- 特点：
  - 合并了原有的osal_error模块到osal_errno
  - 统一的错误码定义和错误信息获取
  - 线程安全的错误处理机制

## 开发工作流

### 添加新功能
1. 在对应层的 `include/` 目录添加接口定义
2. 在 `src/` 目录实现功能
3. 在 `tests/` 目录添加单元测试
4. 运行 `./build.sh -d` 验证编译
5. 运行 `./output/target/bin/unit-test -a` 验证测试

### 修改配置
1. 找到对应模块的配置文件（通常在 `include/config/` 目录）
2. 修改配置参数
3. 重新编译：`./build.sh`

### 添加新测试
1. 在 `tests/src/<layer>/` 创建测试文件（如`test_new_module.c`）
2. 使用 `TEST_MODULE_BEGIN/END` 宏注册测试模块
3. 在 `tests/src/test_entry.c` 的对应层级数组中添加模块引用
4. 重新编译：`./build.sh -d`
5. 运行测试：`./output/target/bin/unit-test -m test_new_module`

### 调试
```bash
# Debug模式编译
./build.sh -d

# 使用GDB
gdb ./output/target/bin/sample_app
(gdb) run
(gdb) bt  # 查看调用栈

# 单步调试特定测试
gdb --args ./output/target/bin/unit-test -m test_osal_task
```

## 构建输出

```
output/
├── build.log        # 构建日志
├── build/           # 编译中间文件
└── target/
    ├── bin/         # 可执行文件
    │   ├── sample_app
    │   └── unit-test
    └── lib/         # 静态库
        ├── libosal.a
        ├── libhal.a
        ├── libpdl.a
        └── libpcl.a
```

## 测试覆盖

| 层级 | 模块数 | 测试用例数 | 覆盖范围 |
|------|--------|-----------|---------|
| OSAL | 5 | 50+ | 任务、队列、互斥锁、信号 |
| HAL | 1 | 3 | CAN驱动 |
| **总计** | **6** | **53+** | **核心功能完整覆盖** |

### 交互式菜单特点
- 三级选择：层级 → 模块 → 测试用例
- 使用序号选择，无需输入完整名称
- 支持单个测试、模块测试、层级测试

## 编码规范

**详细规范请参考**：[docs/CODING_STANDARDS.md](docs/CODING_STANDARDS.md)

### 平台无关性（最重要）
- **平台相关代码隔离**：
  - ✅ **OSAL层**：唯一允许包含操作系统相关代码（`#include <unistd.h>`, `pthread_*` 等）
  - ✅ **HAL层**：唯一允许包含硬件平台相关代码（SocketCAN、硬件寄存器等）
  - ❌ **PCL/PDL/Apps/Tests层**：严禁包含任何平台相关代码，必须可无修改移植
  
- **系统调用封装**：
  - **禁止**PCL/PDL/Apps/Tests层直接使用系统调用
  - **必须**使用OSAL封装的接口：
    - 文件操作：`OSAL_open()`, `OSAL_close()`, `OSAL_read()`, `OSAL_write()`
    - Socket操作：`OSAL_socket()`, `OSAL_bind()`, `OSAL_connect()`, `OSAL_setsockopt()`
    - 内存操作：`OSAL_Memset()`, `OSAL_Memcpy()`, `OSAL_Malloc()`, `OSAL_Free()`
    - 字符串操作：`OSAL_Strlen()`, `OSAL_Strcmp()`, `OSAL_Strcpy()`, `OSAL_Snprintf()`
    - 环境变量：`OSAL_getenv()`, `OSAL_setenv()`, `OSAL_unsetenv()`
  - **违规示例**：直接使用 `socket()`, `bind()`, `open()`, `close()`, `memcpy()`, `strlen()`, `getenv()` 等
  
- **头文件包含规则**：
  - ✅ OSAL层：可以包含 `<unistd.h>`, `<sys/socket.h>`, `<pthread.h>`, `<stdlib.h>` 等
  - ✅ HAL层：可以包含硬件相关头文件（`<linux/can.h>`, `<net/if.h>` 等），但必须使用OSAL封装的系统调用
  - ❌ PCL/PDL/Apps/Tests层：严禁包含任何系统头文件，只能包含OSAL接口头文件

### 日志接口
- **禁止**直接使用`printf`/`fprintf`
- **使用**OSAL日志接口：
  - `OSAL_Printf()` - 简单输出（无格式）
  - `LOG_INFO("MODULE", "message")` - 信息日志
  - `LOG_ERROR("MODULE", "message")` - 错误日志
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
    osal_id_t task_id = OSAL_TaskGetId();
    
    while (!OSAL_TaskShouldShutdown())  // 检查退出标志
    {
        // 执行任务逻辑
        do_work();
        
        // 延时
        OSAL_TaskDelay(100);
    }
    
    // 清理资源
    cleanup();
}
```

## 关键配置

### CAN配置
- 配置目录：`hal/include/config/`
- 接口：`can0`
- 波特率：500Kbps
- 协议：CAN 2.0B标准帧

### 串口配置
- 配置目录：`hal/include/config/`
- 设备：`/dev/ttyS0`
- 波特率：115200
- 数据位：8位，无校验，1停止位

## 常见问题

### 编译警告处理
- 项目使用`-Werror`，所有警告视为错误
- 必须修复所有警告才能编译通过

### 测试失败（硬件相关）
- CAN测试失败：需要CAN设备（can0）
- 串口测试失败：需要串口设备（/dev/ttyS0）
- 这些失败是正常的，不影响代码逻辑

### 任务优雅退出
- 任务循环必须检查`OSAL_TaskShouldShutdown()`
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
某些硬件操作可能需要root权限：
```bash
sudo ./output/target/bin/sample_app
```

## 最近重构

### OSAL接口精简（2026-04-26）
- **目标**：简化OSAL接口，适配用户态库设计
- **删除接口**：
  - `OS_API_Init()` / `OS_API_Teardown()` - 用户态库无需显式初始化
  - `OS_IdleLoop()` - 用户态应用由操作系统调度，不需要空闲循环
- **保留接口**：
  - `OS_GetVersionString()` - 获取OSAL版本信息
- **文件变更**：
  - 删除 `osal/src/posix/util/osal_init.c`
  - 新增 `osal/src/posix/util/osal_version.c`
- **设计理念**：OSAL作为用户态库，使用静态初始化，无需显式Init/Teardown

### 应用层重构（2026-04-26）
- **删除应用**：can_gateway、protocol_converter（业务应用，不属于BSP核心）
- **新增应用**：sample_app（示例应用，展示OSAL基本用法）
- **设计理念**：BSP专注于提供抽象层和驱动，业务应用由用户实现

### PCL平台简化（2026-04-26）
- **删除配置**：H200_32P平台配置（暂时无用）
- **保留配置**：H200_100P（实际产品）、vendor_demo（演示用）
- **设计理念**：按需配置，避免维护无用代码

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

### 目录结构标准化（2026-04-24）
- `inc` → `include`：统一使用 `include/` 目录存放头文件
- `linux` → `src/posix`：统一使用 `src/posix/` 目录存放POSIX实现
- `config` 移入 `include`：配置文件统一放在 `include/config/` 目录
- 标准化结构：`module/include/` + `module/src/posix/` + `module/include/config/`

### 模块化配置重构
- 配置分布：配置文件分布在各模块的 `include/config/` 目录
- 依赖隔离：各模块通过接口依赖，便于多人协作和多仓库拆分
- 构建日志：`build.log` 生成到 `output/` 目录，所有构建产物统一管理

### 测试框架重构
- 统一命名：使用`test_`前缀（`test_framework.h`, `test_runner.c/h`, `test_entry.c`）
- 统一日志：所有`printf`替换为`OSAL_Printf`
- 目录重组：测试框架头文件在`tests/include/`，实现在`tests/src/`

## 开发建议

1. **OSAL接口使用**：
   - 用户态库无需显式初始化，直接调用OSAL接口即可
   - 任务循环检查`OSAL_TaskShouldShutdown()`实现优雅退出
   - 使用LOG_INFO/LOG_ERROR宏记录日志，不要用printf
2. **系统调用封装（最重要）**：HAL/PDL/Apps/Tests层严禁直接调用系统调用，必须使用OSAL封装
3. **遵循分层架构**：不要跨层直接调用
4. **使用OSAL接口**：不要直接使用pthread/socket/open/close等系统API
5. **错误处理**：所有返回值必须检查
6. **测试驱动**：新功能必须编写单元测试
7. **配置管理**：配置文件集中在各模块的配置目录中

## 常见开发场景

### 添加新的OSAL接口
1. 在`osal/include/`添加接口声明
2. 在`osal/src/posix/`实现POSIX版本
3. 在`osal/tests/`添加单元测试
4. 更新`osal/README.md`文档

### 添加新的HAL驱动
1. 在`hal/include/`添加驱动接口
2. 在`hal/include/config/`添加驱动配置
3. 在`hal/src/linux/`实现Linux版本（使用OSAL接口）
4. 在`hal/tests/`添加单元测试
5. 更新`hal/README.md`文档

### 添加新的外设支持
1. 在`pcl/include/peripheral/`添加外设配置结构
2. 在`pcl/platform/`添加具体平台配置
3. 在`pdl/include/`添加外设服务接口
4. 在`pdl/src/`实现外设服务
5. 在`pdl/tests/`添加单元测试

### 调试硬件相关问题
```bash
# 检查CAN设备
ip link show can0
sudo ip link set can0 type can bitrate 500000
sudo ip link set can0 up

# 检查串口设备
ls -l /dev/ttyS*
sudo chmod 666 /dev/ttyS0

# 使用strace跟踪系统调用
strace -e trace=open,read,write,ioctl ./output/target/bin/sample_app

# 使用ltrace跟踪库调用
ltrace ./output/target/bin/sample_app
```

## 快速开发技巧

### 快速编译单个目标
```bash
cd output/build && make sample_app -j$(nproc) && cd ../..
cd output/build && make test_osal_task -j$(nproc) && cd ../..  # 编译单个测试模块
```

### 快速测试单个模块
```bash
./build.sh -d && ./output/target/bin/unit-test -m test_osal_task
```

### 运行示例应用
```bash
./output/target/bin/sample_app
```

### 调试技巧
```bash
# 使用GDB调试应用
gdb ./output/target/bin/sample_app
(gdb) break main
(gdb) run
(gdb) bt

# 使用GDB调试特定测试
gdb --args ./output/target/bin/unit-test -m test_osal_task

# 使用Valgrind检查内存泄漏
valgrind --leak-check=full ./output/target/bin/sample_app
```

## 性能指标

- 任务切换延迟：< 1ms
- 队列操作延迟：< 100μs
- 内存占用：< 64MB
- CPU占用（空闲）：< 2%

## 项目统计

- **代码规模**：约18,000行（生产代码14,000行，测试代码4,000行）
- **文件数量**：97个C/H文件
- **测试覆盖**：70+测试用例
- **模块数量**：5层架构（OSAL/HAL/PCL/PDL/Apps）
- **支持平台**：TI AM6254, 演示平台（可扩展到其他平台）

## 重要文件索引

### 核心配置文件
- [CMakeLists.txt](CMakeLists.txt) - 主构建配置
- [build.sh](build.sh) - 构建脚本
- [CLAUDE.md](CLAUDE.md) - 本文件（开发指南）
- [docs/CODING_STANDARDS.md](docs/CODING_STANDARDS.md) - 编码规范

### 模块入口文件
- [osal/include/osal.h](osal/include/osal.h) - OSAL层总头文件
- [hal/include/hal_can.h](hal/include/hal_can.h) - HAL CAN驱动接口
- [pcl/include/api/pcl_api.h](pcl/include/api/pcl_api.h) - PCL API
- [pdl/include/pdl_satellite.h](pdl/include/pdl_satellite.h) - PDL卫星服务接口
- [apps/sample_app/src/main.c](apps/sample_app/src/main.c) - 示例应用

### 测试入口
- [apps/test_runner/main.c](apps/test_runner/main.c) - 统一测试运行器
- [libutest/include/libutest.h](libutest/include/libutest.h) - 测试框架
