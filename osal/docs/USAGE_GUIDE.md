# OSAL 使用指南

本文档提供OSAL的实用示例和最佳实践。

## 目录

- [快速开始](#快速开始)
- [任务管理](#任务管理)
- [消息队列](#消息队列)
- [互斥锁](#互斥锁)
- [信号处理](#信号处理)
- [日志系统](#日志系统)
- [错误处理](#错误处理)
- [最佳实践](#最佳实践)
- [常见问题](#常见问题)

---

## 快速开始

### 最小示例

```c
#include "osal.h"

int main(void)
{
    /* OSAL作为用户态库，无需显式初始化 */
    
    LOG_INFO("Main", "应用启动");
    
    /* 使用OSAL接口 */
    OSAL_TaskDelay(1000);
    
    LOG_INFO("Main", "应用退出");
    return 0;
}
```

### 编译链接

```bash
# CMakeLists.txt
target_link_libraries(my_app osal)
```

---

## 任务管理

### 创建和删除任务

```c
#include "osal.h"

/* 任务入口函数 */
static void worker_task(void *arg)
{
    uint32_t counter = 0;
    
    LOG_INFO("Worker", "任务启动");
    
    /* 任务循环 - 必须检查退出标志 */
    while (!OSAL_TaskShouldShutdown())
    {
        LOG_INFO("Worker", "工作中: %u", counter++);
        OSAL_TaskDelay(1000);  /* 延时1秒 */
    }
    
    LOG_INFO("Worker", "任务退出");
}

int main(void)
{
    osal_id_t task_id;
    int32_t ret;
    
    /* 创建任务 */
    ret = OSAL_TaskCreate(&task_id, "WorkerTask",
                          worker_task, NULL,
                          16*1024,  /* 栈大小: 16KB */
                          100,      /* 优先级: 100 */
                          0);       /* 标志: 0 */
    if (ret != OS_SUCCESS) {
        LOG_ERROR("Main", "创建任务失败: %d", ret);
        return -1;
    }
    
    LOG_INFO("Main", "任务创建成功");
    
    /* 等待5秒 */
    OSAL_TaskDelay(5000);
    
    /* 删除任务 */
    ret = OSAL_TaskDelete(task_id);
    if (ret == OS_SUCCESS) {
        LOG_INFO("Main", "任务已删除");
    }
    
    return 0;
}
```

### 任务间传递参数

```c
typedef struct {
    uint32_t interval_ms;
    const char *name;
} task_config_t;

static void configurable_task(void *arg)
{
    task_config_t *config = (task_config_t *)arg;
    
    LOG_INFO(config->name, "任务启动，间隔: %u ms", config->interval_ms);
    
    while (!OSAL_TaskShouldShutdown())
    {
        /* 使用配置参数 */
        OSAL_TaskDelay(config->interval_ms);
    }
}

int main(void)
{
    task_config_t config = {
        .interval_ms = 500,
        .name = "FastTask"
    };
    
    osal_id_t task_id;
    OSAL_TaskCreate(&task_id, "ConfigTask",
                    configurable_task, &config,
                    16*1024, 100, 0);
    
    /* 注意: config必须在任务生命周期内有效 */
    OSAL_TaskDelay(5000);
    OSAL_TaskDelete(task_id);
    
    return 0;
}
```

---

## 消息队列

### 基本队列通信

```c
#include "osal.h"

#define QUEUE_DEPTH     10
#define MSG_SIZE        64

static osal_id_t g_queue_id;

/* 生产者任务 */
static void producer_task(void *arg)
{
    uint32_t counter = 0;
    
    while (!OSAL_TaskShouldShutdown())
    {
        char msg[MSG_SIZE];
        OSAL_Snprintf(msg, sizeof(msg), "Message #%u", counter++);
        
        /* 发送消息（超时1秒） */
        int32_t ret = OSAL_QueuePut(g_queue_id, msg, sizeof(msg), 1000);
        if (ret == OS_SUCCESS) {
            LOG_INFO("Producer", "发送: %s", msg);
        } else if (ret == OS_QUEUE_TIMEOUT) {
            LOG_WARN("Producer", "队列发送超时");
        }
        
        OSAL_TaskDelay(500);
    }
}

/* 消费者任务 */
static void consumer_task(void *arg)
{
    while (!OSAL_TaskShouldShutdown())
    {
        char msg[MSG_SIZE];
        uint32_t size;
        
        /* 接收消息（超时2秒） */
        int32_t ret = OSAL_QueueGet(g_queue_id, msg, sizeof(msg), &size, 2000);
        if (ret == OS_SUCCESS) {
            LOG_INFO("Consumer", "接收: %s (大小: %u)", msg, size);
        } else if (ret == OS_QUEUE_TIMEOUT) {
            LOG_WARN("Consumer", "队列接收超时");
        }
    }
}

int main(void)
{
    osal_id_t producer_id, consumer_id;
    
    /* 创建队列 */
    OSAL_QueueCreate(&g_queue_id, "MsgQueue", QUEUE_DEPTH, MSG_SIZE, 0);
    
    /* 创建生产者和消费者任务 */
    OSAL_TaskCreate(&producer_id, "Producer", producer_task, NULL, 16*1024, 100, 0);
    OSAL_TaskCreate(&consumer_id, "Consumer", consumer_task, NULL, 16*1024, 100, 0);
    
    /* 运行10秒 */
    OSAL_TaskDelay(10000);
    
    /* 清理 */
    OSAL_TaskDelete(producer_id);
    OSAL_TaskDelete(consumer_id);
    OSAL_QueueDelete(g_queue_id);
    
    return 0;
}
```

### 非阻塞队列操作

```c
/* 非阻塞发送 */
int32_t ret = OSAL_QueuePut(queue_id, data, size, 0);
if (ret == OS_QUEUE_FULL) {
    LOG_WARN("Worker", "队列已满，丢弃消息");
}

/* 非阻塞接收 */
ret = OSAL_QueueGet(queue_id, data, size, &copied, OS_CHECK);
if (ret == OS_QUEUE_EMPTY) {
    LOG_DEBUG("Worker", "队列为空");
}
```

---

## 互斥锁

### 保护共享资源

```c
#include "osal.h"

static osal_id_t g_mutex_id;
static uint32_t g_shared_counter = 0;

static void increment_task(void *arg)
{
    while (!OSAL_TaskShouldShutdown())
    {
        /* 获取锁 */
        int32_t ret = OSAL_MutexLock(g_mutex_id, 1000);
        if (ret == OS_SUCCESS) {
            /* 临界区 */
            g_shared_counter++;
            LOG_INFO("Task", "计数器: %u", g_shared_counter);
            
            /* 释放锁 */
            OSAL_MutexUnlock(g_mutex_id);
        } else {
            LOG_ERROR("Task", "获取锁超时");
        }
        
        OSAL_TaskDelay(100);
    }
}

int main(void)
{
    osal_id_t task1_id, task2_id;
    
    /* 创建互斥锁 */
    OSAL_MutexCreate(&g_mutex_id, "CounterMutex", 0);
    
    /* 创建两个任务同时访问共享资源 */
    OSAL_TaskCreate(&task1_id, "Task1", increment_task, NULL, 16*1024, 100, 0);
    OSAL_TaskCreate(&task2_id, "Task2", increment_task, NULL, 16*1024, 100, 0);
    
    OSAL_TaskDelay(5000);
    
    /* 清理 */
    OSAL_TaskDelete(task1_id);
    OSAL_TaskDelete(task2_id);
    OSAL_MutexDelete(g_mutex_id);
    
    LOG_INFO("Main", "最终计数: %u", g_shared_counter);
    
    return 0;
}
```

### 避免死锁

```c
/* 错误示例 - 可能死锁 */
void bad_example(void)
{
    OSAL_MutexLock(mutex1, OS_PEND);  /* 永久等待 */
    OSAL_MutexLock(mutex2, OS_PEND);  /* 如果mutex2被占用，死锁 */
    /* ... */
    OSAL_MutexUnlock(mutex2);
    OSAL_MutexUnlock(mutex1);
}

/* 正确示例 - 使用超时 */
void good_example(void)
{
    int32_t ret1 = OSAL_MutexLock(mutex1, 1000);
    if (ret1 != OS_SUCCESS) {
        LOG_ERROR("Worker", "获取mutex1超时");
        return;
    }
    
    int32_t ret2 = OSAL_MutexLock(mutex2, 1000);
    if (ret2 != OS_SUCCESS) {
        LOG_ERROR("Worker", "获取mutex2超时");
        OSAL_MutexUnlock(mutex1);  /* 释放已获取的锁 */
        return;
    }
    
    /* 临界区 */
    
    OSAL_MutexUnlock(mutex2);
    OSAL_MutexUnlock(mutex1);
}
```

---

## 信号处理

### 优雅退出

```c
#include "osal.h"

static volatile bool g_running = true;

/* 信号处理函数 */
static void signal_handler(int32_t sig)
{
    if (sig == OS_SIGNAL_INT || sig == OS_SIGNAL_TERM) {
        LOG_INFO("Main", "收到退出信号");
        g_running = false;
    }
}

int main(void)
{
    /* 注册信号处理 */
    OSAL_SignalRegister(OS_SIGNAL_INT, signal_handler);   /* Ctrl+C */
    OSAL_SignalRegister(OS_SIGNAL_TERM, signal_handler);  /* kill */
    
    LOG_INFO("Main", "应用启动，按Ctrl+C退出");
    
    /* 主循环 */
    while (g_running)
    {
        /* 执行工作 */
        OSAL_TaskDelay(1000);
    }
    
    LOG_INFO("Main", "应用退出");
    return 0;
}
```

---

## 日志系统

### 日志级别

```c
/* DEBUG - 调试信息 */
LOG_DEBUG("Module", "变量值: %d", value);

/* INFO - 一般信息 */
LOG_INFO("Module", "任务启动成功");

/* WARN - 警告信息 */
LOG_WARN("Module", "队列接近满载: %u/%u", count, depth);

/* ERROR - 错误信息 */
LOG_ERROR("Module", "打开文件失败: %s", path);

/* FATAL - 致命错误 */
LOG_FATAL("Module", "内存分配失败，程序退出");
```

### 日志输出格式

```
[2026-04-26 10:30:45.123] [INFO] [Worker] 任务启动成功
[2026-04-26 10:30:46.456] [ERROR] [CAN] 发送失败: -1
```

### 简单打印

```c
/* 不带时间戳和级别的简单打印 */
OSAL_Printf("========================================\n");
OSAL_Printf("  应用版本: %s\n", APP_VERSION);
OSAL_Printf("  OSAL版本: %s\n", OS_GetVersionString());
OSAL_Printf("========================================\n");
```

---

## 错误处理

### 检查返回值

```c
/* 所有OSAL函数都返回int32状态码 */
int32_t ret = OSAL_TaskCreate(&task_id, "MyTask", task_entry, NULL,
                               16*1024, 100, 0);
if (ret != OS_SUCCESS) {
    /* 获取错误描述 */
    const char *err_str = OSAL_GetErrorString(ret);
    LOG_ERROR("Main", "创建任务失败: %s (%d)", err_str, ret);
    return -1;
}
```

### 常见错误码

```c
OS_SUCCESS              /* 成功 */
OS_ERROR                /* 通用错误 */
OS_INVALID_POINTER      /* 无效指针 */
OS_ERR_INVALID_ID       /* 无效ID */
OS_ERR_NAME_TOO_LONG    /* 名称过长 */
OS_ERR_NO_FREE_IDS      /* 无可用ID */
OS_ERR_NAME_TAKEN       /* 名称已存在 */
OS_SEM_TIMEOUT          /* 超时 */
OS_QUEUE_EMPTY          /* 队列为空 */
OS_QUEUE_FULL           /* 队列已满 */
OS_QUEUE_TIMEOUT        /* 队列超时 */
OS_ERR_NO_MEMORY        /* 内存不足 */
```

---

## 最佳实践

### 1. 任务编写规范

```c
/* ✅ 正确 - 检查退出标志 */
static void good_task(void *arg)
{
    while (!OSAL_TaskShouldShutdown())
    {
        /* 执行工作 */
        OSAL_TaskDelay(100);
    }
    /* 清理资源 */
}

/* ❌ 错误 - 无限循环 */
static void bad_task(void *arg)
{
    while (1)  /* 无法优雅退出 */
    {
        /* 执行工作 */
        OSAL_TaskDelay(100);
    }
}
```

### 2. 资源管理

```c
/* ✅ 正确 - 成对创建和删除 */
osal_id_t task_id;
OSAL_TaskCreate(&task_id, "MyTask", task_entry, NULL, 16*1024, 100, 0);
/* 使用任务 */
OSAL_TaskDelete(task_id);  /* 必须删除 */

/* ✅ 正确 - 错误时清理 */
osal_id_t queue_id, task_id;

if (OSAL_QueueCreate(&queue_id, "Queue", 10, 64, 0) != OS_SUCCESS) {
    return -1;
}

if (OSAL_TaskCreate(&task_id, "Task", task_entry, NULL, 16*1024, 100, 0) != OS_SUCCESS) {
    OSAL_QueueDelete(queue_id);  /* 清理已创建的资源 */
    return -1;
}
```

### 3. 使用超时

```c
/* ✅ 正确 - 使用超时避免永久阻塞 */
int32_t ret = OSAL_MutexLock(mutex_id, 1000);  /* 1秒超时 */
if (ret == OS_SEM_TIMEOUT) {
    LOG_ERROR("Worker", "获取锁超时，可能死锁");
    return;
}

/* ❌ 错误 - 永久等待可能导致死锁 */
OSAL_MutexLock(mutex_id, OS_PEND);  /* 危险 */
```

### 4. 日志使用

```c
/* ✅ 正确 - 使用LOG宏 */
LOG_INFO("Module", "任务启动");
LOG_ERROR("Module", "错误: %d", ret);

/* ❌ 错误 - 直接使用printf */
printf("任务启动\n");  /* 不推荐 */
```

### 5. 命名规范

```c
/* ✅ 正确 - 描述性名称 */
OSAL_TaskCreate(&task_id, "WorkerTask", ...);
OSAL_QueueCreate(&queue_id, "MsgQueue", ...);
OSAL_MutexCreate(&mutex_id, "DataMutex", ...);

/* ❌ 错误 - 无意义名称 */
OSAL_TaskCreate(&task_id, "Task1", ...);
OSAL_QueueCreate(&queue_id, "Q", ...);
```

---

## 常见问题

### Q1: 任务无法退出？

**原因**: 任务循环未检查`OSAL_TaskShouldShutdown()`

**解决**:
```c
while (!OSAL_TaskShouldShutdown())  /* 必须检查 */
{
    /* 任务逻辑 */
}
```

### Q2: 队列操作失败？

**原因**: 队列已满或已空

**解决**:
```c
int32_t ret = OSAL_QueuePut(queue_id, data, size, 1000);
if (ret == OS_QUEUE_TIMEOUT) {
    LOG_WARN("Worker", "队列满，消息被丢弃");
}
```

### Q3: 互斥锁超时？

**原因**: 可能死锁或锁持有时间过长

**解决**:
- 检查是否忘记释放锁
- 减少临界区代码
- 检查锁的获取顺序

### Q4: 内存泄漏？

**原因**: 资源未释放

**解决**:
```c
/* 确保所有创建的资源都被删除 */
OSAL_TaskDelete(task_id);
OSAL_QueueDelete(queue_id);
OSAL_MutexDelete(mutex_id);
```

### Q5: 如何调试？

**方法**:
1. 使用LOG_DEBUG输出调试信息
2. 检查返回值和错误码
3. 使用GDB调试
4. 查看日志文件

---

## 完整示例

参考 [sample_app](../../apps/sample_app/README.md) 获取完整的应用示例。

## 相关文档

- [架构设计](ARCHITECTURE.md)
- [API参考](API_REFERENCE.md)
- [模块概述](README.md)
