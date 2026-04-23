# OSAL系统调用封装完成报告

## 概述

已完成对整个工程中所有系统调用的封装，全部封装到OSAL层中。所有API使用明确大小的数据类型（uint32、int32等），不使用size_t。

## 新增的OSAL API模块

### 1. 文件I/O API (osapi_file.h)

**头文件**: `osal/inc/osapi_file.h`  
**实现文件**: `osal/linux/os_file.c`

**主要API**:
- `OS_FileOpen()` - 打开文件，替代 open()
- `OS_FileClose()` - 关闭文件，替代 close()
- `OS_FileRead()` - 读取文件，替代 read()
- `OS_FileWrite()` - 写入文件，替代 write()
- `OS_FileSeek()` - 文件定位，替代 lseek()
- `OS_FileSetFlags()` - 设置文件标志，替代 fcntl()
- `OS_FileGetFlags()` - 获取文件标志，替代 fcntl()
- `OS_FileIoctl()` - 文件控制，替代 ioctl()
- `OS_FileSelect()` - 等待文件I/O就绪，替代 select()

**特性**:
- 使用osal_id_t作为文件描述符
- 支持阻塞/非阻塞模式
- 统一的错误码返回
- 线程安全

### 2. 串口通信API (osapi_serial.h)

**头文件**: `osal/inc/osapi_serial.h`  
**实现文件**: `osal/linux/os_serial.c`

**主要API**:
- `OS_SerialOpen()` - 打开串口，替代 open() + tcgetattr() + tcsetattr()
- `OS_SerialClose()` - 关闭串口
- `OS_SerialRead()` - 读取串口数据，支持超时
- `OS_SerialWrite()` - 写入串口数据，支持超时
- `OS_SerialFlush()` - 刷新串口缓冲区，替代 tcflush()
- `OS_SerialSetConfig()` - 设置串口配置
- `OS_SerialGetConfig()` - 获取串口配置

**配置结构**:
```c
typedef struct
{
    uint32 baud_rate;    /* 波特率: 9600, 115200等 */
    uint8  data_bits;    /* 数据位: 5, 6, 7, 8 */
    uint8  stop_bits;    /* 停止位: 1, 2 */
    uint8  parity;       /* 校验位: NONE, ODD, EVEN */
    uint8  flow_control; /* 流控制: NONE, HARDWARE, SOFTWARE */
} os_serial_config_t;
```

**特性**:
- 封装了所有termios相关调用
- 支持超时读写
- 预定义常用波特率
- 线程安全

### 3. 网络API扩展 (osapi_network.h)

**头文件**: `osal/inc/osapi_network.h` (已扩展)  
**实现文件**: `osal/linux/os_network.c` (新建)

**新增API**:
- `OS_SocketSetOpt()` - 设置Socket选项，替代 setsockopt()
- `OS_SocketGetOpt()` - 获取Socket选项，替代 getsockopt()
- `OS_SocketSelect()` - 等待Socket I/O就绪，替代 select()

**支持的Socket选项**:
- `OS_SO_REUSEADDR` - 地址重用
- `OS_SO_KEEPALIVE` - 保持连接
- `OS_SO_RCVTIMEO` - 接收超时
- `OS_SO_SNDTIMEO` - 发送超时
- `OS_SO_RCVBUF` - 接收缓冲区大小
- `OS_SO_SNDBUF` - 发送缓冲区大小
- `OS_SO_ERROR` - 获取错误状态
- `OS_TCP_NODELAY` - 禁用Nagle算法

**完整实现的网络API**:
- `OS_SocketOpen()` - 创建Socket，替代 socket()
- `OS_SocketClose()` - 关闭Socket，替代 close()
- `OS_SocketBind()` - 绑定地址，替代 bind()
- `OS_SocketConnect()` - 连接服务器，替代 connect()
- `OS_SocketListen()` - 监听连接，替代 listen()
- `OS_SocketAccept()` - 接受连接，替代 accept()
- `OS_SocketSend()` - 发送数据，替代 send()
- `OS_SocketRecv()` - 接收数据，替代 recv()
- `OS_SocketSendTo()` - 发送UDP数据，替代 sendto()
- `OS_SocketRecvFrom()` - 接收UDP数据，替代 recvfrom()

### 4. 信号处理API (osapi_signal.h)

**头文件**: `osal/inc/osapi_signal.h`  
**实现文件**: `osal/linux/os_signal.c`

