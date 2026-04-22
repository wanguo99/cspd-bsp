# CSPD-BSP 软件架构详细分析

## 📋 目录
1. [系统概述](#系统概述)
2. [软件架构](#软件架构)
3. [模块详细分析](#模块详细分析)
4. [接口清单](#接口清单)
5. [数据流分析](#数据流分析)
6. [依赖关系](#依赖关系)

---

## 系统概述

**项目名称**: CSPD-BSP (Compute and Storage Payload Board Support Package)  
**用途**: 卫星算存载荷板级支持包  
**功能**: 作为卫星平台与算存载荷之间的桥接设备

### 系统拓扑

```
┌─────────────┐                    ┌──────────────┐                    ┌─────────────┐
│  卫星平台    │ <--- CAN 2.0 ---> │   管理板      │ <--- Ethernet --> │  算存载荷    │
│ (自定义协议) │                    │  (本框架)     │ <--- UART(备份)-> │ (IPMI/Redfish)│
└─────────────┘                    └──────────────┘                    └─────────────┘
```

### 核心功能
1. **CAN通信** - 与卫星平台进行CAN总线通信
2. **协议转换** - 卫星命令 ↔ IPMI/Redfish命令
3. **通信冗余** - 以太网主通道 + UART备份通道
4. **状态监控** - 定期查询载荷状态并上报
5. **故障处理** - 自动检测和恢复通信故障

---

## 软件架构

### 分层架构

```
┌─────────────────────────────────────────────────────────┐
│                    应用层 (Apps)                         │
│  ┌──────────────┐  ┌──────────────────────────────┐    │
│  │ CAN Gateway  │  │ Protocol Converter           │    │
│  │ - CAN接收    │  │ - 协议转换                   │    │
│  │ - CAN发送    │  │ - 命令执行                   │    │
│  │ - 心跳维护   │  │ - 载荷服务                   │    │
│  └──────────────┘  └──────────────────────────────┘    │
├─────────────────────────────────────────────────────────┤
│                   服务层 (Service)                       │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐ │
│  │ Satellite    │  │ Payload BMC  │  │ Watchdog     │ │
│  │ Service      │  │ Service      │  │ Service      │ │
│  └──────────────┘  └──────────────┘  └──────────────┘ │
├─────────────────────────────────────────────────────────┤
│              硬件抽象层 (HAL)                            │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐              │
│  │CAN驱动   │  │网络驱动  │  │串口驱动  │              │
│  └──────────┘  └──────────┘  └──────────┘              │
├─────────────────────────────────────────────────────────┤
│          操作系统抽象层 (OSAL)                           │
│  ┌────┐ ┌────┐ ┌────┐ ┌────┐ ┌────┐ ┌────┐           │
│  │任务│ │队列│ │互斥│ │时钟│ │日志│ │错误│           │
│  └────┘ └────┘ └────┘ └────┘ └────┘ └────┘           │
└─────────────────────────────────────────────────────────┘
│                    Linux Kernel                          │
└─────────────────────────────────────────────────────────┘
```

### 文件组织结构

```
cspd-bsp/
├── main.c                          # 主程序入口
├── config/                         # 配置文件
│   ├── system_config.h            # 系统配置
│   └── can_protocol.h             # CAN协议定义
├── osal/                          # 操作系统抽象层
│   ├── inc/                       # 接口定义
│   │   ├── osal.h                # 主头文件
│   │   ├── common_types.h        # 通用类型
│   │   ├── osapi_task.h          # 任务管理接口
│   │   ├── osapi_queue.h         # 消息队列接口
│   │   ├── osapi_mutex.h         # 互斥锁接口
│   │   ├── osapi_clock.h         # 时钟接口
│   │   ├── osapi_heap.h          # 内存接口
│   │   ├── osapi_error.h         # 错误处理接口
│   │   └── osapi_log.h           # 日志接口
│   └── linux/                     # Linux实现
│       ├── os_init.c             # 初始化
│       ├── os_task.c             # 任务管理
│       ├── os_queue.c            # 消息队列
│       ├── os_mutex.c            # 互斥锁
│       ├── os_clock.c            # 时钟
│       ├── os_heap.c             # 内存
│       ├── os_error.c            # 错误处理
│       └── os_log.c              # 日志
├── hal/                           # 硬件抽象层
│   ├── inc/                       # 接口定义
│   │   ├── hal_can.h             # CAN接口
│   │   ├── hal_network.h         # 网络接口
│   │   └── hal_serial.h          # 串口接口
│   └── linux/                     # Linux驱动
│       ├── hal_can_linux.c       # CAN驱动
│       ├── hal_network_linux.c   # 网络驱动
│       └── hal_serial_linux.c    # 串口驱动
├── service/                       # 服务层
│   ├── inc/                       # 接口定义
│   │   ├── service_satellite.h   # 卫星平台服务
│   │   ├── service_payload_bmc.h # BMC载荷服务
│   │   ├── service_payload_linux.h # Linux载荷服务
│   │   ├── service_power.h       # 电源管理服务
│   │   └── watchdog.h            # 看门狗服务
│   └── linux/                     # Linux实现
│       ├── service_satellite.c
│       ├── service_payload_bmc.c
│       ├── service_payload_linux.c
│       ├── service_power.c
│       └── watchdog.c
└── apps/                          # 应用层
    ├── can_gateway/               # CAN网关
    │   ├── can_gateway.h
    │   └── can_gateway.c
    └── protocol_converter/        # 协议转换
        ├── protocol_converter.h
        ├── protocol_converter.c
        ├── payload_service.h
        └── payload_service.c
```

---

## 模块详细分析

### 1. OSAL层 (操作系统抽象层)

**目的**: 提供统一的操作系统接口，支持跨平台移植

#### 1.1 任务管理 (os_task.c)

**功能**:
- 任务创建和删除
- 任务延时
- 任务优先级管理
- 任务信息查询

**关键数据结构**:
```c
typedef struct {
    bool        is_used;
    osal_id_t   id;
    char        name[OS_MAX_API_NAME];
    pthread_t   thread;
    uint32      priority;
    uint32      stack_size;
} OS_task_record_t;
```

**核心接口**:
- `OS_TaskCreate()` - 创建任务
- `OS_TaskDelete()` - 删除任务
- `OS_TaskDelay()` - 任务延时
- `OS_TaskGetId()` - 获取当前任务ID
- `OS_TaskSetPriority()` - 设置任务优先级

#### 1.2 消息队列 (os_queue.c)

**功能**:
- 模块间异步通信
- 环形缓冲区实现
- 支持阻塞/非阻塞/超时模式

**关键数据结构**:
```c
typedef struct {
    uint8  *buffer;
    uint32  head;
    uint32  tail;
    uint32  count;
    uint32  depth;
    uint32  msg_size;
    pthread_mutex_t mutex;
    pthread_cond_t  not_empty;
    pthread_cond_t  not_full;
} queue_impl_t;
```

**核心接口**:
- `OS_QueueCreate()` - 创建队列
- `OS_QueueDelete()` - 删除队列
- `OS_QueuePut()` - 发送消息
- `OS_QueueGet()` - 接收消息

#### 1.3 互斥锁 (os_mutex.c)

**功能**:
- 保护共享资源
- 防止竞态条件

**核心接口**:
- `OS_MutexCreate()` - 创建互斥锁
- `OS_MutexDelete()` - 删除互斥锁
- `OS_MutexLock()` - 获取锁
- `OS_MutexUnlock()` - 释放锁

#### 1.4 时钟服务 (os_clock.c)

**功能**:
- 获取系统时间
- 滴答计数

**核心接口**:
- `OS_GetLocalTime()` - 获取本地时间
- `OS_GetTickCount()` - 获取滴答计数

#### 1.5 日志系统 (os_log.c)

**功能**:
- 分级日志输出
- 时间戳自动添加

**核心接口**:
- `OS_printf()` - 打印日志

---

### 2. HAL层 (硬件抽象层)

**目的**: 封装硬件相关操作，隔离平台差异

#### 2.1 CAN驱动 (hal_can_linux.c)

**功能**:
- 基于SocketCAN实现
- CAN帧发送/接收
- 过滤器设置
- 统计信息

**关键数据结构**:
```c
typedef struct {
    int sockfd;
    uint32 tx_count;
    uint32 rx_count;
    uint32 err_count;
} can_handle_impl_t;

typedef struct {
    uint32     can_id;
    uint8      dlc;
    can_msg_t  msg;
    uint32     timestamp;
} can_frame_t;
```

**核心接口**:
- `HAL_CAN_Init()` - 初始化CAN
- `HAL_CAN_Deinit()` - 关闭CAN
- `HAL_CAN_Send()` - 发送CAN帧
- `HAL_CAN_Recv()` - 接收CAN帧
- `HAL_CAN_SetFilter()` - 设置过滤器
- `HAL_CAN_GetStats()` - 获取统计信息

#### 2.2 网络驱动 (hal_network_linux.c)

**功能**:
- TCP/IP通信
- 以太网连接管理

#### 2.3 串口驱动 (hal_serial_linux.c)

**功能**:
- UART通信
- 串口配置

---

### 3. Service层 (服务层)

**目的**: 提供业务逻辑封装和高级服务接口

#### 3.1 卫星平台服务 (service_satellite.c)

**功能**:
- 封装与卫星平台的CAN通信
- 处理卫星命令
- 心跳维护
- 状态上报

**核心接口**:
- `SatelliteService_Init()` - 初始化服务
- `SatelliteService_Deinit()` - 关闭服务
- `SatelliteService_RegisterCallback()` - 注册命令回调
- `SatelliteService_SendResponse()` - 发送响应
- `SatelliteService_SendHeartbeat()` - 发送心跳

#### 3.2 载荷服务 (payload_service.c)

**功能**:
- 以太网和UART双通道通信
- 自动通道切换
- 连接管理
- 故障恢复

**关键数据结构**:
```c
typedef struct {
    payload_service_config_t config;
    payload_channel_t        current_channel;
    int32                    eth_fd;
    int32                    uart_fd;
    bool                     connected;
    uint32                   fail_count;
} payload_service_context_t;
```

**核心接口**:
- `PayloadService_Init()` - 初始化服务
- `PayloadService_Deinit()` - 关闭服务
- `PayloadService_Send()` - 发送数据
- `PayloadService_Recv()` - 接收数据
- `PayloadService_IsConnected()` - 检查连接状态
- `PayloadService_SwitchChannel()` - 切换通道

#### 3.3 看门狗服务 (watchdog.c)

**功能**:
- 任务心跳监控
- 系统健康检查
- 自动故障恢复
- 硬件看门狗喂狗

**核心接口**:
- `Watchdog_Init()` - 初始化看门狗
- `Watchdog_Deinit()` - 关闭看门狗
- `Watchdog_RegisterTask()` - 注册监控任务
- `Watchdog_Heartbeat()` - 任务心跳上报
- `Watchdog_GetTaskHealth()` - 获取任务健康状态
- `Watchdog_GetSystemHealth()` - 获取系统健康状态

---

### 4. Apps层 (应用层)

**目的**: 实现具体业务逻辑

#### 4.1 CAN网关 (can_gateway.c)

**功能**:
- 接收卫星平台CAN消息
- 解析自定义CAN协议
- 转发到协议转换模块
- 发送响应和状态上报
- 心跳维护

**任务**:
- `can_rx_task` - CAN接收任务
- `can_tx_task` - CAN发送任务
- `can_heartbeat_task` - 心跳任务

**核心接口**:
- `CAN_Gateway_Init()` - 初始化网关
- `CAN_Gateway_GetRxQueue()` - 获取接收队列
- `CAN_Gateway_GetTxQueue()` - 获取发送队列
- `CAN_Gateway_SendResponse()` - 发送响应
- `CAN_Gateway_SendStatus()` - 发送状态
- `CAN_Gateway_GetStats()` - 获取统计信息

#### 4.2 协议转换 (protocol_converter.c)

**功能**:
- 卫星命令 → IPMI/Redfish命令
- 载荷响应 → 卫星协议格式
- 命令队列和超时处理

**任务**:
- `protocol_converter_task` - 协议转换任务

**核心接口**:
- `Protocol_Converter_Init()` - 初始化转换器
- `Protocol_Converter_GetStats()` - 获取统计信息
- `Protocol_Converter_SwitchChannel()` - 切换通道
- `Protocol_Converter_GetChannel()` - 获取当前通道

---

## 接口清单

### OSAL层接口 (共35个)

#### 任务管理 (7个)
1. `OS_API_Init()` - 初始化OSAL
2. `OS_API_Teardown()` - 关闭OSAL
3. `OS_TaskCreate()` - 创建任务
4. `OS_TaskDelete()` - 删除任务
5. `OS_TaskDelay()` - 任务延时
6. `OS_TaskGetId()` - 获取当前任务ID
7. `OS_TaskSetPriority()` - 设置任务优先级

#### 消息队列 (5个)
8. `OS_QueueCreate()` - 创建队列
9. `OS_QueueDelete()` - 删除队列
10. `OS_QueuePut()` - 发送消息
11. `OS_QueueGet()` - 接收消息
12. `OS_QueueGetIdByName()` - 根据名称获取队列ID

#### 互斥锁 (5个)
13. `OS_MutexCreate()` - 创建互斥锁
14. `OS_MutexDelete()` - 删除互斥锁
15. `OS_MutexLock()` - 获取锁
16. `OS_MutexUnlock()` - 释放锁
17. `OS_MutexGetIdByName()` - 根据名称获取互斥锁ID

#### 时钟服务 (2个)
18. `OS_GetLocalTime()` - 获取本地时间
19. `OS_GetTickCount()` - 获取滴答计数

#### 内存管理 (1个)
20. `OS_HeapGetInfo()` - 获取堆信息

#### 错误处理 (1个)
21. `OS_GetErrorName()` - 获取错误名称

#### 日志系统 (1个)
22. `OS_printf()` - 打印日志

### HAL层接口 (共15个)

#### CAN驱动 (6个)
23. `HAL_CAN_Init()` - 初始化CAN
24. `HAL_CAN_Deinit()` - 关闭CAN
25. `HAL_CAN_Send()` - 发送CAN帧
26. `HAL_CAN_Recv()` - 接收CAN帧
27. `HAL_CAN_SetFilter()` - 设置过滤器
28. `HAL_CAN_GetStats()` - 获取统计信息

#### 网络驱动 (预留)
29-31. 网络接口 (待实现)

#### 串口驱动 (预留)
32-34. 串口接口 (待实现)

### Service层接口 (共20个)

#### 卫星平台服务 (6个)
35. `SatelliteService_Init()` - 初始化服务
36. `SatelliteService_Deinit()` - 关闭服务
37. `SatelliteService_RegisterCallback()` - 注册回调
38. `SatelliteService_SendResponse()` - 发送响应
39. `SatelliteService_SendHeartbeat()` - 发送心跳
40. `SatelliteService_GetStats()` - 获取统计信息

#### 载荷服务 (6个)
41. `PayloadService_Init()` - 初始化服务
42. `PayloadService_Deinit()` - 关闭服务
43. `PayloadService_Send()` - 发送数据
44. `PayloadService_Recv()` - 接收数据
45. `PayloadService_IsConnected()` - 检查连接
46. `PayloadService_SwitchChannel()` - 切换通道

#### 看门狗服务 (6个)
47. `Watchdog_Init()` - 初始化看门狗
48. `Watchdog_Deinit()` - 关闭看门狗
49. `Watchdog_RegisterTask()` - 注册任务
50. `Watchdog_Heartbeat()` - 心跳上报
51. `Watchdog_GetTaskHealth()` - 获取任务健康状态
52. `Watchdog_GetSystemHealth()` - 获取系统健康状态

### Apps层接口 (共8个)

#### CAN网关 (6个)
53. `CAN_Gateway_Init()` - 初始化网关
54. `CAN_Gateway_GetRxQueue()` - 获取接收队列
55. `CAN_Gateway_GetTxQueue()` - 获取发送队列
56. `CAN_Gateway_SendResponse()` - 发送响应
57. `CAN_Gateway_SendStatus()` - 发送状态
58. `CAN_Gateway_GetStats()` - 获取统计信息

#### 协议转换 (4个)
59. `Protocol_Converter_Init()` - 初始化转换器
60. `Protocol_Converter_GetStats()` - 获取统计信息
61. `Protocol_Converter_SwitchChannel()` - 切换通道
62. `Protocol_Converter_GetChannel()` - 获取当前通道

**总计**: 62个公共接口

---

## 数据流分析

### 命令转发流程

```
1. 卫星平台发送CAN命令
   ↓
2. CAN驱动接收 (HAL_CAN_Recv)
   ↓
3. CAN网关解析 (can_rx_task)
   ↓
4. 放入接收队列 (OS_QueuePut)
   ↓
5. 协议转换取出 (OS_QueueGet)
   ↓
6. 转换为IPMI命令 (execute_ipmi_command)
   ↓
7. 载荷服务发送 (PayloadService_Send)
   ↓
8. 以太网/UART发送
   ↓
9. 载荷执行命令
   ↓
10. 载荷返回响应
   ↓
11. 载荷服务接收 (PayloadService_Recv)
   ↓
12. 协议转换解析
   ↓
13. CAN网关发送响应 (CAN_Gateway_SendResponse)
   ↓
14. CAN驱动发送 (HAL_CAN_Send)
   ↓
15. 卫星平台接收响应
```

### 心跳流程

```
1. 心跳任务定时触发 (can_heartbeat_task)
   ↓
2. 构造心跳消息 (can_build_heartbeat)
   ↓
3. 放入发送队列 (OS_QueuePut)
   ↓
4. CAN发送任务取出 (can_tx_task)
   ↓
5. CAN驱动发送 (HAL_CAN_Send)
   ↓
6. 卫星平台接收心跳
```

### 看门狗监控流程

```
1. 任务注册到看门狗 (Watchdog_RegisterTask)
   ↓
2. 任务定期上报心跳 (Watchdog_Heartbeat)
   ↓
3. 看门狗任务检查 (watchdog_task)
   ↓
4. 检测超时任务
   ↓
5. 触发故障恢复
   ↓
6. 喂硬件看门狗 (hw_watchdog_feed)
```

---

## 依赖关系

### 层级依赖

```
Apps层
  ↓ 依赖
Service层
  ↓ 依赖
HAL层
  ↓ 依赖
OSAL层
  ↓ 依赖
Linux Kernel
```

### 模块依赖图

```
main.c
  ├─→ CAN_Gateway
  │     ├─→ HAL_CAN
  │     │     └─→ OSAL
  │     └─→ OSAL (Queue, Task)
  │
  ├─→ Protocol_Converter
  │     ├─→ PayloadService
  │     │     ├─→ HAL_Network
  │     │     ├─→ HAL_Serial
  │     │     └─→ OSAL
  │     └─→ OSAL (Queue, Task)
  │
  └─→ Watchdog
        └─→ OSAL (Task, Mutex)
```

### 编译依赖

```
libosal.a
  ← os_task.o
  ← os_queue.o
  ← os_mutex.o
  ← os_clock.o
  ← os_heap.o
  ← os_error.o
  ← os_log.o
  ← os_init.o

libhal.a
  ← hal_can_linux.o (依赖 libosal.a)
  ← hal_network_linux.o
  ← hal_serial_linux.o

libservice.a
  ← service_satellite.o (依赖 libhal.a, libosal.a)
  ← service_payload_bmc.o
  ← service_payload_linux.o
  ← service_power.o
  ← watchdog.o

libapps.a
  ← can_gateway.o (依赖 libhal.a, libosal.a)
  ← protocol_converter.o (依赖 libservice.a)
  ← payload_service.o

cspd-bsp (可执行文件)
  ← main.o
  ← libapps.a
  ← libservice.a
  ← libhal.a
  ← libosal.a
  ← -lpthread
```

---

## 关键设计模式

### 1. 分层架构模式
- 清晰的职责划分
- 单向依赖
- 易于测试和维护

### 2. 抽象工厂模式
- OSAL层提供统一接口
- 不同平台实现相同接口
- 易于跨平台移植

### 3. 生产者-消费者模式
- 消息队列解耦模块
- CAN接收/发送任务
- 协议转换任务

### 4. 观察者模式
- 看门狗监控任务
- 心跳上报机制

### 5. 策略模式
- 通道切换策略
- 故障恢复策略

---

## 性能指标

| 指标 | 目标值 | 实际值 |
|------|--------|--------|
| CAN消息延迟 | < 10ms | 待测试 |
| 以太网延迟 | < 50ms | 待测试 |
| 命令处理时间 | < 100ms | 待测试 |
| 状态上报周期 | 1-10s可配置 | 5s |
| 内存占用 | < 128MB | 待测试 |
| CPU占用(空闲) | < 5% | 待测试 |
| 故障恢复时间 | < 3s | 待测试 |

---

## 配置参数

### 系统配置 (system_config.h)

```c
/* CAN配置 */
#define CAN_INTERFACE           "can0"
#define CAN_BAUDRATE            500000
#define CAN_RX_QUEUE_DEPTH      100
#define CAN_TX_QUEUE_DEPTH      100

/* 以太网配置 */
#define SERVER_IP_ADDRESS       "192.168.1.100"
#define IPMI_PORT               623
#define ETHERNET_TIMEOUT_MS     5000

/* UART配置 */
#define UART_DEVICE             "/dev/ttyS0"
#define UART_BAUDRATE           115200

/* 任务优先级 */
#define PRIORITY_CRITICAL       10
#define PRIORITY_HIGH           50
#define PRIORITY_NORMAL         100
#define PRIORITY_LOW            150
```

---

## 总结

CSPD-BSP是一个设计良好的嵌入式软件框架，具有：

✅ **清晰的分层架构** - 4层设计，职责明确  
✅ **良好的可移植性** - OSAL抽象层支持跨平台  
✅ **完善的错误处理** - 多层次错误检测和恢复  
✅ **高可靠性设计** - 看门狗、心跳、通信冗余  
✅ **模块化设计** - 低耦合、高内聚  
✅ **62个公共接口** - 功能完整、接口清晰  

下一步需要进行全面的单元测试和集成测试，确保系统的可靠性和稳定性。
