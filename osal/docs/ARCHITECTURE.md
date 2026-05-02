# OSAL 架构设计

## 设计理念

OSAL (Operating System Abstraction Layer) 采用**用户态库**设计，提供跨平台的操作系统抽象接口。设计目标：

1. **零初始化开销**：使用静态初始化，无需显式Init/Teardown
2. **跨平台移植**：接口层与实现层分离，易于移植到RTOS
3. **线程安全**：所有接口均为线程安全，适用于多线程环境
4. **资源保护**：引用计数、死锁检测等机制防止资源泄漏
5. **优雅退出**：避免使用pthread_cancel，支持任务优雅关闭

## 整体架构

```
┌─────────────────────────────────────────────────────────┐
│                    OSAL Public API                       │
│  (osal.h - 对外接口，平台无关)                            │
└─────────────────────────────────────────────────────────┘
                          │
        ┌─────────────────┼─────────────────┐
        │                 │                 │
┌───────▼──────┐  ┌──────▼──────┐  ┌──────▼──────┐
│  IPC模块     │  │  SYS模块    │  │  NET模块    │
│  任务/队列   │  │  文件/信号  │  │  Socket     │
│  互斥锁/原子 │  │  时钟/环境  │  │  Termios    │
└───────┬──────┘  └──────┬──────┘  └──────┬──────┘
        │                 │                 │
        └─────────────────┼─────────────────┘
                          │
┌─────────────────────────▼─────────────────────────────┐
│              LIB模块 + UTIL模块                        │
│  字符串/内存/错误处理 + 日志/版本                      │
└─────────────────────────┬─────────────────────────────┘
                          │
┌─────────────────────────▼─────────────────────────────┐
│            POSIX Implementation Layer                  │
│  (src/posix/ - 平台相关实现)                           │
│  pthread, socket, file I/O, signal, etc.              │
└─────────────────────────┬─────────────────────────────┘
                          │
┌─────────────────────────▼─────────────────────────────┐
│              Linux Kernel / POSIX OS                   │
└───────────────────────────────────────────────────────┘
```

## 模块详解

### 1. IPC模块 - 进程间通信

#### 1.1 任务管理 (osal_task.c)

**设计要点**：
- 使用pthread实现，但封装为OSAL接口
- **优雅关闭机制**：使用shutdown标志而非pthread_cancel
- **超时等待**：使用pthread_timedjoin_np（5秒超时）
- **资源清理**：超时后使用pthread_detach而非强制取消

**数据结构**：
```c
typedef struct {
    bool        is_used;
    osal_id_t   id;
    char       name[OS_MAX_API_NAME];
    pthread_t   pthread_id;
    void       (*entry_point)(void *);
    void       *arg;
    bool        shutdown_flag;  /* 优雅退出标志 */
} osal_task_record_t;
```

**关键实现**：
```c
/* 任务入口包装 */
static void* task_wrapper(void *arg)
{
    osal_task_record_t *task = arg;
    
    /* 调用用户任务函数 */
    task->entry_point(task->arg);
    
    return NULL;
}

/* 优雅关闭检查 */
bool OSAL_TaskShouldShutdown(void)
{
    osal_id_t task_id = OSAL_TaskGetId();
    /* 查找任务记录，返回shutdown_flag */
}
```

**任务生命周期**：
```
创建 → 运行 → 检查shutdown标志 → 清理资源 → 退出
  ↑                                            ↓
  └────────── 超时后detach ←──────────────────┘
```

#### 1.2 消息队列 (osal_queue.c)

**设计要点**：
- **环形缓冲区**实现，固定大小
- **引用计数**保护，防止use-after-free
- **条件变量**实现阻塞/超时等待
- **线程安全**：全局表锁 + 队列实例锁

**数据结构**：
```c
typedef struct {
    uint8_t  *buffer;           /* 环形缓冲区 */
    uint32_t  head;             /* 读指针 */
    uint32_t  tail;             /* 写指针 */
    uint32_t  count;            /* 当前消息数 */
    uint32_t  depth;            /* 队列深度 */
    uint32_t  msg_size;         /* 消息大小 */
    pthread_mutex_t mutex;      /* 队列锁 */
    pthread_cond_t  not_empty;  /* 非空条件变量 */
    pthread_cond_t  not_full;   /* 非满条件变量 */
    atomic_int      ref_count;  /* 引用计数 */
    bool            valid;      /* 有效标志 */
} queue_impl_t;
```

