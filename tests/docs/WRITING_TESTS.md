# 编写测试指南

## 创建新测试模块

### 步骤1：创建测试文件

```bash
cd tests/src/osal
touch test_my_module.c
```

### 步骤2：编写测试代码

```c
#include "test_framework.h"
#include "osal.h"

/* 定义测试模块 */
TEST_MODULE_BEGIN(test_my_module)

/* 测试用例1 */
TEST_CASE(test_MyFunction_Success)
{
    /* 准备 */
    int32_t ret;
    
    /* 执行 */
    ret = MyFunction();
    
    /* 验证 */
    TEST_ASSERT(ret == OS_SUCCESS, "函数调用失败");
}

/* 测试用例2 */
TEST_CASE(test_MyFunction_InvalidParam)
{
    int32_t ret;
    
    /* 测试无效参数 */
    ret = MyFunction_WithInvalidParam();
    
    TEST_ASSERT(ret == OS_INVALID_POINTER, "应返回无效指针错误");
}

/* 测试用例3 - 资源清理示例 */
TEST_CASE(test_MyFunction_ResourceCleanup)
{
    osal_id_t resource_id;
    
    /* 创建资源 */
    int32_t ret = CreateResource(&resource_id);
    TEST_ASSERT(ret == OS_SUCCESS, "创建资源失败");
    
    /* 使用资源 */
    ret = UseResource(resource_id);
    TEST_ASSERT(ret == OS_SUCCESS, "使用资源失败");
    
    /* 清理资源 */
    ret = DeleteResource(resource_id);
    TEST_ASSERT(ret == OS_SUCCESS, "删除资源失败");
}

TEST_MODULE_END(test_my_module)
```

### 步骤3：注册测试模块

编辑 `tests/src/test_entry.c`：

```c
/* 声明测试模块 */
extern test_module_t test_my_module_module;

/* 添加到OSAL层模块列表 */
static test_module_t *g_osal_modules[] = {
    &test_osal_task_module,
    &test_osal_queue_module,
    &test_my_module_module,  /* 新增 */
    /* ... */
};
```

### 步骤4：更新CMakeLists.txt

编辑 `tests/CMakeLists.txt`：

```cmake
set(TEST_SOURCES
    src/test_entry.c
    src/test_runner.c
    src/osal/test_osal_task.c
    src/osal/test_osal_queue.c
    src/osal/test_my_module.c  # 新增
    # ...
)
```

### 步骤5：编译运行

```bash
./build.sh -d
./output/target/bin/unit-test -m test_my_module
```

## 测试框架宏

### TEST_MODULE_BEGIN / TEST_MODULE_END

定义测试模块：
```c
TEST_MODULE_BEGIN(module_name)
    /* 测试用例 */
TEST_MODULE_END(module_name)
```

### TEST_CASE

定义测试用例：
```c
TEST_CASE(test_case_name)
{
    /* 测试代码 */
}
```

### 断言宏

```c
/* 条件断言 */
TEST_ASSERT(condition, "错误消息");

/* 相等断言 */
TEST_ASSERT_EQUAL(expected, actual, "值不相等");

/* 非空断言 */
TEST_ASSERT_NOT_NULL(ptr, "指针为空");

/* 空断言 */
TEST_ASSERT_NULL(ptr, "指针应为空");
```

## 测试模式

### 1. 正常路径测试

测试正常使用场景：
```c
TEST_CASE(test_NormalPath)
{
    osal_id_t task_id;
    int32_t ret = OSAL_TaskCreate(&task_id, "TestTask", task_entry, NULL,
                                   16*1024, 100, 0);
    TEST_ASSERT(ret == OS_SUCCESS, "任务创建失败");
    
    OSAL_TaskDelete(task_id);
}
```

### 2. 错误路径测试

测试错误处理：
```c
TEST_CASE(test_ErrorPath)
{
    /* 测试空指针 */
    int32_t ret = OSAL_TaskCreate(NULL, "TestTask", task_entry, NULL,
                                   16*1024, 100, 0);
    TEST_ASSERT(ret == OS_INVALID_POINTER, "应返回无效指针错误");
    
    /* 测试名称过长 */
    ret = OSAL_TaskCreate(&task_id, "VeryLongTaskNameThatExceedsLimit",
                          task_entry, NULL, 16*1024, 100, 0);
    TEST_ASSERT(ret == OS_ERR_NAME_TOO_LONG, "应返回名称过长错误");
}
```

### 3. 边界条件测试

