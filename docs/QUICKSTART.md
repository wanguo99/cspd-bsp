# Lite-cFS 快速入门指南

## 1. 编译和运行示例

### Linux平台

```bash
cd lite-cfs-framework
make
./build/gateway_demo
```

**预期输出:**
```
========================================
  Lite-cFS 网关管理系统示例
========================================

OSAL Lite v1.0.0 initialized (POSIX)
消息队列创建成功

[CAN Gateway] 任务启动
[Device Manager] 任务启动
[Status Reporter] 任务启动
所有任务创建成功
按 Ctrl+C 退出

[CAN Gateway] 发送命令: 查询载荷状态 (seq=0)
[Device Manager] 收到命令: 查询载荷状态 (seq=0)
[Device Manager] 发送状态: 载荷在线, CPU=45%, 内存=60% (seq=0)
[Status Reporter] 上报状态: 载荷在线, CPU=45%, 内存=60% (seq=0)
...
```

## 2. 创建你的第一个应用

### 步骤1: 创建应用文件

创建 `apps/my_app/my_app.c`:

```c
#include "osal.h"

void my_app_task(void)
{
    OS_printf("[My App] 任务启动\n");
    
    while (1)
    {
        OS_printf("[My App] 运行中...\n");
        OS_TaskDelay(1000);  // 延时1秒
    }
}

int main(void)
{
    osal_id_t task_id;
    
    OS_API_Init();
    
    OS_TaskCreate(&task_id, "MY_APP",
                  my_app_task, NULL,
                  16384, 100, 0);
    
    OS_IdleLoop();  // 永不返回
    return 0;
}
```

### 步骤2: 编译

```bash
gcc -I osal/inc apps/my_app/my_app.c \
    osal/posix/*.c -lpthread -lrt \
    -o build/my_app
```

### 步骤3: 运行

```bash
./build/my_app
```

## 3. 任务间通信示例

```c
#include "osal.h"

osal_id_t g_queue;

// 发送任务
void sender_task(void)
{
    char msg[64];
    int count = 0;
    
    while (1)
    {
        snprintf(msg, sizeof(msg), "消息 #%d", count++);
        OS_QueuePut(g_queue, msg, sizeof(msg), 0);
        OS_printf("[发送] %s\n", msg);
        OS_TaskDelay(1000);
    }
}

// 接收任务
void receiver_task(void)
{
    char msg[64];
    uint32 size;
    
    while (1)
    {
        if (OS_QueueGet(g_queue, msg, sizeof(msg), &size, OS_PEND) == OS_SUCCESS)
        {
            OS_printf("[接收] %s\n", msg);
        }
    }
}

int main(void)
{
    osal_id_t task_id;
    
    OS_API_Init();
    
    // 创建队列
    OS_QueueCreate(&g_queue, "MSG_QUEUE", 10, 64, 0);
    
    // 创建任务
    OS_TaskCreate(&task_id, "SENDER", sender_task, NULL, 16384, 100, 0);
    OS_TaskCreate(&task_id, "RECEIVER", receiver_task, NULL, 16384, 100, 0);
    
    OS_IdleLoop();
    return 0;
}
```

## 4. 互斥锁保护共享资源

```c
#include "osal.h"

int g_counter = 0;
osal_id_t g_mutex;

void worker_task(void)
{
    while (1)
    {
        OS_MutexLock(g_mutex);
        
        // 临界区
        int temp = g_counter;
        OS_TaskDelay(1);  // 模拟处理
        g_counter = temp + 1;
        OS_printf("计数器: %d\n", g_counter);
        
        OS_MutexUnlock(g_mutex);
        
        OS_TaskDelay(100);
    }
}

int main(void)
{
    osal_id_t task_id;
    
    OS_API_Init();
    
    // 创建互斥锁
    OS_MutexCreate(&g_mutex, "COUNTER_MUTEX", 0);
    
    // 创建多个工作任务
    OS_TaskCreate(&task_id, "WORKER1", worker_task, NULL, 16384, 100, 0);
    OS_TaskCreate(&task_id, "WORKER2", worker_task, NULL, 16384, 100, 0);
    
    OS_IdleLoop();
    return 0;
}
```

## 5. 网络通信示例

### TCP通信