**主要API**:
- `OS_SignalRegister()` - 注册信号处理函数，替代 signal() / sigaction()
- `OS_SignalIgnore()` - 忽略信号
- `OS_SignalDefault()` - 恢复默认处理
- `OS_SignalBlock()` - 阻塞信号，替代 sigprocmask()
- `OS_SignalUnblock()` - 解除阻塞

**支持的信号**:
- `OS_SIGNAL_INT` - SIGINT (Ctrl+C)
- `OS_SIGNAL_TERM` - SIGTERM (终止)
- `OS_SIGNAL_HUP` - SIGHUP (挂起)
- `OS_SIGNAL_QUIT` - SIGQUIT (退出)
- `OS_SIGNAL_USR1` - SIGUSR1 (用户自定义1)
- `OS_SIGNAL_USR2` - SIGUSR2 (用户自定义2)

## 设计原则

### 1. 统一的数据类型
- 所有API使用 `uint32`, `int32`, `uint8`, `uint16` 等明确大小的类型
- 不使用 `size_t`, `ssize_t` 等平台相关类型
- 使用 `osal_id_t` 作为所有资源句柄

### 2. 统一的错误处理
- 成功返回 `OS_SUCCESS` (0)
- 错误返回正数错误码 (`OS_ERROR`, `OS_INVALID_POINTER`, `OS_ERROR_TIMEOUT` 等)
- 数据传输函数返回实际字节数(>=0)或错误码(<0)

### 3. 线程安全
- 所有资源表使用互斥锁保护
- 支持多线程并发访问

### 4. 资源管理
- 使用资源表管理所有打开的文件、串口、Socket
- 自动分配和回收资源ID
- 防止资源泄漏

## 需要迁移的代码

以下文件中的系统调用需要替换为OSAL API：

### HAL层
1. **hal/linux/hal_can_linux.c**
   - `socket()` → `OS_SocketOpen()`
   - `bind()` → `OS_SocketBind()`
   - `close()` → `OS_SocketClose()`
   - `read()` → `OS_FileRead()` 或 `OS_SocketRecv()`
   - `write()` → `OS_FileWrite()` 或 `OS_SocketSend()`
   - `ioctl()` → `OS_FileIoctl()`
   - `setsockopt()` → `OS_SocketSetOpt()`

2. **hal/linux/hal_serial_linux.c**
   - `open()` → `OS_SerialOpen()`
   - `close()` → `OS_SerialClose()`
   - `read()` → `OS_SerialRead()`
   - `write()` → `OS_SerialWrite()`
   - `tcgetattr()`, `tcsetattr()`, `cfsetispeed()`, `cfsetospeed()` → 已封装在 `OS_SerialOpen()`
   - `select()` → 已内置在 `OS_SerialRead()` 的超时机制中

3. **hal/linux/hal_network_linux.c**
   - `socket()` → `OS_SocketOpen()`
   - `connect()` → `OS_SocketConnect()`
   - `close()` → `OS_SocketClose()`
   - `send()` → `OS_SocketSend()`
   - `sendto()` → `OS_SocketSendTo()`
   - `recv()` → `OS_SocketRecv()`
   - `recvfrom()` → `OS_SocketRecvFrom()`
   - `select()` → `OS_SocketSelect()`
   - `fcntl()` → `OS_FileSetFlags()`

### Service层
1. **service/linux/watchdog.c**
   - `open()` → `OS_FileOpen()`
   - `close()` → `OS_FileClose()`
   - `write()` → `OS_FileWrite()` (已修复)
   - `ioctl()` → `OS_FileIoctl()`

### Application层
1. **apps/protocol_converter/payload_service.c**
   - `socket()` → `OS_SocketOpen()`
   - `connect()` → `OS_SocketConnect()`
   - `close()` → `OS_SocketClose()` / `OS_SerialClose()`
   - `send()` → `OS_SocketSend()`
   - `recv()` → `OS_SocketRecv()`
   - `select()` → `OS_SocketSelect()`
   - `open()` → `OS_SerialOpen()`
   - `read()` → `OS_SerialRead()`
   - `write()` → `OS_SerialWrite()`
   - `fcntl()` → `OS_FileSetFlags()`
   - `tcgetattr()`, `tcsetattr()` 等 → 已封装在 `OS_SerialOpen()`

2. **main.c, examples/gateway_demo.c**
   - `signal()` → `OS_SignalRegister()`

## 使用示例