测试边界值：
```c
TEST_CASE(test_BoundaryConditions)
{
    /* 测试最小值 */
    int32_t ret = OSAL_QueueCreate(&queue_id, "Queue", 1, 1, 0);
    TEST_ASSERT(ret == OS_SUCCESS, "最小队列创建失败");
    OSAL_QueueDelete(queue_id);
    
    /* 测试最大值 */
    ret = OSAL_QueueCreate(&queue_id, "Queue", 10000, 65536, 0);
    TEST_ASSERT(ret == OS_SUCCESS, "最大队列创建失败");
    OSAL_QueueDelete(queue_id);
}
```

### 4. 并发测试

测试多线程场景：
```c
TEST_CASE(test_Concurrency)
{
    osal_id_t mutex_id;
    OSAL_MutexCreate(&mutex_id, "TestMutex", 0);
    
    /* 创建多个任务同时访问共享资源 */
    osal_id_t task1_id, task2_id;
    OSAL_TaskCreate(&task1_id, "Task1", concurrent_task, &mutex_id,
                    16*1024, 100, 0);
    OSAL_TaskCreate(&task2_id, "Task2", concurrent_task, &mutex_id,
                    16*1024, 100, 0);
    
    OSAL_TaskDelay(5000);
    
    OSAL_TaskDelete(task1_id);
    OSAL_TaskDelete(task2_id);
    OSAL_MutexDelete(mutex_id);
}
```

## 最佳实践

### 1. 测试命名

```c
/* ✅ 好的命名 - 描述性强 */
TEST_CASE(test_OSAL_TaskCreate_Success)
TEST_CASE(test_OSAL_TaskCreate_NullPointer)
TEST_CASE(test_OSAL_QueuePut_Timeout)

/* ❌ 不好的命名 - 不清晰 */
TEST_CASE(test1)
TEST_CASE(test_task)
```

### 2. 资源清理

```c
/* ✅ 正确 - 清理资源 */
TEST_CASE(test_WithCleanup)
{
    osal_id_t task_id;
    OSAL_TaskCreate(&task_id, "Task", task_entry, NULL, 16*1024, 100, 0);
    
    /* 测试逻辑 */
    
    OSAL_TaskDelete(task_id);  /* 清理 */
}

/* ❌ 错误 - 资源泄漏 */
TEST_CASE(test_WithoutCleanup)
{
    osal_id_t task_id;
    OSAL_TaskCreate(&task_id, "Task", task_entry, NULL, 16*1024, 100, 0);
    
    /* 测试逻辑 */
    
    /* 忘记清理 - 资源泄漏 */
}
```

### 3. 独立性

```c
/* ✅ 正确 - 测试独立 */
TEST_CASE(test_Independent)
{
    /* 创建自己的资源 */
    osal_id_t queue_id;
    OSAL_QueueCreate(&queue_id, "TestQueue", 10, 64, 0);
    
    /* 测试 */
    
    /* 清理 */
    OSAL_QueueDelete(queue_id);
}

/* ❌ 错误 - 依赖全局状态 */
static osal_id_t g_shared_queue;  /* 全局变量 */

TEST_CASE(test_Dependent)
{
    /* 依赖全局队列 - 不独立 */
    OSAL_QueuePut(g_shared_queue, data, size, 1000);
}
```

### 4. 错误消息

```c
/* ✅ 好的错误消息 - 描述性强 */
TEST_ASSERT(ret == OS_SUCCESS, "任务创建失败，返回值: %d", ret);
TEST_ASSERT(count == 10, "期望10个消息，实际: %u", count);

/* ❌ 不好的错误消息 - 不清晰 */
TEST_ASSERT(ret == OS_SUCCESS, "失败");
TEST_ASSERT(count == 10, "错误");
```

## 调试测试

### 使用GDB

```bash
gdb --args ./output/target/bin/unit-test -m test_my_module
(gdb) break test_MyFunction_Success
(gdb) run
(gdb) next
(gdb) print ret
```

### 添加调试日志

```c
TEST_CASE(test_WithDebugLog)
{
    LOG_DEBUG("Test", "开始测试");
    
    int32_t ret = MyFunction();
    LOG_DEBUG("Test", "返回值: %d", ret);
    
    TEST_ASSERT(ret == OS_SUCCESS, "函数调用失败");
}
```

## 示例测试

完整的测试示例请参考：
- `tests/src/osal/test_osal_task.c` - 任务管理测试
- `tests/src/osal/test_osal_queue.c` - 消息队列测试
- `tests/src/hal/test_hal_can.c` - CAN驱动测试

## 相关文档

- [测试架构](ARCHITECTURE.md)
- [测试使用指南](TESTING.md)
