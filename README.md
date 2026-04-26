# PMC-BSP

载荷管理控制器板级支持包 (Payload Management Controller Board Support Package)

## 项目简介

PMC-BSP 是专为卫星算存载荷设计的板级支持包，作为卫星平台与算存载荷之间的通信桥接和管理中间层。

**系统架构**：
```
卫星平台 <--CAN--> 转接板(PMC-BSP) <--Ethernet/UART--> 算存载荷
```

## 快速开始

### 编译

```bash
./build.sh              # Release模式
./build.sh -d           # Debug模式
./build.sh -c           # 清理
```

### 运行

```bash
# 运行示例应用
./output/target/bin/sample_app

# 运行测试
./output/target/bin/unit-test -i    # 交互式菜单
./output/target/bin/unit-test -a    # 运行所有测试
```

## 模块组成

PMC-BSP采用5层分层架构：

### OSAL - 操作系统抽象层
跨平台的操作系统抽象接口，提供任务、队列、互斥锁、日志等基础服务。

**特性**：用户态库设计、线程安全、优雅关闭、死锁检测、日志轮转

**文档**: [osal/README.md](osal/README.md) | [详细文档](osal/docs/)

### HAL - 硬件抽象层
硬件驱动封装，提供CAN、串口等硬件接口。

**特性**：平台隔离、统一接口、驱动封装、配置管理

**文档**: [hal/README.md](hal/README.md) | [详细文档](hal/docs/)

### PDL - 外设驱动层
统一管理卫星平台、BMC载荷、MCU等外设服务。

**特性**：统一外设管理、多通道冗余、自动故障切换、心跳机制

**文档**: [pdl/README.md](pdl/README.md) | [详细文档](pdl/docs/)

### XConfig - 硬件配置库
类似设备树的硬件配置管理，以外设为单位组织配置。

**特性**：外设为单位、配置与代码分离、接口内嵌、运行时查询

**文档**: [xconfig/README.md](xconfig/README.md) | [详细文档](xconfig/docs/)

### Apps - 应用层
示例应用，展示如何使用BSP接口。

**特性**：平台无关、使用抽象接口、优雅退出、错误处理

**文档**: [apps/README.md](apps/README.md) | [详细文档](apps/docs/)

## 目录结构

```
pmc-bsp/
├── osal/                   # 操作系统抽象层
│   ├── include/           # 公共头文件
│   ├── src/posix/         # POSIX实现
│   └── docs/              # 文档
├── hal/                    # 硬件抽象层
│   ├── include/           # 驱动接口
│   ├── src/linux/         # Linux实现
│   └── docs/              # 文档
├── pdl/                    # 外设驱动层
│   ├── include/           # 服务接口
│   ├── src/               # 服务实现
│   └── docs/              # 文档
├── xconfig/                # 硬件配置库
│   ├── include/           # 配置接口
│   ├── platform/          # 平台配置
│   └── docs/              # 文档
├── apps/                   # 应用层
│   ├── sample_app/        # 示例应用
│   └── docs/              # 文档
├── tests/                  # 测试框架
│   ├── src/               # 测试代码
│   └── docs/              # 文档
├── output/                 # 编译输出
│   ├── build/             # 中间文件
│   └── target/            # 最终产物
└── docs/                   # 项目文档
    ├── ARCHITECTURE.md    # 架构设计
    ├── CODING_STANDARDS.md # 编码规范
    └── CLAUDE.md          # 开发指南
```

## 核心特性

- **跨平台设计**：POSIX实现，易于移植到RTOS
- **模块化架构**：5层分层，职责清晰
- **完整测试**：70+测试用例，覆盖核心功能
- **详细文档**：每个模块都有完整的架构、API、使用文档

## 文档导航

### 模块文档
- [OSAL层](osal/README.md) - 操作系统抽象层（任务、队列、互斥锁、日志等）
- [HAL层](hal/README.md) - 硬件抽象层（CAN、串口等驱动）
- [PDL层](pdl/README.md) - 外设驱动层（卫星平台、BMC载荷、MCU服务）
- [XConfig层](xconfig/README.md) - 硬件配置库（设备树式配置管理）
- [Apps层](apps/README.md) - 应用层（示例应用）
- [Tests层](tests/README.md) - 测试框架（70+测试用例）

### 详细文档
- [OSAL详细文档](osal/docs/) - API参考、设计文档
- [HAL详细文档](hal/docs/) - 驱动开发、移植指南
- [PDL详细文档](pdl/docs/) - 服务设计、协议文档
- [XConfig详细文档](xconfig/docs/) - 配置规范、平台适配
- [Apps详细文档](apps/docs/) - 应用开发指南

### 项目文档
- [架构设计](docs/ARCHITECTURE.md) - 系统架构和设计理念
- [编码规范](docs/CODING_STANDARDS.md) - 代码风格和规范
- [开发指南](CLAUDE.md) - 开发者指南

## 测试

```bash
# 交互式测试菜单
./output/target/bin/unit-test -i

# 运行所有测试
./output/target/bin/unit-test -a

# 运行指定层测试
./output/target/bin/unit-test -L OSAL
./output/target/bin/unit-test -L HAL

# 运行指定模块测试
./output/target/bin/unit-test -m test_osal_task
./output/target/bin/unit-test -m test_hal_can
```

**测试覆盖**：
- OSAL层：6模块，60测试用例
- HAL层：1模块，3测试用例
- PDL层：1模块，2测试用例
- 总计：70+测试用例

## 开发环境

- **操作系统**：Linux (Ubuntu 20.04+)
- **编译器**：GCC 9.0+
- **构建工具**：CMake 3.16+
- **依赖库**：pthread, rt

## 代码规模

- 总代码量：约18,000行
- 生产代码：约14,000行
- 测试代码：约4,000行
- 代码文件：97个（.c/.h）

## 设计原则

1. **平台无关性**：OSAL是唯一允许包含系统头文件的层
2. **分层隔离**：上层只能调用下层接口，不能跨层调用
3. **配置分离**：硬件配置集中在XConfig层
4. **错误处理**：所有函数返回int32状态码
5. **资源管理**：所有资源必须显式释放

## 许可证

本项目采用 [许可证名称] 许可证。

## 贡献

欢迎提交Issue和Pull Request。

## 联系方式

- 项目主页：[项目URL]
- 问题反馈：[Issue URL]
