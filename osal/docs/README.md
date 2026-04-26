# OSAL - 操作系统抽象层

## 概述

OSAL (Operating System Abstraction Layer) 是PMC-BSP的基础层，提供跨平台的操作系统抽象接口。设计基于NASA cFS OSAL，经过轻量化改造，适用于嵌入式Linux和RTOS环境。

## 主要特性

- **用户态库设计**：无需显式初始化，使用静态初始化机制
- **跨平台支持**：POSIX实现，易于移植到RTOS（FreeRTOS、VxWorks等）
- **线程安全**：所有接口均为线程安全
- **优雅关闭**：任务管理支持优雅退出，避免资源泄漏
- **引用计数**：消息队列使用引用计数，防止use-after-free
- **死锁检测**：互斥锁内置5秒超时和死锁检测
- **日志轮转**：5级日志系统，支持自动轮转（10MB/5个备份）

## 模块组成

### IPC - 进程间通信
- **任务管理** (`osal_task.h`) - 任务创建、删除、优雅退出
- **消息队列** (`osal_queue.h`) - 线程安全的消息队列（引用计数）
- **互斥锁** (`osal_mutex.h`) - 互斥锁（死锁检测）
- **原子操作** (`osal_atomic.h`) - 原子操作封装

### SYS - 系统调用封装
- **时钟** (`osal_clock.h`) - 系统时钟
- **信号** (`osal_signal.h`) - 信号处理
- **文件** (`osal_file.h`) - 文件I/O
- **Select** (`osal_select.h`) - I/O多路复用
- **环境变量** (`osal_env.h`) - 环境变量操作
- **时间** (`osal_time.h`) - 时间操作

### NET - 网络抽象
- **Socket** (`osal_socket.h`) - Socket操作
- **Termios** (`osal_termios.h`) - 串口控制

### LIB - 标准库封装
- **字符串** (`osal_string.h`) - 字符串操作
- **堆内存** (`osal_heap.h`) - 内存管理
- **错误处理** (`osal_errno.h`) - 错误码和错误信息（37个标准错误码）

### UTIL - 工具类
- **日志** (`osal_log.h`) - 日志系统（5级别，自动轮转）
- **版本** - 版本信息查询

## 快速开始

```c
#include "osal.h"

int main(void)
{
    /* OSAL作为用户态库，无需显式初始化 */
    
    /* 创建任务 */
    osal_id_t task_id;
    OSAL_TaskCreate(&task_id, "MyTask", my_task_entry, NULL,
                    16*1024, 100, 0);
    
    /* 创建队列 */
    osal_id_t queue_id;
    OSAL_QueueCreate(&queue_id, "MyQueue", 10, 64, 0);
    
    /* 使用日志 */
    LOG_INFO("Main", "应用启动成功");
    
    /* 等待任务完成 */
    OSAL_TaskDelay(5000);
    
    /* 清理资源 */
    OSAL_TaskDelete(task_id);
    OSAL_QueueDelete(queue_id);
    
    return 0;
}
```

## 文档导航

- **[架构设计](ARCHITECTURE.md)** - OSAL内部架构和实现原理
- **[API参考](API_REFERENCE.md)** - 完整的API接口文档
- **[使用指南](USAGE_GUIDE.md)** - 使用示例和最佳实践

## 目录结构

```
osal/
├── include/                 # 公共头文件
│   ├── osal.h              # 总头文件
│   ├── osal_types.h        # 类型定义
│   ├── ipc/                # IPC接口
│   ├── sys/                # 系统调用接口
│   ├── net/                # 网络接口
│   ├── lib/                # 标准库接口
│   └── util/               # 工具接口
├── src/posix/              # POSIX实现
│   ├── ipc/                # IPC实现
│   ├── sys/                # 系统调用实现
│   ├── net/                # 网络实现
│   ├── lib/                # 标准库实现
│   └── util/               # 工具实现
├── docs/                   # 文档
│   ├── README.md           # 本文件
│   ├── ARCHITECTURE.md     # 架构设计
│   ├── API_REFERENCE.md    # API参考
│   └── USAGE_GUIDE.md      # 使用指南
└── CMakeLists.txt          # 构建配置
```

## 设计原则

1. **平台无关性**：OSAL是唯一允许包含系统头文件的层
2. **接口稳定性**：API保持向后兼容
3. **资源管理**：所有资源必须显式释放
4. **错误处理**：所有函数返回int32状态码
5. **线程安全**：所有接口均为线程安全

## 依赖关系

```
应用层 (Apps)
    ↓
外设驱动层 (PDL)
    ↓
硬件抽象层 (HAL)
    ↓
操作系统抽象层 (OSAL) ← 你在这里
    ↓
Linux/POSIX 或 RTOS
```

## 版本信息

当前版本：v1.0.0

获取版本信息：
```c
const str_t *version = OS_GetVersionString();
OSAL_Printf("OSAL版本: %s\n", version);
```

## 相关文档

- [项目总体架构](../../docs/ARCHITECTURE.md)
- [编码规范](../../docs/CODING_STANDARDS.md)
- [HAL层文档](../../hal/docs/README.md)
- [PDL层文档](../../pdl/docs/README.md)
