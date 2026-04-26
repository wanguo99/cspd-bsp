# Tests (测试框架)

## 模块概述

Tests模块提供统一的测试框架，支持OSAL、HAL、PDL等各层的单元测试。

**设计理念**：
- 参考NASA cFS ut_assert设计
- 统一测试入口，支持交互式菜单
- 三级选择：层级 → 模块 → 测试用例
- 完全平台无关，只使用OSAL接口

**测试覆盖**：
- OSAL层：任务、队列、互斥锁、信号
- HAL层：CAN驱动
- 总计：5个模块，50+测试用例

## 编译说明

### 快速开始

```bash
# 在项目根目录编译测试
./build.sh -d           # Debug模式（推荐用于测试）
./build.sh              # Release模式
```

### 单独编译测试模块

```bash
# 方法1: 使用CMake直接编译
mkdir -p output/build && cd output/build
cmake ../.. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
make unit-test -j$(nproc)
cd ../..

# 方法2: 在已配置的构建目录中编译
cd output/build
make unit-test -j$(nproc)
cd ../..
```

### 支持的编译参数

#### CMake配置参数

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `CMAKE_BUILD_TYPE` | STRING | Release | 编译类型：Release/Debug |
| `BUILD_TESTING` | BOOL | ON | 是否编译测试 |
| `ENABLE_COVERAGE` | BOOL | OFF | 是否启用代码覆盖率 |

#### 编译类型说明

**Debug模式**（推荐用于测试）：
```bash
cmake ../.. -DCMAKE_BUILD_TYPE=Debug
```
- 优化级别：`-O0`
- 包含调试信息：`-g`
- 便于调试测试失败

**Release模式**：
```bash
cmake ../.. -DCMAKE_BUILD_TYPE=Release
```
- 优化级别：`-O2`
- 无调试信息
- 测试性能

#### 控制测试编译

**禁用测试编译**：
```bash
cmake ../.. -DBUILD_TESTING=OFF
```
- 不编译测试程序
- 减少编译时间

**启用代码覆盖率**：
```bash
cmake ../.. \
    -DCMAKE_BUILD_TYPE=Debug \
    -DENABLE_COVERAGE=ON
make unit-test
make coverage
```
- 生成覆盖率报告
- 报告位置：`output/build/coverage_html/index.html`

### 配置编译参数

#### 示例1: Debug模式编译测试

```bash
cd output/build
cmake ../.. \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_TESTING=ON
make unit-test -j$(nproc)
cd ../..
```

#### 示例2: 启用代码覆盖率

```bash
cd output/build
cmake ../.. \
    -DCMAKE_BUILD_TYPE=Debug \
    -DENABLE_COVERAGE=ON
make coverage
cd ../..

# 查看覆盖率报告
xdg-open output/build/coverage_html/index.html
```

#### 示例3: 编译单个测试模块

```bash
cd output/build
make test_osal_task -j$(nproc)
cd ../..
```

### 编译输出

```
output/
├── build/
│   ├── lib/
│   │   └── libtest_runner.a   # 测试运行器库
│   └── coverage_html/         # 覆盖率报告（可选）
└── target/
    └── bin/
        └── unit-test          # 统一测试程序
```

### 常用编译命令

```bash
# 完整编译流程
./build.sh -d                   # Debug模式编译所有（包含测试）

# 仅编译测试
cd output/build && make unit-test -j$(nproc) && cd ../..

# 编译并运行测试
cd output/build && make run_tests && cd ../..

# 生成覆盖率报告
cd output/build && make coverage && cd ../..

# 查看测试编译配置
cd output/build && cmake -L ../.. | grep -E "BUILD_TESTING|COVERAGE" && cd ../..

# 清理并重新编译
./build.sh -c && ./build.sh -d
```

## 运行测试

### 交互式菜单（推荐）

```bash
./output/target/bin/unit-test -i
```
- 三级选择：层级 → 模块 → 测试用例
- 使用序号选择，无需输入完整名称
- 支持单个测试、模块测试、层级测试

### 命令行模式

