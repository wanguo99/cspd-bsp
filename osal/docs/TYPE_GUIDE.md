# OSAL 数据类型使用指南

本文档详细说明 OSAL 库中各种数据类型的使用场景和最佳实践。

## 核心原则

**禁止使用 C 语言基本类型**：
- ❌ `int`, `long`, `unsigned int`, `unsigned long`
- ❌ `char` (用于字符串时)
- ✅ 使用 OSAL 定义的固定宽度类型和语义化类型

## 类型分类和使用场景

### 1. 固定宽度整数类型

用于需要明确位宽的场景（协议、硬件寄存器、二进制数据）。

```c
/* 有符号整数 */
int8_t    value8;     // 8位有符号整数 (-128 ~ 127)
int16_t   value16;    // 16位有符号整数 (-32768 ~ 32767)
int32_t   value32;    // 32位有符号整数 (-2^31 ~ 2^31-1)
int64_t   value64;    // 64位有符号整数 (-2^63 ~ 2^63-1)

/* 无符号整数 */
uint8_t   byte;       // 8位无符号整数 (0 ~ 255)
uint16_t  word;       // 16位无符号整数 (0 ~ 65535)
uint32_t  dword;      // 32位无符号整数 (0 ~ 2^32-1)
uint64_t  qword;      // 64位无符号整数 (0 ~ 2^64-1)
```

**使用场景**：
- 网络协议字段（CAN ID、IP地址、端口号）
- 硬件寄存器访问
- 二进制数据解析
- 位域操作

**示例**：
```c
/* CAN 消息 ID */
uint32_t can_id = 0x123;

/* 硬件寄存器 */
volatile uint32_t *reg = (uint32_t *)0x40000000;

/* 协议数据包 */
struct packet {
    uint16_t length;
    uint8_t  type;
    uint8_t  data[256];
};
```

### 2. 字符串类型

用于文本数据（设备名、日志消息、配置字符串）。

```c
str_t device_name[64];           // 设备名称
str_t log_message[256];          // 日志消息
const str_t *interface = "can0"; // 字符串常量
str_t parity = 'N';              // 单个字符
```

**使用场景**：
- 设备名称、文件路径
- 日志消息、错误描述
- 配置参数（"115200", "8N1"）
- 与标准 C 库交互（`strcpy`, `strlen`, `fopen`）

**重要**：
- `str_t` 底层是 `char`，与标准 C 库完全兼容
- 用于文本数据，区别于 `uint8_t`（二进制数据）

### 3. 平台相关大小类型

用于内存大小、数组索引、缓冲区长度等平台相关的场景。

```c
osal_size_t   buffer_size;    // 缓冲区大小（无符号）
osal_ssize_t  bytes_read;     // 读取字节数（有符号，可表示错误）
osal_uintptr_t addr;          // 指针大小的整数
osal_intptr_t  offset;        // 有符号指针偏移
osal_ptrdiff_t diff;          // 指针差值
```

**平台适配**：
- 16位平台：16位类型
- 32位平台：32位类型
- 64位平台：64位类型

**使用场景**：
- 内存分配大小：`OSAL_Malloc(osal_size_t size)`
- 数组索引：`for (osal_size_t i = 0; i < count; i++)`
- 指针运算：`osal_uintptr_t addr = OSAL_PTR_TO_UINT(ptr)`
- 返回值（可能为负）：`osal_ssize_t ret = OSAL_Read(...)`

**示例**：
```c
/* 内存分配 */
osal_size_t size = 1024;
void *buffer = OSAL_Malloc(size);

/* 指针转整数（地址计算） */
osal_uintptr_t addr = OSAL_PTR_TO_UINT(buffer);
if (OSAL_IS_ALIGNED(addr, 64)) {
    /* 地址已对齐 */
}

/* 读取操作（返回负数表示错误） */
osal_ssize_t bytes = OSAL_Read(fd, buffer, size);
if (bytes < 0) {
    /* 错误处理 */
}
```

### 4. 文件偏移类型

用于文件操作中的偏移量（支持大文件）。

```c
osal_off_t offset;  // 文件偏移量（64位，支持大文件）
```

**使用场景**：
- 文件定位：`OSAL_Lseek(fd, offset, SEEK_SET)`
- 大文件支持（>2GB）

**示例**：
```c
/* 定位到文件末尾 */
osal_off_t file_size = OSAL_Lseek(fd, 0, SEEK_END);

/* 读取大文件 */
osal_off_t offset = 5000000000LL;  // 5GB 偏移
OSAL_Lseek(fd, offset, SEEK_SET);
```

### 5. 时间类型

用于时间戳、延迟、超时等时间相关操作。

```c
osal_time_t  timestamp;   // 时间戳（秒，64位，无2038年问题）
osal_usec_t  delay_us;    // 微秒延迟
osal_nsec_t  delay_ns;    // 纳秒延迟
```

