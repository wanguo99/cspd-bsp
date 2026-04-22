# 文件命名规范

## 概述

本文档定义 CSPD-BSP 项目的文件命名规范，确保代码库的一致性和可维护性。

## 命名原则

### 1. 使用小写字母和下划线
- 所有文件名使用小写字母
- 单词之间使用下划线 `_` 分隔
- 不使用驼峰命名或连字符

**正确**：
```
service_platform.c
hal_can.h
os_task.c
```

**错误**：
```
ServicePlatform.c
hal-can.h
osTask.c
```

### 2. 避免冗余的平台后缀
- 实现文件已经在平台目录下，不需要再加平台后缀
- 接口文件不包含平台信息

**正确**：
```
service/linux/service_satellite.c    # 已在 linux/ 目录
service/rtos/service_satellite.c     # 已在 rtos/ 目录
```

**错误**：
```
service/linux/service_satellite_linux.c    # 冗余
service/rtos/service_satellite_rtos.c      # 冗余
```

### 3. 模块前缀
每个模块使用统一的前缀：

| 模块 | 前缀 | 示例 |
|------|------|------|
| OSAL | `os_` | `os_task.c`, `os_queue.c` |
| HAL | `hal_` | `hal_can.c`, `hal_network.c` |
| Service | `service_` | `service_satellite.c` |
| Apps | 无前缀 | `can_gateway.c` |
| Config | 无前缀 | `system_config.h` |

### 4. 接口与实现
- 接口文件（头文件）：`模块前缀_功能.h`
- 实现文件（源文件）：`模块前缀_功能.c`

**示例**：
```
osal/inc/osapi_task.h          # OSAL 任务接口
osal/linux/os_task.c           # Linux 实现
osal/rtos/os_task.c            # RTOS 实现

hal/inc/hal_can.h              # HAL CAN 接口
hal/linux/hal_can.c            # Linux 实现

service/inc/service_power.h    # Service 电源接口
service/linux/service_power.c  # Linux 实现
```

## 目录结构与命名

### OSAL 层
```
osal/
├── inc/                    # 接口定义
│   ├── osal.h             # 主头文件
│   ├── common_types.h     # 通用类型
│   ├── osapi_task.h       # 任务接口
│   ├── osapi_queue.h      # 队列接口
│   ├── osapi_mutex.h      # 互斥锁接口
│   ├── osapi_clock.h      # 时间接口
│   ├── osapi_heap.h       # 内存接口
│   ├── osapi_error.h      # 错误处理接口
│   └── osapi_log.h        # 日志接口
└── linux/                 # Linux 实现
    ├── os_task.c          # 任务实现
    ├── os_queue.c         # 队列实现
    ├── os_mutex.c         # 互斥锁实现
    ├── os_clock.c         # 时间实现
    ├── os_heap.c          # 内存实现
    ├── os_error.c         # 错误处理实现
    ├── os_log.c           # 日志实现
    └── os_init.c          # 初始化
```

**命名规则**：
- 接口文件：`osapi_*.h`（表示 OS API）
- 实现文件：`os_*.c`（简洁）

### HAL 层
```
hal/
├── inc/                   # 接口定义
│   ├── hal_can.h         # CAN 接口
│   ├── hal_network.h     # 网络接口
│   └── hal_serial.h      # 串口接口
└── linux/                # Linux 实现
    ├── hal_can.c         # CAN 实现
    ├── hal_network.c     # 网络实现
    └── hal_serial.c      # 串口实现
```

**命名规则**：
- 接口文件：`hal_*.h`
- 实现文件：`hal_*.c`

### Service 层
```
service/
├── inc/                        # 接口定义
│   ├── service_satellite.h    # 卫星平台接口
│   ├── service_payload_bmc.h  # BMC 载荷接口
│   ├── service_payload_linux.h # Linux 载荷接口
│   └── service_power.h        # 电源管理接口
└── linux/                     # Linux 实现
    ├── service_satellite.c    # 卫星平台实现
    ├── service_payload_bmc.c  # BMC 载荷实现
    ├── service_payload_linux.c # Linux 载荷实现
    └── service_power.c        # 电源管理实现
```

**命名规则**：
- 接口文件：`service_*.h`
- 实现文件：`service_*.c`

### Apps 层
```
apps/
├── can_gateway/
│   ├── can_gateway.h
│   └── can_gateway.c
└── protocol_converter/
    ├── protocol_converter.h
    └── protocol_converter.c
```

**命名规则**：
- 应用模块不使用前缀
- 使用描述性名称

### Config 层
```
config/
├── system_config.h
└── can_protocol.h
```

**命名规则**：
- 配置文件不使用前缀
- 使用描述性名称

## 特殊情况

### 1. 平台特定的头文件
如果需要平台特定的头文件（不常见），使用目录区分：

```
hal/inc/hal_can.h              # 通用接口
hal/linux/hal_can_linux.h      # Linux 特定（如果需要）
hal/rtos/hal_can_rtos.h        # RTOS 特定（如果需要）
```

### 2. 测试文件
测试文件使用 `_test` 后缀：

```
tests/
├── os_task_test.c
├── hal_can_test.c
└── service_power_test.c
```

### 3. 示例文件
示例文件使用 `_example` 后缀：

```
examples/
├── can_gateway_example.c
└── power_control_example.c
```

## 文档命名

### Markdown 文档
- 使用大写字母和下划线
- 描述性名称

```
docs/
├── README.md
├── ARCHITECTURE.md
├── QUICKSTART.md
├── DEPLOYMENT.md
├── SERVICE_ARCHITECTURE.md
└── history/
    ├── SERVICE_LAYER_REFACTOR_V2.md
    └── CMAKE_GUIDE.md
```

### 配置文件
- CMake：`CMakeLists.txt`
- Shell：`build.sh`
- Git：`.gitignore`

## 命名检查清单

在添加新文件时，检查以下项：

- [ ] 文件名全部小写
- [ ] 使用下划线分隔单词
- [ ] 使用正确的模块前缀
- [ ] 实现文件在平台目录下，不包含平台后缀
- [ ] 接口文件在 inc/ 目录下
- [ ] 文件名清晰描述其功能
- [ ] 与同模块其他文件命名风格一致

## 重命名历史

### 2024-04-22: Service 层文件重命名

**第一次重命名** - 移除冗余的平台后缀：

| 旧文件名 | 新文件名 | 原因 |
|---------|---------|------|
| `service_platform_linux.c` | `service_platform.c` | 已在 linux/ 目录 |
| `service_payload_bmc_linux.c` | `service_payload_bmc.c` | 已在 linux/ 目录 |
| `service_payload_linux_linux.c` | `service_payload_linux.c` | 避免重复 |
| `service_power_linux.c` | `service_power.c` | 已在 linux/ 目录 |

**第二次重命名** - 强调卫星平台：

| 旧文件名 | 新文件名 | 原因 |
|---------|---------|------|
| `service_platform.h` | `service_satellite.h` | 明确指向卫星平台 |
| `service_platform.c` | `service_satellite.c` | 与头文件保持一致 |

## 总结

良好的文件命名规范：
- 提高代码可读性
- 便于文件查找和导航
- 减少命名冲突
- 保持项目一致性
- 简化构建系统配置

遵循这些规范，使 CSPD-BSP 项目更加专业和易于维护。