```bash
# 运行所有测试
./output/target/bin/unit-test -a

# 列出所有测试
./output/target/bin/unit-test -l

# 运行指定层级测试
./output/target/bin/unit-test -L OSAL      # 运行OSAL层所有测试
./output/target/bin/unit-test -L HAL       # 运行HAL层所有测试

# 运行指定模块测试
./output/target/bin/unit-test -m test_osal_task    # 运行任务测试
./output/target/bin/unit-test -m test_hal_can      # 运行CAN测试

# 运行单个测试用例
./output/target/bin/unit-test -t test_osal_task test_task_create
```

## 模块结构

```
tests/
├── include/                    # 测试框架头文件
│   ├── test_framework.h        # 测试框架宏定义
│   └── test_runner.h           # 测试运行器接口
├── src/                        # 测试源代码
│   ├── test_entry.c            # 统一测试入口
│   ├── test_runner.c           # 测试运行器实现
│   ├── osal/                   # OSAL层测试
│   │   ├── test_osal_task.c    # 任务测试
│   │   ├── test_osal_queue.c   # 队列测试
│   │   ├── test_osal_mutex.c   # 互斥锁测试
│   │   └── test_osal_signal.c  # 信号测试
│   └── hal/                    # HAL层测试
│       └── test_hal_can.c      # CAN驱动测试
└── docs/                       # 测试文档
```

## 添加新测试

### 步骤1: 创建测试文件

创建 `tests/src/<layer>/test_new_module.c`：
```c
#include "test_framework.h"
#include <osal.h>

/* 测试用例1 */
TEST_CASE(test_case_1)
{
    /* 测试逻辑 */
    int32 result = some_function();
    
    /* 断言 */
    TEST_ASSERT(result == OS_SUCCESS, "Function should succeed");
    
    return TEST_PASS;
}

/* 测试用例2 */
TEST_CASE(test_case_2)
{
    /* 测试逻辑 */
    void *ptr = OSAL_Malloc(100);
    
    /* 断言 */
    TEST_ASSERT(ptr != NULL, "Malloc should succeed");
    
    /* 清理 */
    OSAL_Free(ptr);
    
    return TEST_PASS;
}

/* 注册测试模块 */
TEST_MODULE_BEGIN(test_new_module)
    TEST_CASE_ADD(test_case_1)
    TEST_CASE_ADD(test_case_2)
TEST_MODULE_END()
```

### 步骤2: 注册到测试入口

编辑 `tests/src/test_entry.c`：
```c
/* 声明测试模块 */
extern test_module_t test_new_module;

/* 添加到对应层级数组 */
static test_module_t *osal_modules[] = {
    &test_osal_task,
    &test_osal_queue,
    &test_new_module,    // 添加新模块
    NULL
};
```

### 步骤3: 修改CMakeLists.txt

编辑 `tests/CMakeLists.txt`：
```cmake
add_executable(unit-test
    src/test_entry.c
    src/osal/test_osal_task.c
    src/osal/test_new_module.c    # 添加新文件
    # ...
)
```

### 步骤4: 编译并运行

```bash
./build.sh -d
./output/target/bin/unit-test -m test_new_module
```

## 测试框架API

### 测试宏定义

```c
/* 定义测试用例 */
TEST_CASE(test_name)
{
    // 测试逻辑
    return TEST_PASS;  // 或 TEST_FAIL
}

/* 注册测试模块 */
TEST_MODULE_BEGIN(module_name)
    TEST_CASE_ADD(test_case_1)
    TEST_CASE_ADD(test_case_2)
TEST_MODULE_END()

/* 断言宏 */
TEST_ASSERT(condition, message)           // 条件断言
TEST_ASSERT_EQ(actual, expected, message) // 相等断言
TEST_ASSERT_NEQ(actual, expected, message)// 不等断言
TEST_ASSERT_NULL(ptr, message)            // 空指针断言
TEST_ASSERT_NOT_NULL(ptr, message)        // 非空指针断言
```

### 测试返回值

```c
TEST_PASS    // 测试通过
TEST_FAIL    // 测试失败
```

## 编码规范（重要）

