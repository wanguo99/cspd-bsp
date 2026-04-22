# CSPD-BSP 单元测试快速开始指南

## 概述

本测试系统**无需外部依赖**，使用纯C实现的轻量级测试框架。

## 测试统计

- **总测试用例**: 58个
- **OSAL层**: 27个 (任务、队列、互斥锁)
- **HAL层**: 10个 (CAN驱动)
- **Service层**: 10个 (载荷服务)
- **Apps层**: 11个 (CAN网关)

## 环境要求

### Linux环境

```bash
# 必需工具
sudo apt-get install gcc make

# 可选：虚拟CAN接口（用于CAN测试）
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link add dev vcan1 type vcan
sudo ip link set up vcan0
sudo ip link set up vcan1
```

### Windows环境

有以下几种选择：

#### 选项1: WSL (Windows Subsystem for Linux) - 推荐

```bash
# 在PowerShell中启用WSL
wsl --install

# 进入WSL后安装工具
sudo apt-get update
sudo apt-get install gcc make

# 进入项目目录
cd /mnt/z/cspd-bsp/tests
make test
```

#### 选项2: MinGW-w64

```bash
# 下载并安装MinGW-w64
# https://www.mingw-w64.org/downloads/

# 添加到PATH后
cd z:\cspd-bsp\tests
make test
```

#### 选项3: Cygwin

```bash
# 下载并安装Cygwin
# https://www.cygwin.com/

# 安装时选择gcc和make包
cd /cygdrive/z/cspd-bsp/tests
make test
```

#### 选项4: Visual Studio (使用MSVC)

需要修改Makefile以使用cl.exe编译器，或使用CMake生成VS项目。

## 快速开始

### 1. 构建所有测试

```bash
cd tests
make all
```

### 2. 运行所有测试

```bash
make test
```

### 3. 运行单个测试

```bash
# OSAL测试
./test_os_task
./test_os_queue
./test_os_mutex

# HAL测试
./test_hal_can

# Service测试
./test_payload_service

# Apps测试
./test_can_gateway
```

### 4. 清理构建产物

```bash
make clean
```

## 测试输出示例

```
========================================
Starting Test Suite
========================================

[ RUN      ] test_OS_TaskCreate_Success
[       OK ] test_OS_TaskCreate_Success
[ RUN      ] test_OS_TaskCreate_NullPointer
[       OK ] test_OS_TaskCreate_NullPointer
...

========================================
Test Results:
  Total:  13
  Passed: 13
  Failed: 0
========================================
```

## 测试框架特性

我们的自定义测试框架提供：

- ✅ 零外部依赖
- ✅ 彩色输出
- ✅ 详细的失败信息
- ✅ 测试统计
- ✅ 断言宏：
  - `TEST_ASSERT(condition)`
  - `TEST_ASSERT_EQUAL(expected, actual)`
  - `TEST_ASSERT_NOT_EQUAL(expected, actual)`
  - `TEST_ASSERT_NULL(ptr)`
  - `TEST_ASSERT_NOT_NULL(ptr)`
  - `TEST_ASSERT_TRUE(condition)`
  - `TEST_ASSERT_FALSE(condition)`
  - `TEST_ASSERT_GREATER_OR_EQUAL(threshold, actual)`
  - `TEST_ASSERT_STRING_EQUAL(expected, actual)`

## 故障排查

### 问题1: 编译器未找到

```bash
# 检查编译器
which gcc

# 如果未找到，安装gcc
sudo apt-get install gcc  # Ubuntu/Debian
sudo yum install gcc       # CentOS/RHEL
```

### 问题2: 找不到头文件

确保您在`tests`目录下运行make命令。Makefile会自动包含所有必要的头文件路径。

### 问题3: 链接错误

```bash
# 确保pthread库可用
sudo apt-get install libpthread-stubs0-dev
```

### 问题4: vcan接口不存在

某些测试需要虚拟CAN接口。如果没有，这些测试会被自动跳过（标记为IGNORED）。

```bash
# 创建虚拟CAN接口
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0
```

### 问题5: Windows上无法运行

Windows原生不支持POSIX API。请使用WSL、MinGW或Cygwin。

## 添加新测试

### 1. 创建测试文件

```c
#include "../test_framework.h"
#include "your_module.h"

void test_YourFunction_Success(void)
{
    int result = YourFunction(valid_input);
    TEST_ASSERT_EQUAL(EXPECTED_VALUE, result);
}

void test_YourFunction_NullPointer(void)
{
    int result = YourFunction(NULL);
    TEST_ASSERT_EQUAL(ERROR_CODE, result);
}

int main(void)
{
    TEST_BEGIN();
    
    RUN_TEST(test_YourFunction_Success);
    RUN_TEST(test_YourFunction_NullPointer);
    
    TEST_END();
}
```

### 2. 更新Makefile

在`Makefile`中添加新的测试目标：

```makefile
test_your_module: your_layer/test_your_module.c $(YOUR_SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)
```

并将其添加到`TESTS`变量中。

## 持续集成

### GitHub Actions示例

```yaml
name: Unit Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      
      - name: Install dependencies
        run: sudo apt-get install -y gcc make
        
      - name: Setup vcan
        run: |
          sudo modprobe vcan
          sudo ip link add dev vcan0 type vcan
          sudo ip link add dev vcan1 type vcan
          sudo ip link set up vcan0
          sudo ip link set up vcan1
          
      - name: Build and test
        run: |
          cd tests
          make test
```

## 性能测试

测试框架本身非常轻量：

- 编译时间: < 5秒
- 运行时间: < 10秒（所有58个测试）
- 内存占用: < 10MB

## 下一步

1. **运行测试**: `make test`
2. **查看覆盖率**: 使用gcov/lcov生成覆盖率报告
3. **添加测试**: 为新功能添加测试用例
4. **集成CI**: 将测试集成到CI/CD流程

## 支持

如有问题，请查看：
- [README.md](README.md) - 完整文档
- [test_framework.h](test_framework.h) - 测试框架API
- [Makefile](Makefile) - 构建配置

## 许可

本测试框架为项目内部使用，无需额外许可。