```c
#include "osal.h"

void tcp_server_task(void)
{
    osal_id_t server_sock, client_sock;
    char buffer[256];
    int32 ret;
    
    // 创建Socket
    OS_SocketOpen(&server_sock, 0, OS_SOCK_STREAM);
    
    // 绑定端口
    OS_SocketBind(server_sock, "0.0.0.0", 8080);
    
    // 监听
    OS_SocketListen(server_sock, 5);
    OS_printf("TCP监听端口 8080\n");
    
    while (1)
    {
        // 接受连接
        ret = OS_SocketAccept(server_sock, &client_sock, NULL, OS_PEND);
        if (ret == OS_SUCCESS)
        {
            OS_printf("客户端已连接\n");
            
            // 接收数据
            ret = OS_SocketRecv(client_sock, buffer, sizeof(buffer), 5000);
            if (ret > 0)
            {
                OS_printf("收到: %s\n", buffer);
                
                // 发送响应
                OS_SocketSend(client_sock, "OK\n", 3, 1000);
            }
            
            OS_SocketClose(client_sock);
        }
    }
}
```

### UDP通信

```c
void udp_task(void)
{
    osal_id_t sock;
    char buffer[256];
    char addr[32];
    uint16 port;
    int32 ret;
    
    // 创建UDP Socket
    OS_SocketOpen(&sock, 0, OS_SOCK_DGRAM);
    OS_SocketBind(sock, "0.0.0.0", 9000);
    
    OS_printf("UDP监听端口 9000\n");
    
    while (1)
    {
        // 接收数据报
        ret = OS_SocketRecvFrom(sock, buffer, sizeof(buffer),
                                addr, &port, OS_PEND);
        if (ret > 0)
        {
            OS_printf("收到来自 %s:%d 的数据: %s\n", addr, port, buffer);
            
            // 回复
            OS_SocketSendTo(sock, "ACK", 3, addr, port);
        }
    }
}
```

## 6. 串口通信示例

```c
void serial_task(void)
{
    osal_id_t serial;
    OS_SerialConfig_t config = {
        .baudrate = 115200,
        .databits = 8,
        .stopbits = 1,
        .parity = 0  // 无校验
    };
    char buffer[128];
    int32 ret;
    
    // 打开串口
    ret = OS_SerialOpen(&serial, "/dev/ttyS0", &config);
    if (ret != OS_SUCCESS)
    {
        OS_printf("打开串口失败\n");
        return;
    }
    
    OS_printf("串口已打开\n");
    
    while (1)
    {
        // 接收数据
        ret = OS_SerialRead(serial, buffer, sizeof(buffer), 1000);
        if (ret > 0)
        {
            OS_printf("收到: %.*s\n", ret, buffer);
            
            // 回显
            OS_SerialWrite(serial, buffer, ret, 1000);
        }
    }
}
```

## 7. 常见问题

### Q: 如何调整任务优先级?

A: 优先级范围1-255，数字越小优先级越高。建议:
- 高优先级(1-50): 实时任务、中断处理
- 中优先级(51-150): 业务逻辑
- 低优先级(151-255): 后台任务

### Q: 队列满了怎么办?

A: `OS_QueuePut`会阻塞直到有空间。可以:
1. 增加队列深度
2. 加快消费速度
3. 使用非阻塞模式检查队列状态

### Q: 如何处理错误?

A: 所有API返回int32错误码:
```c
int32 ret = OS_TaskCreate(...);
if (ret != OS_SUCCESS)
{
    OS_printf("错误: %s\n", OS_GetErrorName(ret));
    // 处理错误
}
```

### Q: 可以在中断中调用OSAL API吗?

A: 不建议。OSAL API可能会阻塞，应该在中断中设置标志，由任务处理。

## 8. 下一步

- 查看 [examples/gateway_demo.c](../examples/gateway_demo.c) 完整示例
- 阅读 [COMPARISON.md](COMPARISON.md) 了解与cFS的区别
- 参考 [PORTING.md](PORTING.md) 移植到其他平台
- 查看 [API参考](API.md) 完整API文档

## 9. 获取帮助

- 查看源码注释
- 参考NASA cFS文档: https://cfs.gsfc.nasa.gov
- 提交Issue到项目仓库
