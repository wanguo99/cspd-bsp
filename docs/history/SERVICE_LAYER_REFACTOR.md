# Service 层架构重构

## 日期
2024-04-22

## 背景
原架构中 `hal_server` 模块混合了硬件驱动和业务逻辑，违反了分层原则。HAL 层应该只包含纯硬件接口，不应包含通道管理、自动切换等业务逻辑。

## 问题分析
1. **职责不清**：`hal_server` 既管理硬件（网络/串口），又实现业务逻辑（通道切换、故障转移）
2. **可移植性差**：业务逻辑与硬件驱动耦合，难以适配不同硬件平台
3. **可测试性差**：无法独立测试业务逻辑和硬件驱动

## 解决方案
创建 Service 层，实现四层架构：

```
Apps (应用层)
    ↓
Service (服务层) - 业务逻辑
    ↓
HAL (硬件抽象层) - 硬件驱动
    ↓
OSAL (操作系统抽象层) - OS 接口
```

## 架构变更

### 1. HAL 层拆分
**删除**：
- `hal/inc/hal_server.h`
- `hal/linux/hal_server_linux.c`

**新增**：
- `hal/inc/hal_network.h` - 纯网络硬件接口（TCP socket）
- `hal/linux/hal_network_linux.c` - Linux 网络驱动实现
- `hal/inc/hal_serial.h` - 串口硬件接口（从 OSAL 移入）
- `hal/linux/hal_serial_linux.c` - Linux 串口驱动实现

### 2. Service 层创建
**新增**：
- `service/inc/payload_service.h` - 载荷通信服务接口
- `service/linux/payload_service_linux.c` - 载荷服务实现
- `service/CMakeLists.txt` - Service 层编译配置

### 3. 职责划分

#### HAL 层（硬件驱动）
- **hal_network**：TCP socket 连接、发送、接收
- **hal_serial**：串口打开、配置、读写
- **hal_can**：CAN 总线收发

#### Service 层（业务逻辑）
- **payload_service**：
  - 通道管理（网络/串口）
  - 自动故障切换
  - 连接状态监控
  - 重试机制
  - 统一的载荷通信接口

## 接口变更

### 应用层调用变更
**旧接口**（HAL 层）：
```c
#include "hal_server.h"

hal_server_handle_t handle;
hal_server_config_t config;
HAL_Server_Init(&config, &handle);
HAL_Server_Send(handle, data, len);
HAL_Server_Recv(handle, buf, size, timeout);
HAL_Server_IsConnected(handle);
HAL_Server_SwitchChannel(handle, channel);
```

**新接口**（Service 层）：
```c
#include "payload_service.h"

payload_service_handle_t handle;
payload_service_config_t config;
PayloadService_Init(&config, &handle);
PayloadService_Send(handle, data, len);
PayloadService_Recv(handle, buf, size, timeout);
PayloadService_IsConnected(handle);
PayloadService_SwitchChannel(handle, channel);
```

### 配置结构变更
**旧配置**：
```c
typedef struct {
    server_channel_t channel;  // 初始通道
    struct {
        const char *ip_addr;
        uint16 port;
        uint32 timeout_ms;
    } ethernet;
    struct {
        const char *device;
        uint32 baudrate;
        uint32 timeout_ms;
    } uart;
} hal_server_config_t;
```

**新配置**：
```c
typedef struct {
    struct {
        const char *ip_addr;
        uint16 port;
        uint32 timeout_ms;
    } ethernet;
    struct {
        const char *device;
        uint32 baudrate;
        uint32 timeout_ms;
    } uart;
    bool auto_switch;      // 是否自动切换通道
    uint32 retry_count;    // 重试次数
} payload_service_config_t;
```

## 实现细节

### payload_service 功能
1. **通道管理**：
   - 优先使用网络通道
   - 网络失败自动切换到串口
   - 支持手动切换通道

2. **故障处理**：
   - 连接失败自动重试
   - 超时自动切换通道
   - 记录故障统计

3. **状态监控**：
   - 实时连接状态
   - 当前使用通道
   - 通信统计信息

### HAL 层简化
- **hal_network**：只负责 TCP socket 操作
- **hal_serial**：只负责串口硬件操作
- 不包含任何业务逻辑

## 编译系统更新
1. 根 CMakeLists.txt 添加 `add_subdirectory(service)`
2. 主程序链接添加 `service` 库
3. Apps CMakeLists.txt 添加 `service` 依赖

## 影响范围
**修改文件**：
- `apps/protocol_converter/protocol_converter.c` - 使用新接口
- `apps/protocol_converter/protocol_converter.h` - 更新头文件引用
- `CMakeLists.txt` - 添加 Service 层
- `apps/CMakeLists.txt` - 添加 service 依赖
- `hal/CMakeLists.txt` - 更新源文件列表

**新增文件**：
- `service/` 目录及所有文件

**删除文件**：
- `hal/inc/hal_server.h`
- `hal/linux/hal_server_linux.c`

## 优势
1. **职责清晰**：每层只负责自己的功能
2. **易于移植**：HAL 层纯硬件，Service 层跨平台
3. **易于测试**：可独立测试各层
4. **易于扩展**：添加新通道只需修改 HAL 和 Service 层
5. **代码复用**：Service 层可用于其他项目

## 后续工作
1. 实现 Service 层的单元测试
2. 添加通道切换的日志记录
3. 实现更智能的故障检测算法
4. 支持更多通信通道（如 USB、SPI）

## 总结
通过引入 Service 层，实现了业务逻辑与硬件驱动的分离，使架构更加清晰、可维护。这是一次重要的架构优化，为后续功能扩展奠定了良好基础。
