# Tests 架构设计

## 测试框架设计

PMC-BSP采用统一的测试框架，支持交互式菜单和命令行两种模式。

## 整体架构

```
┌─────────────────────────────────────────────────────────┐
│              统一测试入口 (unit-test)                     │
│  test_entry.c - 主入口，解析参数，调用测试运行器          │
└─────────────────────────────────────────────────────────┘
                          │
┌─────────────────────────▼─────────────────────────────┐
│              测试运行器 (test_runner.c)                 │
│  - 管理测试注册表                                       │
│  - 执行测试用例                                         │
│  - 统计测试结果                                         │
└─────────────────────────────────────────────────────────┘
                          │
        ┌─────────────────┼─────────────────┐
        │                 │                 │
┌───────▼──────┐  ┌──────▼──────┐  ┌──────▼──────┐
│  OSAL测试    │  │  HAL测试    │  │  PDL测试    │
│  6模块       │  │  1模块      │  │  1模块      │
│  60用例      │  │  3用例      │  │  2用例      │
└──────────────┘  └─────────────┘  └─────────────┘
```

## 测试框架宏

### 模块定义

```c
/* 定义测试模块 */
TEST_MODULE_BEGIN(test_osal_task)

/* 定义测试用例 */
TEST_CASE(test_OSAL_TaskCreate_Success)
{
    /* 测试代码 */
    TEST_ASSERT(ret == OS_SUCCESS, "任务创建失败");
}

TEST_MODULE_END(test_osal_task)
```

### 断言宏

```c
TEST_ASSERT(condition, message)           /* 条件断言 */
TEST_ASSERT_EQUAL(expected, actual, msg)  /* 相等断言 */
TEST_ASSERT_NOT_NULL(ptr, message)        /* 非空断言 */
```

## 测试层次结构

### 三级结构

```
层级 (Layer)
  ↓
模块 (Module)
  ↓
测试用例 (Test Case)
```

**示例**：
```
OSAL层
  ├── test_osal_task (任务管理模块)
  │   ├── test_OSAL_TaskCreate_Success
  │   ├── test_OSAL_TaskDelete_Success
  │   └── test_OSAL_TaskShouldShutdown
  ├── test_osal_queue (消息队列模块)
  │   ├── test_OSAL_QueueCreate_Success
  │   ├── test_OSAL_QueuePut_Success
  │   └── test_OSAL_QueueGet_Success
  └── ...
```

## 测试注册机制

### 模块注册

```c
/* 在test_entry.c中注册 */
static test_module_t *g_osal_modules[] = {
    &test_osal_task_module,
    &test_osal_queue_module,
    &test_osal_mutex_module,
    /* ... */
};

static test_layer_t g_osal_layer = {
    .name = "OSAL",
    .modules = g_osal_modules,
    .module_count = sizeof(g_osal_modules) / sizeof(g_osal_modules[0])
};
```

### 层级注册

```c
static test_layer_t *g_all_layers[] = {
    &g_osal_layer,
    &g_hal_layer,
    &g_pdl_layer,
    &g_apps_layer
};
```

## 测试运行模式

### 1. 交互式模式

```bash
./unit-test -i
```

**菜单结构**：
```
1. Run all tests
2. Run OSAL layer tests
3. Run HAL layer tests
4. Run PDL layer tests
5. Run Apps layer tests
6. Run specific module tests
7. Run single test
8. List all tests
0. Exit
```

### 2. 命令行模式

```bash
# 运行所有测试
./unit-test -a

# 运行指定层
./unit-test -L OSAL

# 运行指定模块
./unit-test -m test_osal_task

# 运行单个测试
./unit-test -t test_osal_task test_OSAL_TaskCreate_Success

# 列出所有测试
./unit-test -l
```

## 测试结果统计

```c
typedef struct {
    uint32_t total;      /* 总测试数 */
    uint32_t passed;     /* 通过数 */
    uint32_t failed;     /* 失败数 */
    uint32_t skipped;    /* 跳过数 */
} test_stats_t;
```

**输出格式**：
```
========== 测试结果 ==========
总计: 60
通过: 58
失败: 2
跳过: 0
成功率: 96.67%
==============================
```

## 测试隔离

每个测试用例应该：
1. **独立运行**：不依赖其他测试
2. **清理资源**：测试结束后清理所有资源
3. **可重复**：多次运行结果一致

## 测试覆盖

### OSAL层测试
- **任务管理**：创建、删除、延时、退出检查
- **消息队列**：创建、删除、发送、接收、超时
- **互斥锁**：创建、删除、加锁、解锁、超时
- **文件操作**：打开、关闭、读写
- **网络**：Socket创建、连接、收发
- **信号**：信号注册、处理

### HAL层测试
- **CAN驱动**：初始化、发送、接收、过滤器

### PDL层测试
- **卫星平台**：初始化、命令处理、遥测上报

## 性能测试

测试框架支持性能测试：
```c
TEST_CASE(test_Performance)
{
    uint64_t start = get_timestamp();
    
    /* 执行操作 */
    for (int i = 0; i < 1000; i++) {
        OSAL_QueuePut(queue_id, data, size, 0);
    }
    
    uint64_t end = get_timestamp();
    uint64_t elapsed = end - start;
    
    LOG_INFO("Perf", "1000次队列操作耗时: %llu us", elapsed);
}
```

## 相关文档

- [编写测试指南](WRITING_TESTS.md)
- [测试使用指南](TESTING.md)
