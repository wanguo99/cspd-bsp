# CSPD-BSP

卫星算存载荷板级支持包 (Compute and Storage Payload Board Support Package)

## 系统概述

CSPD-BSP 是专为卫星算存载荷设计的板级支持包，提供卫星平台与算存载荷之间的通信桥接和管理功能。

```
卫星平台 <--CAN--> 算存载荷管理板 <--Ethernet/UART--> 算存载荷
```

## 主要功能

- **CAN通信**: 与卫星平台进行CAN总线通信
- **协议转换**: 卫星命令 ↔ IPMI/Redfish命令
- **通信冗余**: 以太网主通道 + UART备份通道
- **状态监控**: 定期查询载荷状态并上报
- **故障处理**: 自动检测和恢复通信故障
- **高稳定性**: 看门狗、心跳、重试机制

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
./build.sh

# 或手动使用CMake
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### 4. 配置系统参数

编辑 `config/system_config.h`:

```c
/* CAN接口 */
#define CAN_INTERFACE           "can0"

/* 载荷IP地址 */
#define SERVER_IP_ADDRESS       "192.168.1.100"

/* UART设备 */
#define UART_DEVICE             "/dev/ttyS0"
```

### 5. 配置CAN接口

```bash
# 设置CAN波特率为500Kbps
sudo ip link set can0 type can bitrate 500000

# 启动CAN接口
sudo ip link set can0 up

# 验证CAN接口
ip -details link show can0
```

### 6. 运行

```bash
sudo ./build/bin/cspd-bsp
```

## 目录结构

```
cspd-bsp/
├── CMakeLists.txt              # 根CMake配置
├── build.sh                    # 自动构建脚本
├── README.md                   # 本文件
├── main.c                      # 主程序
├── config/                     # 配置文件
│   ├── system_config.h        # 系统配置
│   └── can_protocol.h         # CAN协议定义
├── osal/                       # 操作系统抽象层
│   ├── CMakeLists.txt         # OSAL构建配置
│   ├── inc/                   # 头文件
│   └── linux/                 # Linux实现
│       ├── os_task.c          # 任务管理
│       ├── os_queue.c         # 消息队列
│       ├── os_mutex.c         # 互斥锁
│       ├── os_clock.c         # 时间服务
│       ├── os_heap.c          # 内存管理
│       ├── os_error.c         # 错误处理
│       ├── os_init.c          # 初始化
│       └── os_log.c           # 日志系统
├── hal/                        # 硬件抽象层
│   ├── CMakeLists.txt         # HAL构建配置
│   ├── inc/                   # 接口定义
│   └── linux/                 # Linux驱动实现
│       ├── hal_can_linux.c    # CAN驱动
│       └── hal_server_linux.c # 载荷通信驱动
├── apps/                       # 应用模块
│   ├── CMakeLists.txt         # Apps构建配置
│   ├── can_gateway/           # CAN网关
│   └── protocol_converter/    # 协议转换
├── docs/                       # 文档目录
│   ├── ARCHITECTURE.md        # 架构设计文档
│   ├── CMAKE_GUIDE.md         # CMake使用指南
│   ├── QUICKSTART.md          # 快速入门指南
│   ├── DEPLOYMENT.md          # 部署运维指南
│   └── QUICK_REFERENCE.md     # 快速参考卡片
└── build/                      # 构建输出目录（自动生成）
    ├── bin/                   # 可执行文件
    │   └── cspd-bsp           # 主程序
    └── lib/                   # 静态库
        ├── libosal.a          # OSAL库
        ├── libhal.a           # HAL库
        └── libapps.a          # Apps库
```

## CAN协议

### 消息格式 (8字节)

| Byte | 说明 |
|------|------|
| 0 | 消息类型 |
| 1 | 命令类型/状态码 |
| 2-3 | 序列号 |
| 4-7 | 数据/参数 |

### 消息类型

- `0x01`: 命令请求
- `0x02`: 命令响应
- `0x03`: 状态查询
- `0x04`: 状态上报
- `0x05`: 心跳
- `0x06`: 错误报告

### 命令类型

- `0x10`: 载荷上电
- `0x11`: 载荷下电
- `0x12`: 载荷重启
- `0x20`: 查询状态
- `0x21`: 查询温度
- `0x22`: 查询电压

## 测试

### 模拟CAN发送

```bash
# 发送上电命令
cansend can0 100#0110000100000000

# 发送查询状态命令
cansend can0 100#0120000200000000
```

### 监控CAN消息

```bash
candump can0
```

## 性能指标

| 指标 | 目标值 |
|------|--------|
| CAN消息延迟 | < 10ms |
| 命令处理时间 | < 100ms |
| 内存占用 | < 128MB |
| CPU占用(空闲) | < 5% |

## 构建选项

### Debug模式编译

```bash
./build.sh -d
```

### 清理后重新编译

```bash
./build.sh -c -r
```

### 安装到系统

```bash
./build.sh -i --prefix /opt/cspd-bsp
```

### 查看所有选项

```bash
./build.sh -h
```

## 故障处理

### 以太网故障

- 自动检测连续5次失败
- 自动切换到UART通道
- 定期尝试恢复以太网

### 载荷离线

- 返回离线状态码
- 缓存命令等待恢复
- 定期重试连接

## 开发指南

### 添加新命令

1. 在 `config/can_protocol.h` 添加命令类型
2. 在 `protocol_converter.c` 的 `execute_ipmi_command()` 添加处理逻辑
3. 重新编译

### 添加新应用模块

1. 在 `apps/` 创建新目录
2. 实现初始化函数
3. 在 `main.c` 调用初始化
4. 在 `apps/CMakeLists.txt` 添加源文件
5. 重新编译

## 调试

### 查看日志

```bash
# 实时查看
tail -f /var/log/cspd-bsp.log

# 查看统计信息
# 程序每30秒自动打印统计
```

### 调试模式

在 `config/system_config.h` 设置:

```c
#define DEBUG_MODE              1
#define LOG_LEVEL               LOG_LEVEL_DEBUG
```

## 常见问题

### Q: CAN接口无法启动

A: 检查CAN驱动是否加载:
```bash
lsmod | grep can
sudo modprobe can
sudo modprobe can_raw
sudo modprobe vcan
```

### Q: 权限不足

A: 需要root权限访问CAN设备:
```bash
sudo ./build/bin/cspd-bsp
```

### Q: 载荷连接失败

A: 检查网络连接和IP配置:
```bash
ping 192.168.1.100
telnet 192.168.1.100 623
```

### Q: CMake版本过低

A: 安装最新版CMake:
```bash
sudo apt install cmake
# 或从源码安装
```

## 文档

详细文档请查看 `docs/` 目录：

- **[架构设计](docs/ARCHITECTURE.md)** - 系统架构和设计思想
- **[CMake指南](docs/CMAKE_GUIDE.md)** - CMake构建系统使用
- **[快速入门](docs/QUICKSTART.md)** - 详细的入门教程
- **[部署指南](docs/DEPLOYMENT.md)** - 生产环境部署
- **[快速参考](docs/QUICK_REFERENCE.md)** - API快速参考

## 许可证

Apache 2.0
