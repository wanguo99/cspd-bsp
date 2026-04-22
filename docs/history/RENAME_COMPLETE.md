# 项目重命名和更新完成

## 概述

项目已成功从 `lite-cfs-framework` 重命名为 `CSPD-BSP` (Compute and Storage Payload Board Support Package)，并完成所有相关术语的更新。

## 新项目标识

**项目名称**: CSPD-BSP  
**全称**: Compute and Storage Payload Board Support Package  
**中文**: 算存载荷板级支持包

## 主要变更

### 1. 项目名称
- ✅ `lite-cfs-framework` → `cspd-bsp`
- ✅ `Lite-cFS` → `CSPD-BSP`
- ✅ `LiteCFS` → `CSPD-BSP`

### 2. 可执行文件
- ✅ `sat_bridge` → `cspd-bsp`

### 3. 术语统一
- ✅ `服务器` → `载荷`
- ✅ `服务器管理板` → `载荷管理板`
- ✅ `SERVER_OFFLINE` → `PAYLOAD_OFFLINE`

### 4. 文件路径
- ✅ `/var/log/sat_bridge.log` → `/var/log/cspd-bsp.log`
- ✅ `/var/cache/sat_bridge.cache` → `/var/cache/cspd-bsp.cache`

## 更新的文件

### 配置文件 (4个)
- ✅ CMakeLists.txt
- ✅ build.sh
- ✅ config/system_config.h
- ✅ config/can_protocol.h

### 源代码 (8个)
- ✅ osal/inc/osal.h
- ✅ osal/linux/os_init.c
- ✅ hal/inc/hal_server.h
- ✅ hal/linux/hal_server_linux.c
- ✅ apps/can_gateway/can_gateway.c
- ✅ apps/protocol_converter/protocol_converter.c
- ✅ examples/gateway_demo.c
- ✅ main.c

### 文档 (6个)
- ✅ README.md
- ✅ docs/ARCHITECTURE.md
- ✅ docs/CMAKE_GUIDE.md
- ✅ docs/DEPLOYMENT.md
- ✅ docs/QUICKSTART.md
- ✅ docs/QUICK_REFERENCE.md

## 使用方式

### 编译
```bash
./build.sh
```

### 运行
```bash
sudo ./build/bin/cspd-bsp
```

### 查看日志
```bash
tail -f /var/log/cspd-bsp.log
```

## 系统架构

```
┌─────────────┐                    ┌──────────────┐                    ┌─────────────┐
│  卫星平台    │ <--- CAN 2.0 ---> │ 载荷管理板    │ <--- Ethernet --> │  算存载荷    │
│ (自定义协议) │                    │  (CSPD-BSP)   │ <--- UART(备份)-> │ (IPMI/Redfish)│
└─────────────┘                    └──────────────┘                    └─────────────┘
```

## 验证清单

- [x] CMake项目名称
- [x] 可执行文件名
- [x] 所有文档
- [x] 源代码注释
- [x] 配置文件
- [x] 日志路径
- [x] 缓存路径
- [x] 术语统一
- [x] 系统架构图

**所有更新已完成！项目现在完全使用 CSPD-BSP 作为统一标识。**
