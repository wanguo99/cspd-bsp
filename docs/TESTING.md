# CSPD-BSP 测试系统使用指南

## 概述

CSPD-BSP 提供了统一的测试入口程序，支持交互式菜单和命令行参数，可以方便地运行所有层的测试。

## 测试层次结构

```
CSPD-BSP Test Suite
├── OSAL Layer (6 modules)
│   ├── test_os_task      - 任务管理测试
│   ├── test_os_queue     - 消息队列测试
│   ├── test_os_mutex     - 互斥锁测试
│   ├── test_os_file      - 文件I/O测试
│   ├── test_os_network   - 网络通信测试
│   └── test_os_signal    - 信号处理测试
├── HAL Layer (1 module)
│   └── test_hal_can      - CAN驱动测试
├── Service Layer (1 module)
│   └── test_payload_service - 载荷服务测试
└── Apps Layer (2 modules)
    ├── test_can_gateway        - CAN网关测试
    └── test_protocol_converter - 协议转换测试
```

## 构建测试

### 构建所有测试
```bash
./build.sh -d
```

### 仅构建测试（不构建应用）
```bash
./build.sh -d --target all_tests
```

## 运行测试

### 方式1：统一测试入口（推荐）

统一测试入口程序 `unit-test` 整合了所有层的测试，支持交互式和命令行两种模式。

#### 交互式模式
```bash
cd build
./bin/unit-test
# 或
./bin/unit-test -i
```

交互式菜单提供以下选项：
```
1. Run all tests (all layers)        - 运行所有测试
2. Run OSAL layer tests              - 运行OSAL层测试
3. Run HAL layer tests               - 运行HAL层测试
4. Run Service layer tests           - 运行Service层测试
5. Run Apps layer tests              - 运行Apps层测试
6. Run specific module tests         - 运行指定模块测试
7. Run single test                   - 运行单个测试
8. List all tests                    - 列出所有测试
0. Exit                              - 退出
```

#### 命令行模式

**运行所有测试**
```bash
./bin/unit-test -a
```

**运行指定层的测试**
```bash
./bin/unit-test -L OSAL      # 运行OSAL层所有测试
./bin/unit-test -L HAL       # 运行HAL层所有测试
./bin/unit-test -L Service   # 运行Service层所有测试
./bin/unit-test -L Apps      # 运行Apps层所有测试
```

**运行指定模块的测试**
```bash
./bin/unit-test -m test_os_file          # 运行文件I/O测试
./bin/unit-test -m test_hal_can          # 运行CAN驱动测试
./bin/unit-test -m test_can_gateway      # 运行CAN网关测试
```

**运行单个测试用例**
```bash
./bin/unit-test -t test_os_file test_OS_FileOpen_Close_Success
./bin/unit-test -t test_os_task test_OS_TaskCreate_Success
```

**列出所有测试**
```bash
./bin/unit-test -l
```

**查看帮助**
```bash
./bin/unit-test -h
```

### 方式2：使用 Make 目标

**运行所有测试（使用CTest）**
```bash
cd build
make run_tests
```

**运行统一测试套件**
```bash
cd build
make run_unified
```

### 方式3：使用 CTest

**运行所有测试**
```bash
cd build
ctest
```

**详细输出**
```bash
cd build
ctest --output-on-failure
```

**运行特定测试**
```bash
cd build
ctest -R test_os_task        # 运行名称匹配的测试
ctest -V                     # 详细模式
```

### 方式4：直接运行独立测试程序

每个测试模块都可以独立运行：

```bash
cd build/bin

# OSAL层测试
./test_os_task
./test_os_queue
./test_os_mutex
./test_os_file
./test_os_network
./test_os_signal

# HAL层测试
./test_hal_can

# Service层测试
./test_payload_service

# Apps层测试
./test_can_gateway
./test_protocol_converter
```

## 代码覆盖率

### 生成覆盖率报告
```bash
./build.sh -d --coverage -t
```

### 查看覆盖率报告
```bash
firefox build/coverage_html/index.html
```

## 测试输出示例

### 成功的测试输出
```
========================================
Running ALL Tests
========================================

>>> Testing Layer: OSAL

========================================
Running module: test_os_file
Total tests: 10
========================================

[RUN ] test_os_file::test_OS_FileOpen_Close_Success
[PASS] test_os_file::test_OS_FileOpen_Close_Success
...

========================================
Test Summary:
  Total:  67
  Passed: 67
  Failed: 0

[SUCCESS] All tests passed!
========================================
```

### 失败的测试输出
```
[RUN ] test_os_file::test_OS_FileOpen_InvalidPath
[  FAILED  ] test_os_file.c:45: Expected -1, got 0
[FAIL] test_os_file::test_OS_FileOpen_InvalidPath

========================================
Test Summary:
  Total:  67
  Passed: 66
  Failed: 1

[FAILURE] 1 test(s) failed!
========================================
```

## 最佳实践

1. **优先使用统一测试入口** - `unit-test` 提供了最完整的测试体验
2. **交互模式用于调试** - 方便快速定位和重复运行特定测试
3. **命令行模式用于CI/CD** - 适合自动化测试流程
4. **定期运行覆盖率测试** - 确保测试覆盖率
5. **测试命名规范** - 使用 `test_<Module>_<Scenario>` 格式

## 持续集成

在CI/CD流程中使用测试：

```bash
# 构建并运行所有测试
./build.sh -d -t

# 或者分步执行
./build.sh -d
cd build
./bin/unit-test -a

# 检查退出码
if [ $? -eq 0 ]; then
    echo "All tests passed"
else
    echo "Tests failed"
    exit 1
fi
```