**引用计数机制**：
```c
/* 获取队列（增加引用计数） */
queue_impl_t* osal_queue_acquire(osal_id_t queue_id)
{
    /* 查找队列 */
    if (impl->valid) {
        atomic_fetch_add(&impl->ref_count, 1);
        return impl;
    }
    return NULL;
}

/* 释放队列（减少引用计数） */
void osal_queue_release(queue_impl_t *impl)
{
    int old_count = atomic_fetch_sub(&impl->ref_count, 1);
    
    /* 引用计数降为0且已标记无效，则释放资源 */
    if (old_count == 1 && !impl->valid) {
        /* 销毁mutex、cond、释放内存 */
    }
}
```

**删除流程**：
```
1. 从全局表移除
2. 标记valid=false
3. 唤醒所有等待线程
4. 释放初始引用
5. 等待所有使用者释放引用
6. 最后一个释放者销毁资源
```

#### 1.3 互斥锁 (osal_mutex.c)

**设计要点**：
- 使用pthread_mutex实现
- **死锁检测**：5秒超时机制
- **递归锁支持**：PTHREAD_MUTEX_RECURSIVE
- **优先级继承**：PTHREAD_PRIO_INHERIT（可选）

**数据结构**：
```c
typedef struct {
    bool        is_used;
    osal_id_t   id;
    char       name[OS_MAX_API_NAME];
    pthread_mutex_t mutex;
} osal_mutex_record_t;
```

**超时锁实现**：
```c
int32_t OSAL_MutexLock(osal_id_t mutex_id, int32_t timeout)
{
    if (timeout == OS_PEND) {
        /* 永久等待 */
        pthread_mutex_lock(&mutex);
    } else {
        /* 超时等待（5秒） */
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += timeout / 1000;
        
        int ret = pthread_mutex_timedlock(&mutex, &ts);
        if (ret == ETIMEDOUT) {
            return OS_SEM_TIMEOUT;  /* 死锁检测 */
        }
    }
    return OS_SUCCESS;
}
```

### 2. SYS模块 - 系统调用封装

#### 2.1 信号处理 (osal_signal.c)

**设计要点**：
- 封装POSIX信号机制
- 支持信号注册和注销
- 线程安全的信号处理表

**信号映射**：
```c
OS_SIGNAL_INT  → SIGINT   (Ctrl+C)
OS_SIGNAL_TERM → SIGTERM  (终止信号)
OS_SIGNAL_HUP  → SIGHUP   (挂起信号)
```

#### 2.2 文件操作 (osal_file.c)

**设计要点**：
- 封装POSIX文件I/O
- 统一错误码转换
- 支持阻塞/非阻塞模式

### 3. NET模块 - 网络抽象

#### 3.1 Socket封装 (osal_socket.c)

**设计要点**：
- 封装socket、bind、connect等系统调用
- 统一错误处理
- 支持TCP/UDP

### 4. LIB模块 - 标准库封装

#### 4.1 错误处理 (osal_errno.c)

**37个标准错误码**：
```c
OS_SUCCESS              =  0   /* 成功 */
OS_ERROR                = -1   /* 通用错误 */
OS_INVALID_POINTER      = -2   /* 无效指针 */
OS_ERR_INVALID_ID       = -3   /* 无效ID */
OS_ERR_NAME_TOO_LONG    = -4   /* 名称过长 */
OS_ERR_NO_FREE_IDS      = -5   /* 无可用ID */
OS_ERR_NAME_TAKEN       = -6   /* 名称已存在 */
OS_SEM_TIMEOUT          = -7   /* 信号量超时 */
OS_QUEUE_EMPTY          = -8   /* 队列为空 */
OS_QUEUE_FULL           = -9   /* 队列已满 */
OS_QUEUE_TIMEOUT        = -10  /* 队列超时 */
/* ... 共37个 ... */
```

**错误信息查询**：
```c
const char* OSAL_GetErrorString(int32_t error_code);
```

### 5. UTIL模块 - 工具类

#### 5.1 日志系统 (osal_log.c)

**设计要点**：
- **5级日志**：DEBUG, INFO, WARN, ERROR, FATAL
- **自动轮转**：10MB/文件，保留5个备份
- **线程安全**：使用互斥锁保护
- **格式化输出**：时间戳 + 级别 + 模块 + 消息

**日志级别**：
```c
typedef enum {
    OSAL_LOG_DEBUG = 0,
    OSAL_LOG_INFO  = 1,
    OSAL_LOG_WARN  = 2,
    OSAL_LOG_ERROR = 3,
    OSAL_LOG_FATAL = 4
} osal_log_level_t;
```

**日志宏**：
```c
LOG_DEBUG(module, fmt, ...)
LOG_INFO(module, fmt, ...)
LOG_WARN(module, fmt, ...)
LOG_ERROR(module, fmt, ...)
LOG_FATAL(module, fmt, ...)
```

