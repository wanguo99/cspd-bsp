# XConfig 更新日志

## 2026-04-24 - 初始版本

### 新增功能

1. **硬件配置库框架**
   - 类似Linux设备树的硬件配置管理系统
   - 以外设为单位进行配置（MCU/BMC/传感器/存储等）
   - 支持多平台、多产品、多版本配置管理

2. **核心数据结构**
   - `xconfig_mcu_cfg_t` - MCU外设配置（内嵌通信接口）
   - `xconfig_bmc_cfg_t` - BMC外设配置（支持主备双通道）
   - `xconfig_satellite_cfg_t` - 卫星平台接口配置
   - `xconfig_sensor_cfg_t` - 传感器外设配置
   - `xconfig_storage_cfg_t` - 存储设备配置
   - `xconfig_board_config_t` - 板级配置（顶层）

3. **支持的接口类型**
   - CAN总线
   - UART串口
   - I2C总线
   - SPI总线
   - Ethernet以太网
   - SpaceWire（航天专用）
   - MIL-STD-1553B（航天专用）

4. **配置管理功能**
   - 配置注册和查询
   - 按平台/产品/版本查找配置
   - 按外设名称查找配置
   - 配置验证和打印

5. **配置选择机制**
   - 环境变量选择（优先级最高）
   - 编译时定义选择
   - 默认配置选择

6. **示例配置**
   - TI AM625平台 - H200载荷板基础配置
   - TI AM625平台 - H200载荷板V1.0配置（增加冗余MCU和IMU）
   - TI AM625平台 - H200载荷板V2.0配置（升级1553B和NVMe）

### 设计特点

1. **以外设为单位**
   - 配置以MCU、BMC、传感器等外设为核心
   - 每个外设配置内嵌其通信接口配置
   - 使用联合体节省内存

2. **配置与代码分离**
   - 硬件配置以结构体形式定义
   - 与业务代码完全解耦
   - 便于移植和定制

3. **类型安全**
   - 编译时类型检查
   - 运行时配置验证

4. **易于扩展**
   - 新增平台只需添加配置文件
   - 支持配置继承（base + 版本差异）

### 文件结构

```
xconfig/
├── include/
│   ├── xconfig_types.h       # 配置类型定义
│   └── xconfig_api.h         # API接口
├── src/
│   ├── xconfig_api.c         # API实现
│   └── xconfig_register.c    # 配置注册
├── platform/
│   └── ti/am625/
│       ├── h200_payload_base.c # 基础配置
│       ├── h200_payload_v1.c   # V1.0配置
│       └── h200_payload_v2.c   # V2.0配置
├── examples/
│   └── xconfig_example.c     # 使用示例
├── README.md                   # 详细说明
├── INTEGRATION.md              # 集成指南
├── CHANGELOG.md                # 更新日志
└── CMakeLists.txt              # 构建配置
```

### 集成到PMC-BSP

- 已集成到主项目CMakeLists.txt
- 已更新主项目README.md
- 编译成功，生成libxconfig.a静态库

### 下一步计划

- [ ] 实现配置继承和合并机制
- [ ] 添加更多平台配置示例
- [ ] 实现配置序列化/反序列化
- [ ] 开发配置可视化工具
- [ ] 添加配置热更新支持
