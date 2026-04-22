# RTOS 平台实现

本目录用于存放 RTOS（如 FreeRTOS、RTEMS）平台的 Service 层实现。

## 待实现的服务

### 1. service_satellite.c
- 卫星平台接口服务的 RTOS 实现
- 基于 RTOS 任务和消息队列
- CAN 总线通信

### 2. service_payload_bmc.c
- BMC 载荷通信服务的 RTOS 实现
- 网络和串口通信
- IPMI/Redfish 协议支持

### 3. service_payload_linux.c
- 通用 Linux 载荷通信服务的 RTOS 实现
- SSH/HTTP API 客户端
- 远程命令执行

### 4. service_power.c
- 电源管理服务的 RTOS 实现
- GPIO 控制
- 传感器读取
- 定时任务

## 实现要点

1. **任务管理**：使用 OSAL 任务接口，确保跨平台兼容
2. **内存管理**：注意 RTOS 的内存限制，避免动态分配
3. **同步机制**：使用 OSAL 互斥锁和信号量
4. **中断处理**：CAN 和 GPIO 中断需要特殊处理
5. **资源限制**：优化代码大小和栈使用

## 移植步骤

1. 复制 Linux 实现作为模板
2. 替换 Linux 特定的系统调用
3. 使用 OSAL 和 HAL 接口
4. 测试和优化
5. 更新 CMakeLists.txt

## 注意事项

- 所有接口定义在 `service/inc/` 中，保持不变
- 只需实现平台相关的部分
- 确保与 Linux 实现行为一致
