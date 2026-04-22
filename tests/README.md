# CSPD-BSP 单元测试指南

## 概述

本文档描述了CSPD-BSP嵌入式软件框架的单元测试体系，包括测试框架、测试用例、执行方法和覆盖率分析。

## 测试框架

- **测试框架**: 自研轻量级测试框架 (test_framework.h)
- **构建系统**: Makefile
- **覆盖率工具**: gcov + lcov (可选)
- **特点**: 零外部依赖，纯C实现

## 测试结构

```
tests/
├── Makefile                 # 构建配置
├── test_framework.h         # 自定义测试框架
├── README.md                # 本文档
├── QUICKSTART.md            # 快速开始指南
├── SUMMARY.md               # 项目总结
├── osal/                    # OSAL层测试
│   ├── test_os_task.c       # 任务管理测试 (13个用例)
│   ├── test_os_queue.c      # 消息队列测试 (13个用例)
│   └── test_os_mutex.c      # 互斥锁测试 (11个用例)
├── hal/                     # HAL层测试
│   └── test_hal_can.c       # CAN驱动测试 (10个用例)
├── service/                 # Service层测试
│   └── test_payload_service.c  # 载荷服务测试 (10个用例)
└── apps/                    # Apps层测试
    └── test_can_gateway.c   # CAN网关测试 (11个用例)
```

## 测试覆盖范围

### OSAL层 (37个测试用例)
- **test_os_task.c**: 任务创建、删除、延时、优先级设置、信息获取
- **test_os_queue.c**: 队列创建、发送、接收、超时、满/空状态、删除
- **test_os_mutex.c**: 互斥锁创建、加锁、解锁、线程安全保护

### HAL层 (10个测试用例)
- **test_hal_can.c**: CAN初始化、发送、接收、过滤器、统计信息、回环测试

### Service层 (10个测试用例)
- **test_payload_service.c**: 载荷服务初始化、发送、接收、通道切换、连接状态

### Apps层 (11个测试用例)
- **test_can_gateway.c**: CAN网关初始化、启动、停止、消息转发、统计信息

**总计**: 58个单元测试用例

## 环境准备

### Linux环境

```bash
# 安装编译工具
sudo apt-get install gcc make

# 安装覆盖率工具 (可选)
sudo apt-get install lcov
```

### Windows环境

推荐使用以下方案之一：

#### 方案1: WSL (推荐)
```bash
# 在PowerShell中启用WSL
wsl --install

# 进入WSL后
sudo apt-get update
sudo apt-get install gcc make
```

#### 方案2: MinGW-w64
下载并安装 MinGW-w64，添加到系统PATH

#### 方案3: Cygwin
下载并安装 Cygwin，选择gcc和make包

### 配置虚拟CAN接口 (用于CAN测试)

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
ifconfig vcan1
```

## 构建测试

```bash
# 进入测试目录
cd tests

# 构建所有测试
make all

# 构建单个测试
make test_os_task
make test_hal_can
```

## 运行测试

### 运行所有测试

```bash
# 构建并运行所有测试
make test
```

### 运行单个测试

```bash
# 直接运行测试可执行文件
./test_os_task
./test_os_queue
./test_os_mutex
./test_hal_can
./test_payload_service
./test_can_gateway

