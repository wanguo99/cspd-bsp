# CSPD-BSP 测试集成说明

## 概述

CSPD-BSP项目已完全集成单元测试到CMake构建系统中。测试使用自研的轻量级测试框架，无需外部依赖。

## 测试统计

- **总测试用例**: 58个
- **OSAL层**: 37个（任务、队列、互斥锁）
- **HAL层**: 10个（CAN驱动）
- **Service层**: 10个（载荷服务）
- **Apps层**: 11个（CAN网关）

## 快速开始

### 方法1: 使用test.sh脚本（推荐）

```bash
# 构建并运行所有测试
./test.sh -r

# 生成覆盖率报告
./test.sh --coverage

# 仅运行OSAL层测试
./test.sh -r --osal

# 清理后重新构建并运行
./test.sh -c -r
```

### 方法2: 使用build.sh脚本

```bash
# 编译并运行测试
./build.sh -d -t

# 启用覆盖率
./build.sh -d --coverage -t

# 禁用测试编译
./build.sh --no-tests
```

### 方法3: 手动使用CMake

```bash
# 配置（启用测试）
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON ..

# 构建所有测试
make all_tests

# 运行测试
ctest --output-on-failure

# 或使用make
make run_tests
```

## 测试选项

### CMake选项

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `BUILD_TESTING` | ON | 是否构建测试 |
| `ENABLE_COVERAGE` | OFF | 是否启用代码覆盖率 |

### 示例

```bash
# 禁用测试
cmake -DBUILD_TESTING=OFF ..

# 启用覆盖率
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON ..
make all_tests
make coverage
```

## 运行测试

### 运行所有测试

```bash
cd build
ctest --output-on-failure
```

### 运行特定层的测试

```bash
# OSAL层
ctest -R "test_os_*"

# HAL层
ctest -R "test_hal_*"

# Service层
ctest -R "test_payload_*"

# Apps层
ctest -R "test_can_*"
```

### 运行单个测试

```bash
cd build
./test_os_task
./test_os_queue
./test_hal_can
```

### 详细输出

```bash
ctest -V
```

## 代码覆盖率

### 生成覆盖率报告

```bash
# 方法1: 使用test.sh
./test.sh --coverage

# 方法2: 使用CMake
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON ..
make all_tests
make coverage
```

### 查看报告

```bash
firefox build/coverage_html/index.html
```

### 覆盖率目标

- OSAL层: 90%+
- HAL层: 80%+
- Service层: 75%+
- Apps层: 70%+

## 测试环境准备

### 虚拟CAN接口（用于CAN测试）

```bash
# 加载vcan模块
sudo modprobe vcan

# 创建虚拟CAN接口
sudo ip link add dev vcan0 type vcan
sudo ip link add dev vcan1 type vcan

# 启动接口
sudo ip link set up vcan0
sudo ip link set up vcan1

# 验证
ifconfig vcan0
```

### 安装覆盖率工具（可选）

```bash
sudo apt-get install lcov
```

## 构建目标

### 测试相关目标

| 目标 | 说明 |
|------|------|
| `all_tests` | 构建所有测试 |
| `osal_tests` | 构建OSAL层测试 |
| `hal_tests` | 构建HAL层测试 |
| `service_tests` | 构建Service层测试 |
| `apps_tests` | 构建Apps层测试 |
| `run_tests` | 构建并运行所有测试 |
| `coverage` | 生成覆盖率报告（需要ENABLE_COVERAGE=ON） |

### 使用示例

```bash
cd build

# 构建所有测试
make all_tests

# 仅构建OSAL测试
make osal_tests

# 运行测试
make run_tests

# 生成覆盖率
make coverage
```

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
        run: |
          sudo apt-get update
          sudo apt-get install -y build-essential cmake lcov
          
      - name: Setup vcan
        run: |
          sudo modprobe vcan
          sudo ip link add dev vcan0 type vcan
          sudo ip link add dev vcan1 type vcan
          sudo ip link set up vcan0
          sudo ip link set up vcan1
          
      - name: Build and test
        run: |
          ./test.sh --coverage
          
      - name: Upload coverage
        uses: codecov/codecov-action@v2
        with:
          files: ./build/coverage.info.cleaned
```

## 测试框架

### 自定义测试框架

项目使用自研的轻量级测试框架（`tests/test_framework.h`），特点：

- ✅ 零外部依赖
- ✅ 纯C实现
- ✅ 彩色输出
- ✅ 详细错误信息
- ✅ 9种断言宏

### 断言宏

```c
TEST_ASSERT(condition)
TEST_ASSERT_EQUAL(expected, actual)
TEST_ASSERT_NOT_EQUAL(expected, actual)
TEST_ASSERT_NULL(ptr)
TEST_ASSERT_NOT_NULL(ptr)
TEST_ASSERT_TRUE(condition)
TEST_ASSERT_FALSE(condition)
TEST_ASSERT_GREATER_OR_EQUAL(threshold, actual)
TEST_ASSERT_STRING_EQUAL(expected, actual)
```

### 测试控制宏

```c
TEST_BEGIN()              // 开始测试套件
TEST_END()                // 结束测试套件
RUN_TEST(test_func)       // 运行测试用例
TEST_MESSAGE(msg)         // 输出信息
TEST_IGNORE()             // 忽略测试
```

## 添加新测试

### 1. 创建测试文件

在相应目录创建测试文件，例如 `tests/osal/test_new_feature.c`:

```c
#include "../test_framework.h"
#include "osal.h"

void setUp(void) {
    OS_API_Init();
}

void tearDown(void) {
    OS_API_Teardown();
}

void test_NewFeature_Success(void) {
    setUp();
    // 测试代码
    TEST_ASSERT_EQUAL(expected, actual);
    tearDown();
}

int main(void) {
    TEST_BEGIN();
    RUN_TEST(test_NewFeature_Success);
    TEST_END();
}
```

### 2. 更新CMakeLists.txt

在 `tests/CMakeLists.txt` 中添加：

```cmake
add_executable(test_new_feature
    osal/test_new_feature.c
    ${OSAL_SOURCES}
)
target_link_libraries(test_new_feature
    Threads::Threads
    rt
)
add_test(NAME test_new_feature COMMAND test_new_feature)
```

### 3. 重新构建

```bash
cd build
cmake ..
make all_tests
ctest -R test_new_feature
```

## 故障排查

### 问题1: vcan接口不存在

某些测试需要虚拟CAN接口，如果没有会被自动跳过（标记为IGNORED）。

```bash
# 创建虚拟CAN接口
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0
```

### 问题2: 测试超时

某些测试可能因为等待超时而失败，这通常是正常的（测试超时机制）。

### 问题3: 权限问题

某些测试可能需要root权限：

```bash
sudo ctest --output-on-failure
```

### 问题4: 覆盖率工具未找到

```bash
sudo apt-get install lcov
```

## 文档

- [tests/README.md](tests/README.md) - 测试详细文档
- [tests/QUICKSTART.md](tests/QUICKSTART.md) - 快速开始指南
- [tests/SUMMARY.md](tests/SUMMARY.md) - 测试总结
- [tests/test_framework.h](tests/test_framework.h) - 测试框架API

## 总结

CSPD-BSP的测试系统已完全集成到CMake构建流程中：

- ✅ 使用 `./test.sh -r` 快速运行测试
- ✅ 使用 `./build.sh -d -t` 在主构建中包含测试
- ✅ 使用 `--coverage` 选项生成覆盖率报告
- ✅ 支持分层测试和单个测试运行
- ✅ 零外部依赖，易于集成CI/CD

详细信息请参考 [tests/README.md](tests/README.md)。
