# cFS (Core Flight System) 编译框架总结

## 1. 整体架构

cFS是NASA开发的通用飞行软件架构框架，采用模块化设计，使用CMake作为构建系统。

### 1.1 核心组件层次结构

```
cFS/
├── cfe/          # Core Flight Executive (核心飞行执行环境)
├── osal/         # Operating System Abstraction Layer (操作系统抽象层)
├── psp/          # Platform Support Package (平台支持包)
├── apps/         # 应用程序模块
├── libs/         # 库模块
├── tools/        # 构建工具
└── sample_defs/  # 示例配置定义
```

## 2. 编译系统设计

### 2.1 双层构建架构

cFS采用**两级构建系统**：

1. **Mission Build (任务级构建)** - 顶层构建，处理多目标架构
   - 入口：`cfe/CMakeLists.txt` (当 `TARGETSYSTEM` 未设置时)
   - 使用：`cmake/mission_build.cmake`
   - 功能：读取配置、组织多架构子构建

2. **Architecture Build (架构级构建)** - 针对特定CPU架构的交叉编译
   - 入口：`cfe/CMakeLists.txt` (当 `TARGETSYSTEM` 已设置时)
   - 使用：`cmake/arch_build.cmake`
   - 功能：生成特定架构的二进制文件

### 2.2 构建流程

```
顶层Makefile (GNU make包装器)
    ↓
make prep → 调用CMake配置
    ↓
CMake读取 sample_defs/targets.cmake
    ↓
为每个CPU架构创建子构建
    ↓
make all → 编译所有目标
    ↓
make install → 安装到 build/exe/cpuX/
```

## 3. 配置系统

### 3.1 核心配置文件

位于 `sample_defs/` 目录：

| 文件 | 用途 |
|------|------|
| `targets.cmake` | 定义CPU目标、应用列表、工具链 |
| `global_build_options.cmake` | 全局编译选项 |
| `mission_build_custom.cmake` | 任务级自定义构建脚本 |
| `arch_build_custom.cmake` | 架构级自定义构建脚本 |
| `default_osconfig.cmake` | OSAL默认配置 |
| `default_pspconfig.cmake` | PSP默认配置 |
| `example_mission_cfg.h` | 任务配置头文件模板 |
| `example_platform_cfg.h` | 平台配置头文件模板 |
| `cpu1_cfe_es_startup.scr` | 启动脚本 |
| `toolchain-*.cmake` | 交叉编译工具链定义 |

### 3.2 targets.cmake 配置详解

```cmake
# 任务名称和航天器ID
SET(MISSION_NAME "SampleMission")
SET(SPACECRAFT_ID 0x42)

# 全局应用列表（所有CPU都会构建）
list(APPEND MISSION_GLOBAL_APPLIST sample_app sample_lib)

# CPU定义
SET(MISSION_CPUNAMES cpu1)

# CPU1配置
SET(cpu1_PROCESSORID 1)                    # 处理器ID
SET(cpu1_APPLIST ci_lab to_lab sch_lab)   # 动态加载应用
SET(cpu1_FILELIST cfe_es_startup.scr)     # 额外文件
SET(cpu1_SYSTEM i686-linux-gnu)           # 工具链选择
```

**关键变量说明：**
- `<cpuname>_APPLIST`: 动态加载的应用（编译为.so文件）
- `<cpuname>_STATIC_APPLIST`: 静态链接的应用（直接链接到cFE可执行文件）
- `<cpuname>_PSP_MODULELIST`: PSP模块列表
- `<cpuname>_SYSTEM`: 工具链名称，映射到 `toolchain-<name>.cmake`

## 4. 模块编译机制

### 4.1 核心层次依赖

```
应用层 (Apps)
    ↓ 依赖
cFE Core API (core_api)
    ↓ 依赖
PSP (Platform Support Package)
    ↓ 依赖
OSAL (Operating System Abstraction Layer)
    ↓ 依赖
操作系统 (Linux/VxWorks/RTEMS/QNX)
```

