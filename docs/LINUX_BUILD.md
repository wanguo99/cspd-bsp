# Linux 环境编译指南

## 前提条件

### 必需的软件包

在 Linux 环境中编译前，需要安装以下软件包：

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git

# CentOS/RHEL
sudo yum install -y \
    gcc \
    make \
    cmake \
    git

# Arch Linux
sudo pacman -S base-devel cmake git
```

### 验证环境

```bash
# 检查 GCC
gcc --version

# 检查 CMake
cmake --version

# 检查 Make
make --version
```

## 编译步骤

### 1. 传输代码到 Linux 环境

如果你的代码在 Windows 上，需要先传输到 Linux：

```bash
# 方法1: 使用 scp
scp -r /z/cFS/cspd-bsp user@linux-host:/path/to/destination/

# 方法2: 使用 git
cd /z/cFS/cspd-bsp
git init
git add .
git commit -m "Initial commit"
# 然后在 Linux 上 git clone

# 方法3: 如果使用 WSL
# 代码已经在 /mnt/z/cFS/cspd-bsp 可直接访问
```

### 2. 本地编译（native）

```bash
cd cspd-bsp

# 方法1: 使用构建脚本（推荐）
./build.sh

# 方法2: 手动使用 CMake
mkdir build && cd build
cmake ..
make -j$(nproc)
cd ..

# 运行程序
./build/bin/cspd-bsp
```

### 3. 交叉编译（generic-linux）

如果需要为其他 Linux 平台（如 ARM）交叉编译：

```bash
# 安装交叉编译工具链
sudo apt-get install gcc-arm-linux-gnueabihf

# 使用构建脚本
./build.sh --platform generic-linux

# 或手动指定编译器
mkdir build && cd build
cmake .. -DPLATFORM=generic-linux -DCMAKE_C_COMPILER=arm-linux-gnueabihf-gcc
make -j$(nproc)
```

### 4. Debug 模式编译

```bash
# 使用构建脚本
./build.sh -d

# 或手动
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
```

## 编译选项

### build.sh 脚本选项

```bash
./build.sh [选项]

选项:
    -h, --help          显示帮助信息
    -c, --clean         清理构建目录
    -d, --debug         Debug模式编译
    -r, --release       Release模式编译 (默认)
    -t, --test          编译后运行测试
    -i, --install       安装到系统
    --prefix PATH       设置安装路径 (默认: /usr/local)
    --platform PLATFORM 目标平台 (默认: native)
                        - native: 本地编译
                        - generic-linux: 标准Linux交叉编译
```

### 常用编译命令

```bash
# 清理后重新编译
./build.sh -c -r

# Debug 模式编译
./build.sh -d

# 交叉编译
./build.sh --platform generic-linux

# 编译并安装
./build.sh -r -i

# 指定安装路径
./build.sh -i --prefix /opt/cspd-bsp
```

## 编译输出

编译成功后，输出文件位于：

```
build/
├── bin/
│   └── cspd-bsp          # 可执行文件
└── lib/
    ├── libosal.a         # OSAL静态库
    ├── libhal.a          # HAL静态库
    └── libapps.a         # 应用层静态库
```

## 常见问题

### 1. CMake 版本过低

```bash
# 错误: CMake 3.10 or higher is required
# 解决: 升级 CMake
sudo apt-get install cmake

# 或从源码安装最新版本
wget https://github.com/Kitware/CMake/releases/download/v3.27.0/cmake-3.27.0.tar.gz
tar -xzf cmake-3.27.0.tar.gz
cd cmake-3.27.0
./bootstrap && make && sudo make install
```

### 2. 缺少头文件

```bash
# 错误: fatal error: sys/socket.h: No such file or directory
# 解决: 安装开发包
sudo apt-get install build-essential linux-headers-$(uname -r)
```

### 3. 权限问题

```bash
# 错误: Permission denied
# 解决: 添加执行权限
chmod +x build.sh
```

### 4. 交叉编译工具链未找到

```bash
# 错误: arm-linux-gnueabihf-gcc: command not found
# 解决: 安装交叉编译工具链
sudo apt-get install gcc-arm-linux-gnueabihf

# 或指定自定义编译器
export CMAKE_C_COMPILER=/path/to/your/cross-compiler
./build.sh --platform generic-linux
```

## 验证编译结果

```bash
# 检查可执行文件
file build/bin/cspd-bsp

# 本地编译输出示例:
# build/bin/cspd-bsp: ELF 64-bit LSB executable, x86-64, ...

# 交叉编译输出示例:
# build/bin/cspd-bsp: ELF 32-bit LSB executable, ARM, ...

# 查看依赖库
ldd build/bin/cspd-bsp

# 运行程序
./build/bin/cspd-bsp
```

## 性能优化编译

```bash
# Release 模式 + 优化
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS="-O3 -march=native"
make -j$(nproc)
```

## 清理

```bash
# 清理构建文件
./build.sh -c

# 或手动清理
rm -rf build/

# 完全清理（包括 CMake 缓存）
git clean -fdx
```

## 下一步

编译成功后，参考以下文档：

- [快速开始](QUICKSTART.md) - 配置和运行
- [部署指南](DEPLOYMENT.md) - 生产环境部署
- [架构文档](ARCHITECTURE.md) - 了解系统架构

## 技术支持

如遇到编译问题，请检查：

1. 系统是否满足前提条件
2. 所有依赖包是否已安装
3. build.sh 是否有执行权限
4. CMake 版本是否 >= 3.10

更多信息请参考项目 README.md
