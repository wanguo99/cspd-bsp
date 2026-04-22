# 快速参考

## 编译和运行

```bash
# 编译
make

# 配置CAN接口
sudo ip link set can0 type can bitrate 500000
sudo ip link set can0 up

# 运行
sudo ./build/cspd-bsp
```

## CAN协议速查

### CAN ID
- `0x100`: 卫星平台 → 管理板
- `0x200`: 管理板 → 卫星平台

### 消息类型
| 值 | 类型 | 说明 |
|----|------|------|
| 0x01 | CMD_REQ | 命令请求 |
| 0x02 | CMD_RESP | 命令响应 |
| 0x03 | STATUS_QUERY | 状态查询 |
| 0x04 | STATUS_REPORT | 状态上报 |
| 0x05 | HEARTBEAT | 心跳 |
| 0x06 | ERROR | 错误 |

### 命令类型
| 值 | 命令 | 说明 |
|----|------|------|
| 0x10 | POWER_ON | 载荷上电 |
| 0x11 | POWER_OFF | 载荷下电 |
| 0x12 | RESET | 载荷重启 |
| 0x20 | QUERY_STATUS | 查询状态 |
| 0x21 | QUERY_TEMP | 查询温度 |
| 0x22 | QUERY_VOLTAGE | 查询电压 |

### 状态码
| 值 | 状态 | 说明 |
|----|------|------|
| 0x00 | OK | 成功 |
| 0x01 | ERROR | 错误 |
| 0x02 | TIMEOUT | 超时 |
| 0x03 | INVALID_CMD | 无效命令 |
| 0x04 | PAYLOAD_OFFLINE | 载荷离线 |
| 0x05 | COMM_ERROR | 通信错误 |

## CAN测试命令

```bash
# 发送上电命令
cansend can0 100#0110000100000000

# 发送下电命令
cansend can0 100#0111000200000000

# 发送重启命令
cansend can0 100#0112000300000000

# 发送查询状态命令
cansend can0 100#0120000400000000

# 监控CAN消息
candump can0

# 查看CAN统计
ip -s link show can0
```

## 配置文件位置

| 配置项 | 文件 | 位置 |
|--------|------|------|
| CAN接口 | system_config.h | `#define CAN_INTERFACE "can0"` |
| CAN波特率 | system_config.h | `#define CAN_BAUDRATE 500000` |
| 服务器IP | system_config.h | `#define SERVER_IP_ADDRESS "192.168.1.100"` |
| IPMI端口 | system_config.h | `#define IPMI_PORT 623` |
| UART设备 | system_config.h | `#define UART_DEVICE "/dev/ttyS0"` |
| UART波特率 | system_config.h | `#define UART_BAUDRATE 115200` |

## 常用命令

### 系统管理
```bash
# 启动服务
sudo systemctl start cspd-bsp

# 停止服务
sudo systemctl stop cspd-bsp

# 查看状态
sudo systemctl status cspd-bsp

# 查看日志
journalctl -u cspd-bsp -f

# 查看实时日志
tail -f /var/log/cspd-bsp.log
```

### CAN接口管理
```bash
# 启动CAN
sudo ip link set can0 up

# 停止CAN
sudo ip link set can0 down

# 设置波特率
sudo ip link set can0 type can bitrate 500000

# 查看CAN状态
ip -details link show can0

# 查看CAN错误
ip -s -s link show can0
```

### 调试
```bash
# 查看进程
ps aux | grep cspd-bsp

# 查看内存占用
top -p $(pidof cspd-bsp)

# 查看网络连接
netstat -anp | grep cspd-bsp

# 查看打开的文件
lsof -p $(pidof cspd-bsp)
```

## OSAL API速查

### 任务管理
```c
// 创建任务
OS_TaskCreate(&task_id, "TASK_NAME", task_func, NULL, 
              stack_size, priority, 0);

// 延时
OS_TaskDelay(1000);  // 延时1秒

// 获取当前任务ID
osal_id_t id = OS_TaskGetId();
```

### 消息队列
```c
// 创建队列
OS_QueueCreate(&queue_id, "QUEUE_NAME", depth, msg_size, 0);

// 发送消息
OS_QueuePut(queue_id, &msg, sizeof(msg), 0);

// 接收消息(阻塞)
OS_QueueGet(queue_id, &msg, sizeof(msg), &size, OS_PEND);

// 接收消息(超时1秒)
OS_QueueGet(queue_id, &msg, sizeof(msg), &size, 1000);
```

### 互斥锁
```c
// 创建互斥锁
OS_MutexCreate(&mutex_id, "MUTEX_NAME", 0);

// 加锁
OS_MutexLock(mutex_id);

// 解锁
OS_MutexUnlock(mutex_id);
```

## HAL API速查

### CAN驱动
```c
// 初始化
hal_can_config_t config = {
    .interface = "can0",
    .baudrate = 500000,
    .rx_timeout = 1000,
    .tx_timeout = 1000
};
HAL_CAN_Init(&config, &handle);

// 发送
HAL_CAN_Send(handle, &frame);

// 接收
HAL_CAN_Recv(handle, &frame, 1000);
```

### 服务器通信
```c
// 初始化
hal_server_config_t config = {
    .channel = SERVER_CHANNEL_ETHERNET,
    .ethernet.ip_addr = "192.168.1.100",
    .ethernet.port = 623,
    .ethernet.timeout_ms = 5000
};
HAL_Server_Init(&config, &handle);

// 发送
HAL_Server_Send(handle, data, size);

// 接收
HAL_Server_Recv(handle, buffer, size, timeout);

// 切换通道
HAL_Server_SwitchChannel(handle, SERVER_CHANNEL_UART);
```

## 故障排查

| 问题 | 检查命令 | 解决方案 |
|------|----------|----------|
| CAN接口无法启动 | `ip link show can0` | `sudo modprobe can && sudo modprobe can_raw` |
| 权限不足 | `ls -l /dev/can*` | 使用sudo运行 |
| 服务器连接失败 | `ping 192.168.1.100` | 检查网络和IP配置 |
| 进程崩溃 | `dmesg \| tail` | 查看内核日志 |
| 内存泄漏 | `valgrind ./build/cspd-bsp` | 使用valgrind检测 |

## 性能监控

```bash
# CPU和内存
top -p $(pidof cspd-bsp)

# 网络流量
iftop -i eth0

# CAN流量
candump can0 -t A | pv -l > /dev/null

# 系统调用
strace -p $(pidof cspd-bsp)

# 函数调用
perf record -p $(pidof cspd-bsp)
perf report
```

## 重要文件路径

| 文件 | 路径 |
|------|------|
| 可执行文件 | `/usr/bin/cspd-bsp` |
| 配置文件 | `/etc/cspd-bsp/` |
| 日志文件 | `/var/log/cspd-bsp.log` |
| 缓存文件 | `/var/cache/cspd-bsp.cache` |
| systemd服务 | `/etc/systemd/system/cspd-bsp.service` |
| 看门狗设备 | `/dev/watchdog` |

## 联系方式

- 文档: 查看 `docs/` 目录
- 架构: `ARCHITECTURE.md`
- 部署: `docs/DEPLOYMENT.md`
- 快速开始: `docs/QUICKSTART.md`
