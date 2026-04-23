# CSPD-BSP 软件架构说明

## 目录结构

```
cspd-bsp/
├── apps/                      # 应用程序目录
│   ├── can_gateway/          # CAN网关应用
│   │   ├── can_gateway.c     # CAN网关核心逻辑
│   │   ├── can_gateway.h     # CAN网关头文件
│   │   └── main.c            # CAN网关主程序
│   └── protocol_converter/   # 协议转换应用
│       ├── protocol_converter.c  # 协议转换核心逻辑
│       ├── protocol_converter.h  # 协议转换头文件
│       ├── payload_service.c     # 载荷服务实现
│       ├── payload_service.h     # 载荷服务头文件
│       └── main.c                # 协议转换主程序
├── osal/                      # 操作系统抽象层
│   ├── inc/                   # OSAL头文件
│   └── linux/                 # Linux实现
├── hal/                       # 硬件抽象层
│   ├── inc/                   # HAL头文件
│   └── linux/                 # Linux实现
├── service/                   # 服务层
│   ├── inc/                   # Service头文件
│   └── linux/                 # Linux实现
├── config/                    # 配置文件
├── tests/                     # 单元测试
├── docs/                      # 文档
├── examples/                  # 示例代码
├── build.sh                   # 构建脚本
└── CMakeLists.txt            # CMake配置
```

## 架构层次

### 1. 应用层 (apps/)
- **can_gateway**: 与卫星平台通过CAN总线通信的应用程序
  - 接收卫星平台命令
  - 发送响应和状态上报
  - 维护心跳
  
- **protocol_converter**: 与载荷通信的应用程序
  - 通过IPMI/Redfish协议与载荷通信
  - 协议转换和命令执行
  - 支持网络和串口备份通道

### 2. 服务层 (service/)
提供高级服务功能：
- 载荷服务 (BMC/Linux)
- 电源管理
- 卫星服务
- 看门狗

### 3. 硬件抽象层 (hal/)
提供硬件接口抽象：
- CAN总线驱动
- 网络接口
- 串口接口

### 4. 操作系统抽象层 (osal/)
提供操作系统接口抽象：
- 任务管理
- 消息队列
- 互斥锁
- 文件I/O
- 网络通信
- 信号处理

## 构建系统

### 构建所有应用和库
```bash
./build.sh
```

### 构建特定应用
```bash
./build.sh --target can_gateway          # 仅构建CAN网关
./build.sh --target protocol_converter   # 仅构建协议转换器
```

### 构建特定库
```bash
./build.sh --target osal                 # 仅构建OSAL库
./build.sh --target hal                  # 仅构建HAL库
./build.sh --target service              # 仅构建Service库
```

### Debug模式构建
```bash
./build.sh -d
```

### 构建并运行测试
```bash
./build.sh -d -t
```

### 代码覆盖率
```bash
./build.sh -d --coverage -t
```

### 清理构建
```bash
./build.sh -c
```

## 输出文件

构建完成后，输出文件位于 `build/` 目录：

```
build/
├── bin/                       # 可执行文件
│   ├── can_gateway           # CAN网关应用
│   └── protocol_converter    # 协议转换应用
└── lib/                       # 库文件
    ├── libosal.a
    ├── libhal.a
    ├── libservice.a
    ├── libcan_gateway_lib.a
    └── libprotocol_converter_lib.a
```

## 运行应用

### 运行CAN网关
```bash
./build/bin/can_gateway
```

### 运行协议转换器
```bash
./build/bin/protocol_converter
```

## 测试

### 运行所有测试
```bash
cd build
ctest --output-on-failure
```

### 运行特定测试
```bash
cd build
./bin/test_os_task
./bin/test_can_gateway
```

## 交叉编译

### 编译到通用Linux平台
```bash
./build.sh --platform generic-linux
```

## 设计原则

1. **模块化**: 每个应用程序独立，可单独构建和运行
2. **分层架构**: 应用层 → 服务层 → HAL层 → OSAL层
3. **可测试性**: 每层都有对应的单元测试
4. **可移植性**: 通过OSAL和HAL抽象，支持多平台
5. **灵活构建**: 支持选择性构建特定模块或应用

## 依赖关系

```
can_gateway
  └── osal
  └── hal

protocol_converter
  └── can_gateway_lib
  └── service
  └── hal
  └── osal
```