### 文件I/O示例
```c
osal_id_t fd;
uint8 buffer[256];
int32 ret;

/* 打开文件 */
ret = OS_FileOpen(&fd, "/dev/watchdog", OS_FILE_MODE_WRITE, OS_FILE_FLAG_NONE);
if (ret != OS_SUCCESS) {
    OS_printf("打开文件失败\n");
    return ret;
}

/* 写入数据 */
ret = OS_FileWrite(fd, "V", 1);
if (ret < 0) {
    OS_printf("写入失败\n");
}

/* 关闭文件 */
OS_FileClose(fd);
```

### 串口通信示例
```c
osal_id_t serial;
os_serial_config_t config;
uint8 buffer[256];
int32 ret;

/* 配置串口 */
config.baud_rate = OS_SERIAL_BAUD_115200;
config.data_bits = OS_SERIAL_DATA_8;
config.stop_bits = OS_SERIAL_STOP_1;
config.parity = OS_SERIAL_PARITY_NONE;
config.flow_control = OS_SERIAL_FLOW_NONE;

/* 打开串口 */
ret = OS_SerialOpen(&serial, "/dev/ttyS0", &config);
if (ret != OS_SUCCESS) {
    OS_printf("打开串口失败\n");
    return ret;
}

/* 读取数据（超时1000ms） */
ret = OS_SerialRead(serial, buffer, sizeof(buffer), 1000);
if (ret > 0) {
    OS_printf("读取了 %d 字节\n", ret);
}

/* 写入数据 */
ret = OS_SerialWrite(serial, "Hello", 5, 1000);

/* 关闭串口 */
OS_SerialClose(serial);
```

### 网络通信示例
```c
osal_id_t sock;
int32 ret;
int32 keepalive = 1;
int32 nodelay = 1;

/* 创建TCP Socket */
ret = OS_SocketOpen(&sock, 0, OS_SOCK_STREAM);
if (ret != OS_SUCCESS) {
    OS_printf("创建socket失败\n");
    return ret;
}

/* 设置Socket选项 */
OS_SocketSetOpt(sock, OS_SOL_SOCKET, OS_SO_KEEPALIVE, &keepalive, sizeof(keepalive));
OS_SocketSetOpt(sock, OS_SOL_TCP, OS_TCP_NODELAY, &nodelay, sizeof(nodelay));

/* 连接服务器（超时5000ms） */
ret = OS_SocketConnect(sock, "192.168.1.100", 8080, 5000);
if (ret != OS_SUCCESS) {
    OS_printf("连接失败\n");
    OS_SocketClose(sock);
    return ret;
}

/* 发送数据 */
ret = OS_SocketSend(sock, "Hello", 5, 1000);

/* 接收数据 */
uint8 buffer[256];
ret = OS_SocketRecv(sock, buffer, sizeof(buffer), 1000);

/* 关闭Socket */
OS_SocketClose(sock);
```

### 信号处理示例
```c
void signal_handler(int32 signum)
{
    OS_printf("收到信号: %d\n", signum);
    /* 执行清理操作 */
}

int main(void)
{
    /* 注册信号处理函数 */
    OS_SignalRegister(OS_SIGNAL_INT, signal_handler);
    OS_SignalRegister(OS_SIGNAL_TERM, signal_handler);
    
    /* 程序主循环 */
    while (running) {
        /* ... */
    }
    
    return 0;
}
```

## 编译配置

已更新 `osal/CMakeLists.txt`，添加了以下源文件：
- `linux/os_file.c`
- `linux/os_serial.c`
- `linux/os_network.c`
- `linux/os_signal.c`

## 优势

1. **平台无关性**: 应用层代码不再直接调用系统API，便于移植到其他操作系统
2. **类型安全**: 使用明确大小的数据类型，避免跨平台问题
3. **统一接口**: 所有I/O操作使用一致的API风格
4. **错误处理**: 统一的错误码体系，便于调试
5. **资源管理**: 自动管理资源生命周期，防止泄漏
6. **线程安全**: 内置互斥保护，支持多线程环境

## 下一步工作

1. 将HAL层、Service层、Application层的代码迁移到新的OSAL API
2. 删除直接的系统调用
3. 测试所有功能确保正常工作
4. 更新相关文档

## 总结

已完成对整个工程的系统调用封装，创建了4个新的OSAL模块：
- 文件I/O (9个API)
- 串口通信 (7个API)
- 网络通信扩展 (13个API，包括新增的3个)
- 信号处理 (5个API)

所有API均使用明确大小的数据类型，不使用size_t。现在可以开始将现有代码迁移到新的OSAL API。