**必须遵守**：
- ✅ 使用OSAL接口：`OSAL_TaskCreate()`, `OSAL_Malloc()`
- ❌ 禁止直接调用系统调用：`pthread_create()`, `malloc()`
- ✅ 使用OSAL日志：`LOG_INFO()`, `LOG_ERROR()`
- ❌ 禁止使用：`printf()`, `fprintf()`
- ✅ 使用测试框架宏：`TEST_ASSERT()`, `TEST_CASE()`
- ❌ 禁止使用：`assert()`, 自定义测试宏

**示例**：
```c
/* ✅ 正确 */
TEST_CASE(test_task_create)
{
    osal_id_t task_id;
    int32 ret = OSAL_TaskCreate(&task_id, "test", task_func, NULL, 
                                 8192, OSAL_PRIORITY_C(100), 0);
    TEST_ASSERT(ret == OS_SUCCESS, "Task create should succeed");
    LOG_INFO("TEST", "Task created successfully");
    return TEST_PASS;
}

/* ❌ 错误 */
void test_task_create(void)
{
    pthread_t thread;  // 禁止
    int ret = pthread_create(&thread, NULL, task_func, NULL);  // 禁止
    assert(ret == 0);  // 禁止
    printf("Task created\n");  // 禁止
}
```

## 测试覆盖率

### 生成覆盖率报告

```bash
# 1. 启用覆盖率编译
cd output/build
cmake ../.. -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON

# 2. 生成覆盖率报告
make coverage

# 3. 查看报告
xdg-open coverage_html/index.html
cd ../..
```

### 覆盖率要求

- OSAL层：目标 > 80%
- HAL层：目标 > 70%
- PDL层：目标 > 60%

## 常见问题

**Q: 测试失败如何调试？**
```bash
# 使用GDB调试
gdb --args ./output/target/bin/unit-test -m test_osal_task
(gdb) run
(gdb) bt  # 查看调用栈
```

**Q: 如何跳过硬件相关测试？**
```c
/* 在测试用例中检查硬件 */
TEST_CASE(test_can_send)
{
    if (!hardware_available()) {
        LOG_INFO("TEST", "Hardware not available, skipping");
        return TEST_PASS;  // 跳过测试
    }
    
    // 测试逻辑
    return TEST_PASS;
}
```

**Q: 如何添加测试前置/后置操作？**
```c
/* 在测试模块中添加setup/teardown */
static int32 setup(void)
{
    /* 初始化操作 */
    return OS_SUCCESS;
}

static int32 teardown(void)
{
    /* 清理操作 */
    return OS_SUCCESS;
}

TEST_MODULE_BEGIN(test_module)
    TEST_MODULE_SETUP(setup)
    TEST_MODULE_TEARDOWN(teardown)
    TEST_CASE_ADD(test_case_1)
TEST_MODULE_END()
```

**Q: 测试超时如何处理？**
```c
/* 使用OSAL超时机制 */
TEST_CASE(test_with_timeout)
{
    int32 ret = OSAL_QueueGet(queue_id, &data, sizeof(data), 1000);
    if (ret == OS_QUEUE_TIMEOUT) {
        TEST_ASSERT(false, "Operation timeout");
        return TEST_FAIL;
    }
    return TEST_PASS;
}
```

**Q: 如何测试多线程代码？**
```c
TEST_CASE(test_multithread)
{
    /* 创建多个任务 */
    osal_id_t task1, task2;
    OSAL_TaskCreate(&task1, "task1", thread_func1, NULL, 8192, 100, 0);
    OSAL_TaskCreate(&task2, "task2", thread_func2, NULL, 8192, 100, 0);
    
    /* 等待任务完成 */
    OSAL_TaskDelay(1000);
    
    /* 验证结果 */
    TEST_ASSERT(shared_data == expected_value, "Shared data mismatch");
    
    return TEST_PASS;
}
```

## 参考文档

- [测试框架详细文档](docs/README.md)
- [NASA cFS ut_assert](https://github.com/nasa/cFS/tree/main/cfe/modules/core_api/ut-stubs)
- [编码规范](../docs/CODING_STANDARDS.md)