### 4.2 OSAL (操作系统抽象层)

**构建目标：**
- `osal_public_api`: 接口库（仅头文件）
- `osal`: 主库（包含OS抽象实现）
- `osal_bsp`: 板级支持库（包含main入口）
- `ut_assert`: 单元测试支持库

**配置：**
- `OSAL_SYSTEM_BSPTYPE`: BSP类型（如 pc-linux, pc-rtems）
- `OSAL_SYSTEM_OSTYPE`: OS类型（posix, vxworks, rtems）

### 4.3 PSP (平台支持包)

**模块化结构：**
```
psp-<target>
├── psp-<target>-impl      # 平台特定实现
├── psp-<target>-shared    # 共享组件
└── PSP模块                # 可选驱动模块
```

**构建过程：**
1. 读取 `fsw/<target>/psp_module_list.cmake` 获取模块列表
2. 编译平台特定实现和共享代码
3. 编译PSP模块（如设备驱动）
4. 生成静态库 `psp-<target>`

### 4.4 cFE (核心飞行执行环境)

**核心服务模块：**
- ES (Executive Services): 应用管理、启动
- EVS (Event Services): 事件消息
- SB (Software Bus): 软件总线通信
- TBL (Table Services): 表管理
- TIME (Time Services): 时间管理
- FS (File Services): 文件服务

**构建产物：**
- `core_api`: 接口库（供应用使用）
- `core-cpu<x>`: cFE核心可执行文件

### 4.5 应用程序编译

**标准应用CMakeLists.txt结构：**

```cmake
project(CFE_SAMPLE_APP C)

# 定义源文件
set(APP_SRC_FILES
  fsw/src/sample_app.c
  fsw/src/sample_app_cmds.c
  fsw/src/sample_app_utils.c
)

# 创建应用模块
add_cfe_app(sample_app ${APP_SRC_FILES})

# 添加头文件目录
target_include_directories(sample_app PUBLIC fsw/inc)

# 添加依赖库
add_cfe_app_dependency(sample_app sample_lib)

# 添加表文件
add_cfe_tables(sample_app fsw/tables/sample_app_tbl.c)

# 单元测试（可选）
if (ENABLE_UNIT_TESTS)
  add_subdirectory(unit-test)
endif()
```

**关键CMake函数：**

1. **`add_cfe_app(APP_NAME SRC_FILES)`**
   - 创建应用模块（MODULE或STATIC类型）
   - 自动链接 `core_api`
   - 动态应用会生成 `.so` 文件并安装到目标目录
   - 创建 `<APP_NAME>.table` 接口目标供表文件使用

2. **`add_cfe_app_dependency(APP_NAME DEP_MODULE)`**
   - 添加应用间依赖关系
   - 自动处理头文件路径和链接

3. **`add_cfe_tables(APP_NAME TABLE_SRC)`**
   - 编译表文件（.tbl）
   - 使用 `elf2cfetbl` 工具转换

### 4.6 库模块编译

库模块与应用类似，但通常提供可重用功能：

```cmake
project(CFE_SAMPLE_LIB C)

set(LIB_SRC_FILES
  fsw/src/sample_lib.c
)

add_cfe_app(sample_lib ${LIB_SRC_FILES})
target_include_directories(sample_lib PUBLIC fsw/inc)
```

## 5. 构建命令详解

### 5.1 顶层Makefile变量

```makefile
O ?= build                    # 构建目录
ARCH ?= native/default_cpu1   # 架构
BUILDTYPE ?= debug            # 构建类型 (debug/release)
INSTALLPREFIX ?= /exe         # 安装前缀
DESTDIR ?= $(O)               # 安装目标目录
```

### 5.2 常用构建命令

