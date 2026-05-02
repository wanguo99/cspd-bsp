# BSP

板级支持包 (Board Support Package)

## 项目简介

BSP 是专为卫星算存载荷管理控制器设计的板级支持包。

**系统架构**：
```
卫星平台 <--CAN--> 转接板(BSP) <--Ethernet/UART--> 算存载荷
```

## 快速开始

### 编译

**本地编译**：
```bash
./build.sh              # Release模式
./build.sh -d           # Debug模式
./build.sh -c           # 清理
```

**交叉编译**（支持 ARM32/ARM64/RISC-V 64）：
```bash
# ARM32 (ARMv7-A)
./build.sh -a arm32

# ARM64 (ARMv8-A)
./build.sh -a arm64

# RISC-V 64
./build.sh -a riscv64

# 安装交叉编译工具链 (Ubuntu/Debian)
sudo apt-get install gcc-arm-linux-gnueabihf      # ARM32
sudo apt-get install gcc-aarch64-linux-gnu        # ARM64
sudo apt-get install gcc-riscv64-linux-gnu        # RISC-V 64
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

BSP采用5层分层架构：

### OSAL - 操作系统抽象层
跨平台的操作系统抽象接口，提供任务、队列、互斥锁、日志等基础服务，所有标准库函数、系统调用都应该在此进行封装。

**特性**：用户态库设计、线程安全、优雅关闭、死锁检测、日志轮转

**文档**: [osal/README.md](osal/README.md) | [详细文档](osal/docs/)

### HAL - 硬件抽象层
硬件驱动封装，提供CAN、串口、I2C、SPI等硬件接口，只允许PDL访问该库。

**特性**：平台隔离、统一接口、驱动封装、配置管理

**支持硬件**：CAN总线、串口(UART)、I2C总线、SPI总线

**文档**: [hal/README.md](hal/README.md) | [详细文档](hal/docs/)

### PDL - 外设驱动层
统一管理卫星平台、BMC载荷、MCU等外设服务，对应用提供访问外设的统一接口，，只允许APP和TEST层访问该库的接口。。

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

### Tests - 测试框架
统一测试框架，提供跨层级的单元测试和集成测试能力。

**特性**：统一框架、交互式菜单、命令行模式、自动注册、详细报告

**文档**: [tests/README.md](tests/README.md)

## 目录结构

```
bsp/
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
├── pcl/                # 硬件配置库
│   ├── include/           # 配置接口
│   ├── platform/          # 平台配置
│   └── docs/              # 文档
├── apps/                   # 应用层
│   ├── sample_app/        # 示例应用
│   └── docs/              # 文档
├── tests/                  # 测试框架
│   ├── include/           # 测试框架头文件
│   ├── core/              # 测试框架核心
│   ├── osal/              # OSAL层测试
│   ├── hal/               # HAL层测试
│   ├── pcl/               # PCL层测试
│   ├── pdl/               # PDL层测试
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
- **多架构支持**：支持 x86_64、ARM32、ARM64、RISC-V 64
  - 固定大小类型（`uint32_t`、`int64_t`）保证跨架构一致性
  - 自动字节序检测和转换（`OSAL_HTONS/HTONL`）
  - C11 原子操作支持所有架构
- **模块化架构**：5层分层，职责清晰
- **完整测试**：142+测试用例，覆盖核心功能
  - OSAL层：50+测试用例
  - HAL层：72测试用例（CAN、串口、I2C、SPI）
  - PCL层：5+测试用例
  - PDL层：15+测试用例
- **丰富驱动**：支持CAN、串口、I2C、SPI等常用总线
- **详细文档**：每个模块都有完整的架构、API、使用文档

## 文档导航

### 模块文档
- [OSAL层](osal/README.md) - 操作系统抽象层（任务、队列、互斥锁、日志等）
- [HAL层](hal/README.md) - 硬件抽象层（CAN、串口等驱动）
- [PDL层](pdl/README.md) - 外设驱动层（卫星平台、BMC载荷、MCU服务）
- [PCL层](pcl/README.md) - 硬件配置库（设备树式配置管理）
- [Apps层](apps/README.md) - 应用层（示例应用）
- [Tests层](tests/README.md) - 测试框架（70+测试用例）

### 详细文档
- [OSAL详细文档](osal/docs/) - API参考、设计文档
- [HAL详细文档](hal/docs/) - 驱动开发、移植指南
- [PDL详细文档](pdl/docs/) - 服务设计、协议文档
- [PCL详细文档](pcl/docs/) - 配置规范、平台适配
- [Apps详细文档](apps/docs/) - 应用开发指南
- [Tests详细文档](tests/docs/) - 测试框架、测试指南

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
./output/target/bin/unit-test -m test_hal_i2c
./output/target/bin/unit-test -m test_hal_spi
```

**测试覆盖**：
- OSAL层：10模块，50+测试用例
- HAL层：4模块，72测试用例（CAN、串口、I2C、SPI）
- PCL层：1模块，5+测试用例
- PDL层：3模块，15+测试用例
- 总计：142+测试用例

## 开发环境

- **操作系统**：Linux (Ubuntu 20.04+)
- **编译器**：GCC 9.0+
- **构建工具**：CMake 3.16+
- **依赖库**：pthread, rt

## 代码规模

- 总代码量：约22,000行
- 生产代码：约15,500行
- 测试代码：约4,500行
- 代码文件：116个（.c/.h）

## 设计原则

1. **平台无关性**：OSAL是唯一允许包含系统头文件的层
2. **分层隔离**：上层只能调用下层接口，不能跨层调用
3. **配置分离**：硬件配置集中在PCL层
4. **错误处理**：所有函数返回int32状态码
5. **资源管理**：所有资源必须显式释放

## 许可证

本项目采用 GNU General Public License v3.0 许可证。详见 [LICENSE](LICENSE) 文件。

## 贡献

欢迎贡献代码和反馈！请遵循以下规范：

1. 阅读 [编码规范](docs/CODING_STANDARDS.md)
2. 提交前运行测试：`./output/target/bin/unit-test -a`
3. 确保代码通过编译：`./build.sh`
4. 提交清晰的 commit message

## 联系方式

- 问题反馈：请通过 GitHub Issues 提交
- 代码贡献：请通过 Pull Request 提交
