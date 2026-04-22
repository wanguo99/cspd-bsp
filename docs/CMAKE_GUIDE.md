# CMake 构建系统使用指南

## 概述

本项目使用 CMake 作为构建系统，支持跨平台编译和模块化管理。

## 目录结构

```
cspd-bsp/
├── CMakeLists.txt              # 根CMake配置
├── build.sh                    # 自动构建脚本
├── osal/
│   └── CMakeLists.txt         # OSAL模块配置
├── hal/
│   └── CMakeLists.txt         # HAL模块配置
├── apps/
│   └── CMakeLists.txt         # Apps模块配置
└── build/                      # 构建输出目录（自动生成）
    ├── bin/                   # 可执行文件
    └── lib/                   # 静态库
```

## 快速开始

### 方法1: 使用自动构建脚本（推荐）

```bash
# Release模式编译
./build.sh

# Debug模式编译
./build.sh -d

# 清理后重新编译
./build.sh -c -r

# 编译并安装
./build.sh -r -i --prefix /opt/cspd-bsp
```

### 方法2: 手动使用CMake

```bash
# 创建构建目录
mkdir build
cd build

# 配置项目
cmake ..

# 编译
make -j$(nproc)

# 运行程序
./bin/cspd-bsp
```

## 构建选项

### 构建类型

CMake支持以下构建类型：

| 构建类型 | 说明 | 编译选项 |
|---------|------|---------|
| Release | 发布版本（默认） | `-O2` 优化 |
| Debug | 调试版本 | `-g -O0` 无优化，包含调试符号 |
| RelWithDebInfo | 带调试信息的发布版 | `-O2 -g` |
| MinSizeRel | 最小体积发布版 | `-Os` |

**指定构建类型**:
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
```

### 安装路径

默认安装路径为 `/usr/local`，可以自定义：

```bash
cmake -DCMAKE_INSTALL_PREFIX=/opt/cspd-bsp ..
make install
```

### 交叉编译

为嵌入式平台交叉编译：

```bash
# 创建工具链文件 toolchain-arm.cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)

set(CMAKE_FIND_ROOT_PATH /usr/arm-linux-gnueabihf)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# 使用工具链文件编译
cmake -DCMAKE_TOOLCHAIN_FILE=toolchain-arm.cmake ..
make
```

## build.sh 脚本详解

### 命令行选项

```bash
./build.sh [选项]

选项:
  -h, --help          显示帮助信息
  -c, --clean         清理构建目录
  -d, --debug         Debug模式编译
  -r, --release       Release模式编译（默认）
  -t, --test          编译后运行测试
  -i, --install       安装到系统
  --prefix PATH       设置安装路径（默认: /usr/local）
```

### 使用示例

```bash
# 1. 首次编译
./build.sh

# 2. 清理后重新编译
./build.sh -c

# 3. Debug模式编译（用于调试）
./build.sh -d

# 4. 编译并安装到自定义路径
./build.sh -i --prefix /opt/cspd-bsp

# 5. 完整的开发流程
./build.sh -c -d        # 清理并Debug编译
./bin/cspd-bsp        # 运行测试
./build.sh -c -r -i     # 清理、Release编译并安装
```

## 模块化构建

项目分为三个独立模块，每个模块都有自己的 CMakeLists.txt：

### 1. OSAL 模块

**位置**: `osal/CMakeLists.txt`

**源文件**:
- `os_task.c` - 任务管理
- `os_queue.c` - 消息队列
- `os_mutex.c` - 互斥锁
- `os_clock.c` - 时间服务
- `os_heap.c` - 内存管理
- `os_error.c` - 错误处理
- `os_init.c` - 初始化
- `os_log.c` - 日志系统

**输出**: `libosal.a` 静态库

### 2. HAL 模块

**位置**: `hal/CMakeLists.txt`

**源文件**:
- `hal_can_linux.c` - CAN驱动
- `hal_server_linux.c` - 载荷通信驱动

**输出**: `libhal.a` 静态库

**依赖**: OSAL

### 3. Apps 模块

**位置**: `apps/CMakeLists.txt`

**源文件**:
- `can_gateway/can_gateway.c` - CAN网关应用
- `protocol_converter/protocol_converter.c` - 协议转换

**输出**: `libapps.a` 静态库

**依赖**: OSAL, HAL

### 主程序

**源文件**: `main.c`

**输出**: `cspd-bsp` 可执行文件

**依赖**: OSAL, HAL, Apps, pthread, rt

## 依赖库

项目依赖以下系统库：

| 库名 | 用途 | CMake查找方式 |
|-----|------|--------------|
| pthread | POSIX线程 | `find_package(Threads)` |
| rt | 实时扩展（时间函数） | 直接链接 |

## 编译输出

```
build/
├── bin/
│   └── cspd-bsp              # 主程序
├── lib/
│   ├── libosal.a              # OSAL静态库
│   ├── libhal.a               # HAL静态库
│   └── libapps.a              # Apps静态库
├── CMakeCache.txt
├── CMakeFiles/
└── compile_commands.json       # 编译命令数据库（用于IDE）
```

## 常见问题

### 1. CMake版本过低

**错误**:
```
CMake Error: CMake 3.10 or higher is required.
```

**解决**:
```bash
# Ubuntu/Debian
sudo apt install cmake