**使用场景**：
- 时间戳：`osal_time_t now = OSAL_GetTime()`
- 高精度延迟：`OSAL_DelayUsec(osal_usec_t usec)`
- 性能测量：`osal_nsec_t elapsed = end_ns - start_ns`

**示例**：
```c
/* 获取当前时间 */
osal_time_t now = OSAL_GetTime();

/* 微秒级延迟 */
osal_usec_t delay = 1000;  // 1ms
OSAL_DelayUsec(delay);

/* 性能测量 */
osal_nsec_t start = OSAL_GetTimeNs();
do_work();
osal_nsec_t end = OSAL_GetTimeNs();
osal_nsec_t elapsed = end - start;
```

### 6. 原子类型

用于多线程环境下的无锁数据结构和状态标志。

```c
osal_atomic_uint32_t counter;     // 原子计数器
osal_atomic_bool_t   flag;        // 原子布尔标志
osal_atomic_int32_t  ref_count;   // 引用计数
```

**使用场景**：
- 引用计数
- 状态标志（启动/停止）
- 无锁队列/栈
- 统计计数器

**示例**：
```c
/* 原子计数器 */
osal_atomic_uint32_t request_count = 0;

void handle_request(void) {
    OSAL_ATOMIC_INC(&request_count);
    /* 处理请求 */
}

/* 原子标志 */
osal_atomic_bool_t shutdown_flag = false;

void task_entry(void *arg) {
    while (!OSAL_ATOMIC_LOAD(&shutdown_flag)) {
        do_work();
    }
}

void shutdown(void) {
    OSAL_ATOMIC_STORE(&shutdown_flag, true);
}

/* CAS 操作 */
uint32_t expected = 0;
uint32_t desired = 1;
if (OSAL_ATOMIC_CAS(&counter, &expected, desired)) {
    /* 成功：counter 从 0 变为 1 */
}
```

### 7. OSAL 对象 ID 类型

用于 OSAL 对象的唯一标识符。

```c
osal_id_t task_id;    // 任务 ID
osal_id_t queue_id;   // 队列 ID
osal_id_t mutex_id;   // 互斥锁 ID
osal_id_t timer_id;   // 定时器 ID
```

**使用场景**：
- 任务创建：`OSAL_TaskCreate(&task_id, ...)`
- 队列操作：`OSAL_QueueGet(queue_id, ...)`
- 互斥锁：`OSAL_MutexLock(mutex_id)`

**示例**：
```c
osal_id_t task_id;
int32_t ret = OSAL_TaskCreate(&task_id, "worker", task_entry, NULL);
if (ret != OSAL_SUCCESS) {
    /* 错误处理 */
}
```

### 8. 布尔类型

用于逻辑判断和标志位。

```c
bool is_ready;        // 状态标志
bool has_error;       // 错误标志
```

**使用场景**：
- 状态标志
- 函数返回值（成功/失败）
- 条件判断

**示例**：
```c
bool is_initialized = false;

int32_t init(void) {
    if (is_initialized) {
        return OSAL_ERR_ALREADY_EXISTS;
    }
    /* 初始化代码 */
    is_initialized = true;
    return OSAL_SUCCESS;
}
```

## 类型转换辅助宏

### 指针与整数转换

```c
/* 指针转整数 */
void *ptr = get_buffer();
osal_uintptr_t addr = OSAL_PTR_TO_UINT(ptr);

/* 整数转指针 */
osal_uintptr_t reg_addr = 0x40000000;
volatile uint32_t *reg = OSAL_UINT_TO_PTR(reg_addr);
```

### 对齐操作

```c
/* 向上对齐到64字节边界 */
osal_size_t size = 100;
osal_size_t aligned_size = OSAL_ALIGN_UP(size, 64);  // 128

/* 向下对齐 */
osal_uintptr_t addr = 0x1234;
osal_uintptr_t aligned_addr = OSAL_ALIGN_DOWN(addr, 16);  // 0x1230

/* 检查是否对齐 */
if (OSAL_IS_ALIGNED(ptr, 64)) {
    /* 指针已对齐到64字节边界 */
}
```

### 数组和结构体操作

```c
/* 数组元素个数 */
uint32_t array[10];
osal_size_t count = OSAL_ARRAY_SIZE(array);  // 10

/* 结构体成员偏移 */
struct config {
    uint32_t id;
    str_t name[32];
};
osal_size_t offset = OSAL_OFFSETOF(struct config, name);

/* 通过成员指针获取结构体指针 */
struct config cfg;
str_t *name_ptr = cfg.name;
struct config *cfg_ptr = OSAL_CONTAINER_OF(name_ptr, struct config, name);
```

### 位操作

```c
uint32_t flags = 0;

/* 设置位 */
OSAL_BIT_SET(flags, 3);      // flags |= (1 << 3)

/* 清除位 */
OSAL_BIT_CLR(flags, 3);      // flags &= ~(1 << 3)

/* 测试位 */
if (OSAL_BIT_TEST(flags, 3)) {
    /* 位3已设置 */
}

/* 翻转位 */
OSAL_BIT_TOGGLE(flags, 3);   // flags ^= (1 << 3)
```

