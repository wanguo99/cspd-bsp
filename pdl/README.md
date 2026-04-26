# PDL (外设驱动层)

## 模块概述

PDL (Peripheral Driver Layer) 是外设驱动层，统一管理卫星平台、载荷（BMC/Linux）、MCU等外设。

**设计理念**：
- 管理板为核心，卫星/载荷/MCU统一抽象为外设
- 完全平台无关，通过HAL层访问底层硬件
- 使用XConfig配置外设硬件接口
- 提供统一的外设管理接口

**支持的外设**：
- MCU外设（微控制器）
- 卫星平台接口
- BMC载荷（基板管理控制器）
- Linux载荷（操作系统载荷）

## 编译说明

### 快速开始

```bash
# 在项目根目录编译整个项目（包含PDL）
./build.sh              # Release模式
./build.sh -d           # Debug模式
```

### 单独编译PDL模块

```bash
# 方法1: 使用CMake直接编译
mkdir -p output/build && cd output/build
cmake ../.. -DCMAKE_BUILD_TYPE=Release
make pdl -j$(nproc)
cd ../..

# 方法2: 在已配置的构建目录中编译
cd output/build
make pdl -j$(nproc)
cd ../..
```

### 支持的编译参数

#### CMake配置参数

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `CMAKE_BUILD_TYPE` | STRING | Release | 编译类型：Release/Debug |
| `PLATFORM` | STRING | native | 目标平台：native/generic-linux/rtems |

#### 平台配置

PDL模块根据 `PLATFORM` 参数选择不同的实现：

**本地编译（Linux）**：
```bash
cmake ../.. -DPLATFORM=native
```
- 使用Linux平台实现
- 支持MCU/卫星/BMC外设

**交叉编译（通用Linux）**：
```bash
cmake ../.. \
    -DPLATFORM=generic-linux \
    -DCMAKE_C_COMPILER=arm-linux-gnueabihf-gcc
```
- 使用Linux平台实现
- 适用于嵌入式Linux

**RTOS平台（预留）**：
```bash
cmake ../.. -DPLATFORM=rtems
```
- 使用RTEMS平台实现（待实现）

### 配置编译参数

#### 示例1: Debug模式编译

```bash
cd output/build
cmake ../.. \
    -DCMAKE_BUILD_TYPE=Debug \
    -DPLATFORM=native
make pdl -j$(nproc)
```

#### 示例2: 交叉编译ARM平台

```bash
cd output/build
cmake ../.. \
    -DCMAKE_BUILD_TYPE=Release \
    -DPLATFORM=generic-linux \
    -DCMAKE_C_COMPILER=arm-linux-gnueabihf-gcc
make pdl -j$(nproc)
```

#### 示例3: 修改编译选项

编辑 `pdl/CMakeLists.txt`：
```cmake
target_compile_options(pdl PRIVATE
    -Wall
    -Wextra
    -Werror
    -DPDL_DEBUG_MODE    # 启用调试模式
)
```

### 编译输出

```
output/
├── build/
│   └── lib/
│       └── libpdl.a           # PDL静态库
└── target/
    └── bin/
        └── sample_app         # 示例应用（使用PDL）
```

### 常用编译命令

```bash
# 完整编译流程
./build.sh -d                   # Debug模式编译所有

# 仅编译PDL库
cd output/build && make pdl -j$(nproc) && cd ../..

# 查看PDL编译配置
cd output/build && cmake -L ../.. | grep -E "PLATFORM|PDL" && cd ../..

# 查看PDL模块信息
cd output/build && cmake ../.. 2>&1 | grep -A 5 "Peripheral Driver Layer" && cd ../..

# 清理并重新编译
./build.sh -c && ./build.sh

# 查看编译日志
cat output/build.log | grep -A 10 "PDL"
```

## 模块结构

