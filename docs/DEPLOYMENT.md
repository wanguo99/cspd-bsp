# 部署指南

## 硬件准备

### 1. 管理板硬件

- **SoC**: AM6254 或其他支持Linux的ARM SoC
- **CAN接口**: 支持CAN 2.0A/B
- **以太网**: 10/100/1000 Mbps
- **串口**: UART (115200 bps)
- **存储**: 至少256MB Flash
- **内存**: 至少512MB RAM

### 2. 连接拓扑

```
┌─────────────┐
│  卫星平台    │
│             │
│  CAN_H ─────┼──────┐
│  CAN_L ─────┼──────┤
└─────────────┘      │
                     │
              ┌──────┴──────┐
              │   管理板     │
              │  (AM6254)   │
              │             │
              │  ETH ───────┼────── 载荷 ETH
              │  UART ──────┼────── 载荷 UART
              └─────────────┘
```

## 软件部署

### 步骤1: 准备Linux系统

推荐使用Yocto或Buildroot构建定制Linux系统。

**最小系统要求:**
- Linux Kernel 4.19+
- glibc 2.28+
- SocketCAN支持
- 基础工具: busybox, can-utils

### 步骤2: 交叉编译

```bash
# 设置交叉编译工具链
export CROSS_COMPILE=arm-linux-gnueabihf-
export CC=${CROSS_COMPILE}gcc

# 修改Makefile
# CC = arm-linux-gnueabihf-gcc

# 编译
make clean
make
```

### 步骤3: 部署到目标板

```bash
# 通过SCP传输
scp build/cspd-bsp root@192.168.1.10:/usr/bin/
scp config/*.h root@192.168.1.10:/etc/cspd-bsp/

# 或通过NFS挂载
mount -t nfs 192.168.1.100:/export/rootfs /mnt
cp build/cspd-bsp /mnt/usr/bin/
```

### 步骤4: 配置系统服务

创建systemd服务文件 `/etc/systemd/system/cspd-bsp.service`:

```ini
[Unit]
Description=Satellite-Server Bridge Service
After=network.target can.target

[Service]
Type=simple
ExecStart=/usr/bin/cspd-bsp
Restart=always
RestartSec=10
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
```

启用服务:

```bash
systemctl daemon-reload
systemctl enable cspd-bsp
systemctl start cspd-bsp
```

### 步骤5: 配置CAN接口

创建 `/etc/systemd/network/80-can.network`:

```ini
[Match]
Name=can0

[CAN]
BitRate=500K
RestartSec=100ms
```

或在启动脚本中配置:

```bash
#!/bin/sh
# /etc/init.d/setup-can

ip link set can0 type can bitrate 500000
ip link set can0 up
```

## 配置调优

### 1. 内核参数优化

编辑 `/etc/sysctl.conf`:

```bash
# 网络优化
net.core.rmem_max = 8388608
net.core.wmem_max = 8388608
net.ipv4.tcp_rmem = 4096 87380 8388608
net.ipv4.tcp_wmem = 4096 65536 8388608

# CAN优化
net.can.rx_queue_len = 1000
```

应用配置:

```bash
sysctl -p
```

### 2. 实时性优化

```bash
# 设置CPU调度策略
chrt -f 50 /usr/bin/cspd-bsp

# 或在代码中设置
# sched_setscheduler(0, SCHED_FIFO, &param);
```

### 3. 看门狗配置

启用硬件看门狗:

```bash
# 加载看门狗驱动
modprobe softdog

# 配置超时时间
echo 30 > /sys/class/watchdog/watchdog0/timeout
```

## 测试验证

### 1. 功能测试

```bash
# 测试CAN通信
candump can0 &
cansend can0 100#0110000100000000

# 测试以太网
ping 192.168.1.100
telnet 192.168.1.100 623

# 测试串口
echo "test" > /dev/ttyS0
cat /dev/ttyS0
```

### 2. 压力测试

```bash
# CAN压力测试
cangen can0 -g 1 -I 100 -L 8 -D r -n 10000

# 监控系统资源
top
free -m
iostat 1
```

### 3. 稳定性测试

```bash
# 长时间运行测试 (24小时+)
./build/cspd-bsp &
PID=$!

# 监控进程
while true; do
    if ! ps -p $PID > /dev/null; then
        echo "进程异常退出"
        break
    fi
    sleep 60
done
```

## 故障排查

### 问题1: CAN接口无法启动

**症状**: `ip link set can0 up` 失败

**解决方案**:
```bash
# 检查驱动
lsmod | grep can
modprobe can
modprobe can_raw

# 检查设备树
ls /sys/class/net/
dmesg | grep can
```

### 问题2: 内存泄漏

**症状**: 内存占用持续增长

**解决方案**:
```bash
# 使用valgrind检测
valgrind --leak-check=full ./build/cspd-bsp

# 或使用mtrace
export MALLOC_TRACE=/tmp/mtrace.log
./build/cspd-bsp
mtrace ./build/cspd-bsp /tmp/mtrace.log
```

### 问题3: 实时性不足

**症状**: CAN消息延迟过大

**解决方案**:
```bash
# 使用RT_PREEMPT内核
# 或调整任务优先级
chrt -f 99 -p $(pidof cspd-bsp)

# 禁用CPU频率调节
cpupower frequency-set -g performance
```

## 生产环境建议

### 1. 安全加固

```bash
# 禁用不必要的服务
systemctl disable bluetooth
systemctl disable wifi

# 配置防火墙
iptables -A INPUT -i can0 -j ACCEPT
iptables -A INPUT -p tcp --dport 623 -j ACCEPT
iptables -A INPUT -j DROP
```

### 2. 日志管理

```bash
# 配置logrotate
cat > /etc/logrotate.d/cspd-bsp << EOF
/var/log/cspd-bsp.log {
    daily
    rotate 7
    compress
    missingok
    notifempty
}
EOF
```

### 3. 监控告警

```bash
# 使用monit监控
cat > /etc/monit/conf.d/cspd-bsp << EOF
check process cspd-bsp with pidfile /var/run/cspd-bsp.pid
    start program = "/usr/bin/systemctl start cspd-bsp"
    stop program = "/usr/bin/systemctl stop cspd-bsp"
    if cpu > 80% for 5 cycles then alert
    if memory > 200 MB then alert
EOF
```

## 维护升级

### 远程升级流程

```bash
# 1. 备份当前版本
cp /usr/bin/cspd-bsp /usr/bin/cspd-bsp.bak

# 2. 停止服务
systemctl stop cspd-bsp

# 3. 上传新版本
scp build/cspd-bsp root@target:/usr/bin/

# 4. 启动服务
systemctl start cspd-bsp

# 5. 验证
systemctl status cspd-bsp
journalctl -u cspd-bsp -f
```

### 回滚方案

```bash
# 如果新版本有问题
systemctl stop cspd-bsp
cp /usr/bin/cspd-bsp.bak /usr/bin/cspd-bsp
systemctl start cspd-bsp
```

## 性能基准

| 指标 | 目标值 | 实测值 |
|------|--------|--------|
| CAN消息延迟 | < 10ms | 5-8ms |
| 命令处理时间 | < 100ms | 50-80ms |
| 内存占用 | < 128MB | 80-100MB |
| CPU占用(空闲) | < 5% | 2-3% |
| 启动时间 | < 5s | 3s |
| MTBF | > 10000h | 待测试 |

## 技术支持

- 查看日志: `journalctl -u cspd-bsp -f`
- 查看统计: 程序自动每30秒打印
- 调试模式: 修改 `system_config.h` 中的 `DEBUG_MODE`