**日志轮转机制**：
```
app.log (当前日志)
app.log.1 (最近备份)
app.log.2
app.log.3
app.log.4
app.log.5 (最旧备份)

当app.log达到10MB时：
1. 删除app.log.5
2. app.log.4 → app.log.5
3. app.log.3 → app.log.4
4. app.log.2 → app.log.3
5. app.log.1 → app.log.2
6. app.log → app.log.1
7. 创建新的app.log
```

## 线程安全设计

### 全局资源保护

所有全局资源表使用互斥锁保护：

```c
/* 任务表 */
static osal_task_record_t g_osal_task_table[OS_MAX_TASKS];
static pthread_mutex_t g_task_table_mutex = PTHREAD_MUTEX_INITIALIZER;

/* 队列表 */
static osal_queue_record_t g_osal_queue_table[OS_MAX_QUEUES];
static pthread_mutex_t g_queue_table_mutex = PTHREAD_MUTEX_INITIALIZER;

/* 互斥锁表 */
static osal_mutex_record_t g_osal_mutex_table[OS_MAX_MUTEXES];
static pthread_mutex_t g_mutex_table_mutex = PTHREAD_MUTEX_INITIALIZER;
```

### 两级锁机制

**队列操作示例**：
```c
int32_t OSAL_QueuePut(osal_id_t queue_id, const void *data, ...)
{
    /* 1. 获取全局表锁，查找队列并增加引用计数 */
    pthread_mutex_lock(&g_queue_table_mutex);
    queue_impl_t *impl = osal_queue_acquire(queue_id);
    pthread_mutex_unlock(&g_queue_table_mutex);
    
    /* 2. 使用队列实例锁进行操作 */
    pthread_mutex_lock(&impl->mutex);
    /* 写入数据 */
    pthread_mutex_unlock(&impl->mutex);
    
    /* 3. 释放引用计数 */
    osal_queue_release(impl);
}
```

## 静态初始化机制

OSAL作为用户态库，使用静态初始化，无需显式Init/Teardown：

```c
/* 全局表静态初始化为0 */
static osal_task_record_t g_osal_task_table[OS_MAX_TASKS] = {0};

/* 互斥锁静态初始化 */
static pthread_mutex_t g_task_table_mutex = PTHREAD_MUTEX_INITIALIZER;

/* 无需Init函数，直接使用 */
```

## 资源限制

```c
#define OS_MAX_TASKS        64   /* 最大任务数 */
#define OS_MAX_QUEUES       64   /* 最大队列数 */
#define OS_MAX_MUTEXES      64   /* 最大互斥锁数 */
#define OS_MAX_API_NAME     20   /* 最大名称长度 */
```

## 移植到RTOS

OSAL设计为易于移植到RTOS，只需实现POSIX接口的RTOS版本：

```
osal/
├── include/           # 接口层（平台无关）
│   └── *.h
├── src/posix/         # POSIX实现
│   └── *.c
├── src/freertos/      # FreeRTOS实现（待添加）
│   └── *.c
└── src/vxworks/       # VxWorks实现（待添加）
    └── *.c
```

**移植步骤**：
1. 创建 `src/<rtos_name>/` 目录
2. 实现所有OSAL接口
3. 修改CMakeLists.txt选择实现
4. 测试验证

## 性能指标

- **任务切换延迟**：< 1ms
- **队列操作延迟**：< 100μs
- **互斥锁获取延迟**：< 50μs
- **日志写入延迟**：< 1ms（异步）

## 设计权衡

### 为什么不使用pthread_cancel？

**问题**：pthread_cancel可能导致：
- 资源泄漏（锁未释放）
- 死锁（取消点不确定）
- 清理函数复杂

**解决方案**：使用shutdown标志 + 超时等待
```c
/* 任务循环检查退出标志 */
while (!OSAL_TaskShouldShutdown()) {
    /* 执行任务逻辑 */
}
/* 清理资源 */
```

### 为什么使用引用计数？

**问题**：多线程环境下，队列可能在使用时被删除

**解决方案**：引用计数 + 延迟释放
- 使用者持有引用，防止提前释放
- 删除时标记无效，等待引用归零
- 最后一个释放者销毁资源

### 为什么需要死锁检测？

**问题**：互斥锁可能因编程错误导致死锁

**解决方案**：5秒超时机制
- 正常情况下，锁持有时间很短
- 超过5秒说明可能死锁
- 返回超时错误，便于调试

## 相关文档

- [API参考](API_REFERENCE.md)
- [使用指南](USAGE_GUIDE.md)
- [编码规范](../../docs/CODING_STANDARDS.md)