```
pdl/
├── include/                    # 公共接口头文件
│   ├── pdl_mcu.h               # MCU外设接口
│   ├── pdl_satellite.h         # 卫星平台接口
│   ├── pdl_bmc.h               # BMC载荷接口
│   └── pdl_linux.h             # Linux载荷接口
└── src/
    ├── pdl_mcu/                # MCU外设驱动
    │   ├── pdl_mcu.c           # MCU管理
    │   ├── pdl_mcu_can.c       # MCU CAN通信
    │   ├── pdl_mcu_serial.c    # MCU串口通信
    │   └── pdl_mcu_protocol.c  # MCU协议处理
    ├── pdl_satellite/          # 卫星平台服务
    │   ├── pdl_satellite.c     # 卫星接口管理
    │   └── pdl_satellite_can.c # 卫星CAN通信
    └── pdl_bmc/                # BMC载荷服务
        ├── pdl_bmc.c           # BMC管理
        ├── pdl_bmc_redfish.c   # Redfish接口
        └── pdl_bmc_ipmi.c      # IPMI接口
```

## 依赖关系

**PDL依赖**：
- OSAL层：操作系统抽象
- HAL层：硬件驱动
- XConfig层：硬件配置

**被依赖**：
- Apps层：应用层

**重要**：PDL必须保持完全平台无关，通过HAL层访问硬件。

## 使用示例

### 在应用中使用PDL

**CMakeLists.txt配置**：
```cmake
# 链接PDL接口库（获取头文件路径）
target_link_libraries(your_app PUBLIC pmc::pdl_public_api)

# 链接PDL实现库（运行时链接）
target_link_libraries(your_app PRIVATE pmc::pdl)
```

**代码中使用**：
```c
#include <pdl_mcu.h>
#include <pdl_satellite.h>
#include <pdl_bmc.h>

int main(void)
{
    /* 初始化MCU外设 */
    pdl_mcu_config_t mcu_cfg = {
        .name = "mcu0",
        .interface_type = PDL_INTERFACE_CAN,
        .can_interface = "can0",
        .baudrate = 500000
    };
    pdl_mcu_handle_t mcu_handle;
    PDL_MCU_Init(&mcu_cfg, &mcu_handle);
    
    /* 发送命令到MCU */
    uint8 cmd_data[] = {0x01, 0x02, 0x03};
    PDL_MCU_SendCommand(&mcu_handle, 0x100, cmd_data, sizeof(cmd_data));
    
    /* 初始化卫星接口 */
    pdl_satellite_config_t sat_cfg = {
        .can_interface = "can1",
        .baudrate = 1000000
    };
    pdl_satellite_handle_t sat_handle;
    PDL_Satellite_Init(&sat_cfg, &sat_handle);
    
    /* 接收卫星遥测数据 */
    uint8 telemetry[64];
    uint32 len;
    PDL_Satellite_ReceiveTelemetry(&sat_handle, telemetry, &len, 1000);
    
    /* 初始化BMC载荷 */
    pdl_bmc_config_t bmc_cfg = {
        .ip_address = "192.168.1.100",
        .port = 443,
        .protocol = PDL_BMC_PROTOCOL_REDFISH
    };
    pdl_bmc_handle_t bmc_handle;
    PDL_BMC_Init(&bmc_cfg, &bmc_handle);
    
    /* 查询BMC状态 */
    pdl_bmc_status_t status;
    PDL_BMC_GetStatus(&bmc_handle, &status);
    LOG_INFO("APP", "BMC Power: %s", 
             status.power_state == PDL_BMC_POWER_ON ? "ON" : "OFF");
    
    return 0;
}
```

## 开发指南

### 添加新的外设类型

1. 在 `include/` 添加接口头文件（如 `pdl_sensor.h`）
2. 在 `src/` 创建实现目录（如 `src/pdl_sensor/`）
3. 实现外设驱动（如 `pdl_sensor.c`）
4. 在 `CMakeLists.txt` 的 `PDL_SOURCES` 中添加源文件
5. 在 `tests/src/pdl/` 添加单元测试

### 外设驱动实现模板