```bash
# 1. 清理构建
make distclean

# 2. 配置构建（准备阶段）
make SIMULATION=native prep
# 可选参数：
#   SIMULATION=native          # 本地仿真
#   BUILDTYPE=release          # 发布版本
#   ENABLE_UNIT_TESTS=true     # 启用单元测试
#   OMIT_DEPRECATED=true       # 禁用废弃特性

# 3. 编译
make

# 4. 安装
make install

# 5. 运行单元测试
make test

# 6. 生成代码覆盖率报告
make lcov

# 7. 生成文档
make doc          # 所有文档
make usersguide   # cFE用户指南
make osalguide    # OSAL API指南
```

### 5.3 CMake配置选项

```bash
# 在prep阶段传递给CMake的选项：
make prep \
  SIMULATION=native \
  BUILDTYPE=release \
  ENABLE_UNIT_TESTS=true \
  CFE_EDS_ENABLED=true \
  OMIT_DEPRECATED=true
```

## 6. 构建产物

### 6.1 目录结构

```
build/
├── native/default_cpu1/        # 架构特定构建目录
│   ├── core-cpu1               # cFE核心可执行文件
│   ├── apps/                   # 应用模块
│   │   ├── sample_app.so
│   │   ├── ci_lab.so
│   │   └── ...
│   └── tables/                 # 表文件
│       └── sample_app_tbl.tbl
└── exe/cpu1/                   # 安装目录
    ├── core-cpu1               # 可执行文件
    ├── cf/                     # 应用和表文件
    │   ├── sample_app.so
    │   ├── sample_app_tbl.tbl
    │   └── cfe_es_startup.scr  # 启动脚本
    └── ...
```

### 6.2 启动脚本

`cpu1_cfe_es_startup.scr` 定义启动时加载的应用：

```
CFE_LIB, sample_lib,  Sample_Lib_Init,  SAMPLE_LIB,  0,   0,    0x0, 0;
CFE_APP, sample_app,  SAMPLE_APP_Main,  SAMPLE_APP,  50,  8192, 0x0, 0;
CFE_APP, ci_lab,      CI_Lab_AppMain,   CI_LAB_APP,  60,  8192, 0x0, 0;
CFE_APP, to_lab,      TO_Lab_AppMain,   TO_LAB_APP,  70,  8192, 0x0, 0;
CFE_APP, sch_lab,     SCH_Lab_AppMain,  SCH_LAB_APP, 80,  8192, 0x0, 0;
```

格式：`类型, 文件名, 入口函数, 名称, 优先级, 栈大小, 异常动作, 标志`

## 7. 依赖关系图

### 7.1 编译时依赖

```
┌─────────────────────────────────────────┐
│         应用层 (Apps & Libs)             │
│  sample_app, ci_lab, to_lab, sch_lab    │
└──────────────┬──────────────────────────┘
               │ 依赖 core_api
┌──────────────▼──────────────────────────┐
│         cFE Core Services                │
│  ES, EVS, SB, TBL, TIME, FS, MSG         │
└──────────────┬──────────────────────────┘
               │ 依赖 psp-<target>
┌──────────────▼──────────────────────────┐
│    PSP (Platform Support Package)        │
│  psp-impl + psp-shared + psp-modules     │
└──────────────┬──────────────────────────┘
               │ 依赖 osal
┌──────────────▼──────────────────────────┐
│    OSAL (OS Abstraction Layer)           │
│         osal + osal_bsp                  │
└──────────────┬──────────────────────────┘
               │
┌──────────────▼──────────────────────────┐
│      操作系统 (Linux/RTEMS/VxWorks)      │
└─────────────────────────────────────────┘
```

### 7.2 运行时依赖

```
core-cpu1 (可执行文件)
├── 静态链接：
│   ├── cFE核心服务
│   ├── PSP
│   ├── OSAL
│   └── 静态应用（如果有）
└── 动态加载：
    ├── sample_lib.so
    ├── sample_app.so
    ├── ci_lab.so
    ├── to_lab.so
    └── sch_lab.so
```

## 8. 交叉编译支持

### 8.1 工具链文件