# 或使用make
make run-test_os_task
```

### 测试输出示例

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

## 代码覆盖率分析

### 生成覆盖率报告

```bash
# 使用覆盖率标志编译
gcc -fprofile-arcs -ftest-coverage -o test_os_task osal/test_os_task.c ../osal/linux/*.c -I../osal/inc -lpthread

# 运行测试
./test_os_task

# 生成覆盖率数据
gcov test_os_task.c

# 使用lcov生成HTML报告
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_html

# 查看报告
firefox coverage_html/index.html
```

### 覆盖率目标

- **OSAL层**: 90%+ (核心抽象层，要求高覆盖率)
- **HAL层**: 80%+ (硬件相关，部分代码依赖实际硬件)
- **Service层**: 75%+ (业务逻辑层)
- **Apps层**: 70%+ (应用层，部分代码依赖外部系统)

## 测试框架API

### 测试控制宏

```c
TEST_BEGIN()              // 开始测试套件
TEST_END()                // 结束测试套件并返回结果
RUN_TEST(test_func)       // 运行单个测试用例
```

### 断言宏

```c
TEST_ASSERT(condition)                          // 断言条件为真
TEST_ASSERT_TRUE(condition)                     // 断言为真
TEST_ASSERT_FALSE(condition)                    // 断言为假
TEST_ASSERT_EQUAL(expected, actual)             // 断言相等
TEST_ASSERT_NOT_EQUAL(expected, actual)         // 断言不相等
TEST_ASSERT_NULL(ptr)                           // 断言指针为空
TEST_ASSERT_NOT_NULL(ptr)                       // 断言指针非空
TEST_ASSERT_GREATER_OR_EQUAL(threshold, actual) // 断言大于等于
TEST_ASSERT_STRING_EQUAL(expected, actual)      // 断言字符串相等
```

### 辅助宏

```c
TEST_MESSAGE(msg)           // 输出信息消息
TEST_IGNORE()               // 忽略当前测试
TEST_IGNORE_MESSAGE(msg)    // 忽略测试并输出消息
```

## 测试用例说明

### 正常功能测试
- 验证API在正常输入下的行为
- 验证返回值和输出参数的正确性

### 边界条件测试
- 空指针参数
- 无效ID/句柄
- 超出范围的参数值
- 资源耗尽情况

### 错误处理测试
- 验证错误码的正确性
- 验证错误恢复机制
- 验证资源清理

### 并发测试
- 多线程访问共享资源
- 互斥锁保护验证
- 竞态条件检测

### 集成测试
- 跨层接口调用
- 端到端数据流
- 实际硬件交互 (需要硬件环境)

## 测试最佳实践

### 1. 测试独立性
- 每个测试用例应该独立运行
- 在测试函数内部调用setUp()和tearDown()
- 不依赖其他测试的执行顺序

### 2. 测试可重复性
- 测试结果应该是确定的
- 避免依赖系统时间、随机数等不确定因素
- 使用Mock对象隔离外部依赖

### 3. 测试可读性
- 使用清晰的测试用例名称
- 添加必要的注释说明测试意图
- 使用有意义的断言消息

### 4. 测试覆盖率
- 优先覆盖关键路径
- 覆盖错误处理分支
- 覆盖边界条件

## 持续集成

### CI/CD集成示例 (GitHub Actions)

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
          sudo apt-get install -y gcc make lcov
          
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
          
      - name: Generate coverage
        run: |
          cd tests
          # 添加覆盖率生成命令
```

## 故障排查

### 问题1: 编译器未找到

```bash
# 检查编译器
which gcc

# 安装gcc
sudo apt-get install gcc  # Ubuntu/Debian
sudo yum install gcc       # CentOS/RHEL
```

### 问题2: vcan接口不存在

```bash
# 检查vcan模块
lsmod | grep vcan

# 如果未加载，加载模块
sudo modprobe vcan

# 创建接口
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0
```

### 问题3: 测试超时

某些测试可能因为等待超时而失败，这通常是正常的（例如测试超时机制）。检查测试日志确认是否是预期行为。

### 问题4: 权限问题

某些测试可能需要root权限（例如创建网络接口）：

```bash
# 使用sudo运行测试
sudo ./test_hal_can
```

### 问题5: 链接错误

```bash
# 确保pthread库可用
sudo apt-get install libpthread-stubs0-dev
```

## 扩展测试

### 添加新测试用例

1. 在相应目录创建测试文件 `test_xxx.c`
2. 包含测试框架头文件和被测模块头文件
3. 实现setUp()和tearDown()函数
4. 编写测试用例函数 (以test_开头)
5. 在main()中使用TEST_BEGIN()和TEST_END()
6. 更新Makefile添加新测试

### 示例

```c
#include "../test_framework.h"
#include "your_module.h"

void setUp(void) {
    // 初始化代码
    OS_API_Init();
}

void tearDown(void) {
    // 清理代码
    OS_API_Teardown();
}

void test_YourFunction_Success(void) {
    setUp();
    int result = YourFunction(valid_input);
    TEST_ASSERT_EQUAL(EXPECTED_VALUE, result);
    tearDown();
}

void test_YourFunction_NullPointer(void) {
    setUp();
    int result = YourFunction(NULL);
    TEST_ASSERT_EQUAL(ERROR_CODE, result);
    tearDown();
}

int main(void) {
    TEST_BEGIN();
    RUN_TEST(test_YourFunction_Success);
    RUN_TEST(test_YourFunction_NullPointer);
    TEST_END();
}
```

### 更新Makefile

```makefile
# 添加新测试
test_your_module: your_layer/test_your_module.c $(YOUR_SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)

# 添加到TESTS变量
TESTS = ... test_your_module
```

## 清理

```bash
# 清理所有构建产物
make clean
```

## 参考资料

- [test_framework.h](test_framework.h) - 测试框架API文档
- [QUICKSTART.md](QUICKSTART.md) - 快速开始指南
- [SUMMARY.md](SUMMARY.md) - 项目总结

## 联系方式

如有问题或建议，请联系开发团队。