```c
/* pdl_sensor.c */
#include "pdl_sensor.h"
#include <hal_i2c.h>
#include <osal.h>

int32 PDL_Sensor_Init(const pdl_sensor_config_t *config, 
                      pdl_sensor_handle_t *handle)
{
    /* 参数检查 */
    if (config == NULL || handle == NULL) {
        return OS_ERROR;
    }
    
    /* 初始化HAL层接口 */
    hal_i2c_config_t i2c_cfg = {
        .device = config->i2c_device,
        .address = config->i2c_address,
        .speed = 400000
    };
    
    int32 ret = HAL_I2C_Init(&i2c_cfg, &handle->i2c_handle);
    if (ret != OS_SUCCESS) {
        LOG_ERROR("PDL_SENSOR", "I2C init failed");
        return ret;
    }
    
    /* 配置传感器 */
    uint8 reg_data = 0x01;
    HAL_I2C_Write(&handle->i2c_handle, 0x00, &reg_data, 1);
    
    LOG_INFO("PDL_SENSOR", "Sensor initialized: %s", config->name);
    return OS_SUCCESS;
}

int32 PDL_Sensor_Read(pdl_sensor_handle_t *handle, 
                      pdl_sensor_data_t *data)
{
    /* 读取传感器数据 */
    uint8 raw_data[4];
    int32 ret = HAL_I2C_Read(&handle->i2c_handle, 0x01, raw_data, 4);
    if (ret != OS_SUCCESS) {
        return ret;
    }
    
    /* 解析数据 */
    data->temperature = (raw_data[0] << 8) | raw_data[1];
    data->humidity = (raw_data[2] << 8) | raw_data[3];
    
    return OS_SUCCESS;
}
```

### 编码规范（重要）

**必须遵守**：
- ✅ 使用HAL层接口访问硬件：`HAL_CAN_*()`, `HAL_Serial_*()`
- ❌ 禁止直接调用OSAL系统调用：`OSAL_socket()`, `OSAL_open()`
- ✅ 使用OSAL日志：`LOG_INFO()`, `LOG_ERROR()`
- ❌ 禁止使用：`printf()`, `fprintf()`
- ✅ 使用XConfig配置：`XConfig_GetMCUConfig()`
- ❌ 禁止硬编码配置

**示例**：
```c
/* ✅ 正确 */
hal_can_config_t can_cfg = {
    .interface = "can0",
    .baudrate = 500000
};
HAL_CAN_Init(&can_cfg, &handle->can_handle);
LOG_INFO("PDL_MCU", "MCU initialized");

/* ❌ 错误 */
int sockfd = OSAL_socket(PF_CAN, SOCK_RAW, CAN_RAW);  // 禁止
printf("MCU initialized\n");                           // 禁止
```

## 配置说明

PDL使用XConfig配置外设硬件接口：

```c
/* 从XConfig获取MCU配置 */
const xconfig_mcu_t *mcu_cfg = XConfig_GetMCUConfig("mcu0");
if (mcu_cfg != NULL) {
    pdl_mcu_config_t pdl_cfg = {
        .name = mcu_cfg->name,
        .interface_type = mcu_cfg->interface_type,
        .can_interface = mcu_cfg->can_interface.device,
        .baudrate = mcu_cfg->can_interface.baudrate
    };
    PDL_MCU_Init(&pdl_cfg, &handle);
}
```

## 测试

```bash
# 编译测试
./build.sh -d

# 运行示例应用（使用PDL）
./output/target/bin/sample_app

# 查看PDL日志
./output/target/bin/sample_app 2>&1 | grep PDL
```

## 常见问题

**Q: PDL和HAL的区别？**
- HAL：硬件抽象层，封装底层驱动（CAN/串口/网络）
- PDL：外设驱动层，管理外设（MCU/卫星/BMC），使用HAL接口

**Q: 如何添加新的通信协议？**
```c
// 1. 在 src/pdl_xxx/ 添加协议处理文件
// 2. 实现协议编解码函数
// 3. 在主驱动中调用协议处理函数
```

**Q: PDL可以直接调用OSAL系统调用吗？**
```c
/* ❌ 禁止 - PDL必须通过HAL层访问硬件 */
int sockfd = OSAL_socket(...);  // 禁止

/* ✅ 正确 - 使用HAL层接口 */
HAL_CAN_Init(&can_cfg, &handle);  // 正确
```

**Q: 如何启用PDL调试日志？**
```c
// 在代码中设置日志级别
OSAL_LogSetLevel(OSAL_LOG_DEBUG);
```

**Q: 如何处理外设通信错误？**
```c
int32 ret = PDL_MCU_SendCommand(&handle, cmd_id, data, len);
if (ret != OS_SUCCESS) {
    LOG_ERROR("APP", "MCU command failed: %d", ret);
    // 重试或错误处理
}
```

## 参考文档

- [PDL详细文档](docs/README.md)
- [HAL层文档](../hal/README.md)
- [XConfig文档](../xconfig/README.md)
- [编码规范](../docs/CODING_STANDARDS.md)