# 或从源码安装最新版
wget https://github.com/Kitware/CMake/releases/download/v3.25.0/cmake-3.25.0.tar.gz
tar -xzf cmake-3.25.0.tar.gz
cd cmake-3.25.0
./bootstrap && make && sudo make install
```

### 2. 找不到pthread库

**错误**:
```
Could not find Threads
```

**解决**:
```bash
# Ubuntu/Debian
sudo apt install build-essential

# CentOS/RHEL
sudo yum groupinstall "Development Tools"
```

### 3. 找不到rt库

**错误**:
```
/usr/bin/ld: cannot find -lrt
```

**解决**:
```bash
# Ubuntu/Debian
sudo apt install libc6-dev

# CentOS/RHEL
sudo yum install glibc-devel
```

### 4. 权限不足

**错误**:
```
Permission denied: ./build.sh
```

**解决**:
```bash
chmod +x build.sh
```

### 5. 清理构建缓存

如果遇到奇怪的编译错误，尝试清理构建缓存：

```bash
./build.sh -c
# 或手动删除
rm -rf build/
```

## 高级用法

### 1. 生成编译命令数据库

用于IDE（如VSCode、CLion）的代码补全和跳转：

```bash
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
```

生成的 `compile_commands.json` 可以被IDE识别。

### 2. 详细编译输出

查看完整的编译命令：

```bash
make VERBOSE=1
```

### 3. 并行编译

利用多核CPU加速编译：

```bash
# 使用所有CPU核心
make -j$(nproc)

# 使用指定数量的核心
make -j4
```

### 4. 只编译特定模块

```bash
# 只编译OSAL
make osal

# 只编译HAL
make hal

# 只编译Apps
make apps

# 只编译主程序
make cspd-bsp
```

### 5. 安装到自定义路径

```bash
cmake -DCMAKE_INSTALL_PREFIX=$HOME/cspd-bsp ..
make install
```

安装后的目录结构：
```
$HOME/cspd-bsp/
└── bin/
    └── cspd-bsp
```

### 6. 静态分析

使用 clang-tidy 进行静态分析：

```bash
cmake -DCMAKE_C_CLANG_TIDY="clang-tidy;-checks=*" ..
make
```

### 7. 生成文档

如果安装了 Doxygen：

```bash
# 在根CMakeLists.txt中添加
find_package(Doxygen)
if(DOXYGEN_FOUND)
    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()

# 生成文档
make doc
```

## IDE集成

### VSCode

1. 安装 CMake Tools 扩展
2. 打开项目文件夹
3. 按 `Ctrl+Shift+P`，选择 "CMake: Configure"
4. 按 `F7` 编译

### CLion

1. 打开项目文件夹
2. CLion 自动识别 CMakeLists.txt
3. 点击 "Build" 按钮编译

### Eclipse

```bash
cmake -G "Eclipse CDT4 - Unix Makefiles" ..
```

然后在 Eclipse 中导入项目。

## 性能优化

### 编译优化级别

```bash
# 最大优化（可能增加编译时间）
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS="-O3 -march=native" ..

# 最小体积
cmake -DCMAKE_BUILD_TYPE=MinSizeRel ..
```

### 链接时优化（LTO）

```bash
cmake -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON ..
```

## 与Makefile对比

| 特性 | Makefile | CMake |
|-----|---------|-------|
| 跨平台 | ❌ 需要手动适配 | ✅ 自动适配 |
| 依赖管理 | ❌ 手动维护 | ✅ 自动处理 |
| IDE集成 | ❌ 有限支持 | ✅ 广泛支持 |
| 模块化 | ⚠️ 需要手动组织 | ✅ 原生支持 |
| 交叉编译 | ⚠️ 复杂 | ✅ 简单 |
| 学习曲线 | ✅ 简单 | ⚠️ 中等 |

## 参考资料

- [CMake官方文档](https://cmake.org/documentation/)
- [CMake教程](https://cmake.org/cmake/help/latest/guide/tutorial/index.html)
- [Modern CMake](https://cliutils.gitlab.io/modern-cmake/)
- [项目架构文档](ARCHITECTURE.md)
- [快速入门](../QUICKSTART.md)