cFS支持多种目标平台，通过工具链文件配置：

- `toolchain-i686-linux-gnu.cmake` - x86 Linux
- `toolchain-i686-rtems5.cmake` - RTEMS 5
- `toolchain-aarch64-vx7rtp.cmake` - VxWorks 7 ARM64
- `toolchain-x86_64-qnx-gnu.cmake` - QNX x86_64
- 等等...

### 8.2 工具链文件结构

```cmake
# 设置交叉编译
set(CMAKE_SYSTEM_NAME RTEMS)
set(CMAKE_SYSTEM_PROCESSOR i686)

# 指定编译器
set(CMAKE_C_COMPILER i686-rtems5-gcc)
set(CMAKE_CXX_COMPILER i686-rtems5-g++)

# 设置编译标志
set(CMAKE_C_FLAGS_INIT "...")
set(CMAKE_EXE_LINKER_FLAGS_INIT "...")
```

## 9. 扩展机制

### 9.1 添加新应用

1. 在 `apps/` 或项目目录创建应用目录
2. 编写 `CMakeLists.txt`
3. 在 `targets.cmake` 中添加到 `<cpu>_APPLIST`
4. 在启动脚本中添加加载条目

### 9.2 添加新库

1. 在 `libs/` 或项目目录创建库目录
2. 编写 `CMakeLists.txt`
3. 在 `targets.cmake` 中添加到 `MISSION_GLOBAL_APPLIST`
4. 其他应用通过 `add_cfe_app_dependency()` 使用

### 9.3 自定义构建步骤

通过以下文件添加自定义逻辑：
- `mission_build_custom.cmake` - 任务级自定义
- `arch_build_custom.cmake` - 架构级自定义
- `arch_build_custom_<target>.cmake` - 特定目标自定义

## 10. 构建优化

### 10.1 并行构建

```bash
make -j8  # 使用8个并行任务
```

### 10.2 增量构建

CMake自动处理增量构建，只重新编译修改的文件。

### 10.3 构建类型

- `debug`: 包含调试符号，无优化
- `release`: 优化编译，无调试符号
- `relwithdebinfo`: 优化+调试符号
- `minsizerel`: 最小体积优化

## 11. 单元测试

### 11.1 启用测试

```bash
make ENABLE_UNIT_TESTS=true prep
make
make test
```

### 11.2 测试框架

- **ut_assert**: OSAL提供的测试框架
- **ut_osapi_stubs**: OSAL API桩函数
- **覆盖率工具**: lcov/gcov

### 11.3 测试结构

```
app/
├── fsw/src/          # 源代码
├── unit-test/        # 单元测试
│   ├── CMakeLists.txt
│   └── app_tests.c
└── ut-stubs/         # 桩函数（可选）
```

## 12. 常见问题

### 12.1 构建失败

- 检查 `MISSION_DEFS` 路径是否正确
- 确认所有子模块已初始化：`git submodule update --init`
- 清理后重新构建：`make distclean && make prep && make`

### 12.2 找不到头文件

- 检查 `target_include_directories()` 设置
- 确认依赖关系通过 `add_cfe_app_dependency()` 正确声明

### 12.3 链接错误

- 检查 `targets.cmake` 中应用列表顺序（库应在应用之前）
- 确认静态/动态链接配置一致

## 13. 总结

cFS编译框架的核心特点：

1. **模块化设计**: 清晰的层次结构（OSAL → PSP → cFE → Apps）
2. **跨平台支持**: 通过OSAL和工具链文件支持多种OS和架构
3. **灵活配置**: 通过 `targets.cmake` 和配置文件定制构建
4. **CMake驱动**: 现代化的构建系统，支持增量编译和并行构建
5. **动态加载**: 应用可以编译为共享库，运行时动态加载
6. **测试集成**: 内置单元测试和覆盖率分析支持

这种设计使得cFS能够适应从CubeSat到载人航天器等各种任务需求。