## 字节序转换

用于网络协议和跨平台数据交换。

```c
/* 主机序 -> 网络序（大端） */
uint16_t port_host = 8080;
uint16_t port_net = OSAL_HTONS(port_host);

uint32_t ip_host = 0xC0A80001;  // 192.168.0.1
uint32_t ip_net = OSAL_HTONL(ip_host);

/* 网络序 -> 主机序 */
uint16_t port = OSAL_NTOHS(port_net);
uint32_t ip = OSAL_NTOHL(ip_net);

/* 64位转换 */
uint64_t value64 = 0x123456789ABCDEF0;
uint64_t value64_net = OSAL_HTONLL(value64);
```

## 对齐和打包

用于硬件寄存器、DMA 缓冲区、网络协议等。

```c
/* 对齐到64字节边界（缓存行大小） */
struct dma_buffer {
    uint8_t data[1024];
} OSAL_ALIGNED(64);

/* 紧凑结构体（无填充） */
struct OSAL_PACKED protocol_header {
    uint16_t length;
    uint8_t  type;
    uint8_t  flags;
};

/* 避免伪共享（多线程） */
struct thread_data {
    osal_atomic_uint32_t counter;
    uint8_t padding[OSAL_CACHE_LINE_SIZE - sizeof(osal_atomic_uint32_t)];
} OSAL_ALIGNED(OSAL_CACHE_LINE_SIZE);
```

## 返回值类型

所有 OSAL 函数返回 `int32_t` 状态码。

```c
int32_t ret;

/* 成功 */
ret = OSAL_TaskCreate(...);
if (ret == OSAL_SUCCESS) {
    /* 成功 */
}

/* 错误处理 */
ret = OSAL_QueueGet(queue_id, buffer, size, timeout);
if (ret != OSAL_SUCCESS) {
    switch (ret) {
        case OSAL_ERR_TIMEOUT:
            /* 超时 */
            break;
        case OSAL_ERR_INVALID_ID:
            /* 无效ID */
            break;
        default:
            /* 其他错误 */
            break;
    }
}
```

## 常见错误和最佳实践

### ❌ 错误示例

```c
/* 错误1：使用基本类型 */
int count;                    // 应使用 int32_t 或 osal_size_t
unsigned int flags;           // 应使用 uint32_t
long offset;                  // 应使用 int32_t/int64_t/osal_off_t

/* 错误2：字符串使用 uint8_t */
uint8_t name[32];             // 应使用 str_t
strcpy(name, "device");       // 类型不匹配

/* 错误3：指针转整数不安全 */
int addr = (int)ptr;          // 64位平台会截断

/* 错误4：硬编码对齐 */
if ((int)ptr % 64 == 0) {     // 应使用 OSAL_IS_ALIGNED
}
```

### ✅ 正确示例

```c
/* 正确1：使用固定宽度类型 */
int32_t count;
uint32_t flags;
osal_off_t offset;

/* 正确2：字符串使用 str_t */
str_t name[32];
OSAL_Strcpy(name, "device");

/* 正确3：安全的指针转换 */
osal_uintptr_t addr = OSAL_PTR_TO_UINT(ptr);

/* 正确4：使用对齐宏 */
if (OSAL_IS_ALIGNED(ptr, 64)) {
    /* 已对齐 */
}
```

## 平台兼容性矩阵

| 类型 | 16位平台 | 32位平台 | 64位平台 | 用途 |
|------|---------|---------|---------|------|
| `int8_t` | 1字节 | 1字节 | 1字节 | 固定宽度 |
| `int16_t` | 2字节 | 2字节 | 2字节 | 固定宽度 |
| `int32_t` | 4字节 | 4字节 | 4字节 | 固定宽度 |
| `int64_t` | 8字节 | 8字节 | 8字节 | 固定宽度 |
| `osal_size_t` | 2字节 | 4字节 | 8字节 | 平台相关 |
| `osal_uintptr_t` | 2字节 | 4字节 | 8字节 | 指针大小 |
| `osal_off_t` | 4字节 | 8字节 | 8字节 | 文件偏移 |
| `osal_time_t` | 8字节 | 8字节 | 8字节 | 时间戳 |

## 总结

1. **永远不要使用 C 基本类型**（`int`, `long`, `unsigned`）
2. **固定宽度类型**用于协议、硬件、二进制数据
3. **平台相关类型**用于内存大小、数组索引
4. **str_t** 用于文本，**uint8_t** 用于二进制
5. **原子类型**用于多线程无锁编程
6. **使用辅助宏**进行类型转换和对齐操作
7. **编译时断言**确保类型大小正确

遵循这些规则，可以确保代码在 16/32/64 位平台、Linux/RTOS 之间完全兼容。
