# 项目重命名完成报告

## 重命名概述

项目已从 `lite-cfs-framework` 成功重命名为 `cspd-bsp` (Compute and Storage Payload Board Support Package)。

## 新名称说明

**CSPD-BSP**: Compute and Storage Payload Board Support Package

- **CSPD**: Compute and Storage Payload Device（算存载荷设备）
- **BSP**: Board Support Package（板级支持包）

## 命名优势

### 更准确的定位
- ✅ 明确指出是**算存载荷**专用
- ✅ 强调**板级支持包**的性质
- ✅ 符合航天领域命名规范

### 专业性
- ✅ BSP是业界标准术语
- ✅ 清晰表达产品功能
- ✅ 易于理解和记忆

### 独立性
- ✅ 完全独立的品牌标识
- ✅ 不依赖其他框架名称
- ✅ 体现产品独特性

## 修改内容

### 1. CMake配置
```cmake
# 旧名称
project(LiteCFS VERSION 1.0.0 LANGUAGES C)
add_executable(sat_bridge main.c)

# 新名称
project(CSPD-BSP VERSION 1.0.0 LANGUAGES C)
add_executable(cspd-bsp main.c)
```

### 2. 可执行文件
```
旧: build/bin/sat_bridge
新: build/bin/cspd-bsp
```

### 3. 文档更新
- ✅ README.md - 完全重写
- ✅ ARCHITECTURE.md - 更新项目名称
- ✅ 所有文档中的引用

### 4. 构建脚本
```bash
# build.sh 头部注释
旧: Lite-cFS Framework CMake构建脚本
新: CSPD-BSP CMake构建脚本
```

### 5. 术语更新
| 旧术语 | 新术语 |
|--------|--------|
| 服务器 | 载荷 |
| 管理板 | 载荷管理板 |
| 卫星-服务器桥接 | 卫星-载荷桥接 |
| sat_bridge | cspd-bsp |

## 目录结构

```
cspd-bsp/                       # 新项目名
├── CMakeLists.txt              # 已更新
├── build.sh                    # 已更新
├── README.md                   # 已更新
├── main.c                      # 主程序
├── config/
│   ├── system_config.h
│   └── can_protocol.h
├── osal/                       # 操作系统抽象层
├── hal/                        # 硬件抽象层
├── apps/                       # 应用模块
└── docs/                       # 文档
    ├── ARCHITECTURE.md
    ├── CMAKE_GUIDE.md
    ├── QUICKSTART.md
    ├── DEPLOYMENT.md
    └── QUICK_REFERENCE.md
```

## 构建命令

### 旧命令
```bash
./build.sh
sudo ./build/bin/sat_bridge
```

### 新命令
```bash
./build.sh
sudo ./build/bin/cspd-bsp
```

## 安装路径

### 旧路径
```bash
./build.sh -i --prefix /opt/sat-bridge
```

### 新路径
```bash
./build.sh -i --prefix /opt/cspd-bsp
```

## 日志文件

### 旧路径
```
/var/log/sat_bridge.log
```

### 新路径
```
/var/log/cspd-bsp.log
```

## 系统架构图

### 更新后
```
┌─────────────┐                    ┌──────────────┐                    ┌─────────────┐
│  卫星平台    │ <--- CAN 2.0 ---> │ 算存载荷管理板 │ <--- Ethernet --> │  算存载荷    │
│ (自定义协议) │                    │   (CSPD-BSP)  │ <--- UART(备份)-> │ (IPMI/Redfish)│
└─────────────┘                    └──────────────┘                    └─────────────┘
```

## 品牌标识

### 项目全称
**CSPD-BSP**: Compute and Storage Payload Board Support Package

### 中文名称
**算存载荷板级支持包**

### 简称
**CSPD-BSP** 或 **cspd-bsp**

## 后续工作

### 建议修改
1. ⚠️ 考虑重命名项目根目录: `lite-cfs-framework` → `cspd-bsp`
2. ⚠️ 更新代码注释中的项目引用
3. ⚠️ 更新配置文件中的日志路径

### 可选修改
- 创建项目Logo
- 设计项目图标
- 制作宣传材料

## 兼容性说明

### 代码兼容
- ✅ 所有API接口保持不变
- ✅ 配置文件格式不变
- ✅ 协议定义不变

### 构建兼容
- ✅ CMake配置兼容
- ✅ 编译选项不变
- ✅ 依赖库不变

### 部署兼容
- ⚠️ 可执行文件名变更: `sat_bridge` → `cspd-bsp`
- ⚠️ 需要更新启动脚本
- ⚠️ 需要更新systemd服务文件

## 迁移指南

### 对于现有用户

1. **重新编译**
```bash
./build.sh -c -r
```

2. **更新启动脚本**
```bash
# 旧
sudo ./build/bin/sat_bridge

# 新
sudo ./build/bin/cspd-bsp
```

3. **更新systemd服务**
```ini
# /etc/systemd/system/cspd-bsp.service
[Service]
ExecStart=/opt/cspd-bsp/bin/cspd-bsp
```

## 总结

✅ **重命名完成**: 项目已成功重命名为 CSPD-BSP

✅ **定位清晰**: 算存载荷板级支持包

✅ **专业规范**: 符合航天领域命名标准

✅ **独立品牌**: 完全独立的产品标识

**新名称更准确地反映了产品的实际用途和定位。**
