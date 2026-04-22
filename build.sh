#!/bin/bash

# CSPD-BSP CMake构建脚本

set -e  # 遇到错误立即退出

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 打印带颜色的消息
print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 显示帮助信息
show_help() {
    cat << EOF
用法: $0 [选项]

选项:
    -h, --help          显示此帮助信息
    -c, --clean         清理构建目录
    -d, --debug         Debug模式编译
    -r, --release       Release模式编译 (默认)
    -t, --test          编译后运行测试
    -i, --install       安装到系统
    --prefix PATH       设置安装路径 (默认: /usr/local)
    --platform PLATFORM 目标平台 (默认: native)
                        - native: 本地编译
                        - generic-linux: 标准Linux交叉编译

示例:
    $0                              # 本地Release模式编译
    $0 -d                           # 本地Debug模式编译
    $0 --platform generic-linux     # 交叉编译到Linux
    $0 -c -r --platform native      # 清理后本地重新编译
    $0 -r -i                        # 编译并安装

EOF
}

# 默认参数
BUILD_TYPE="Release"
PLATFORM="native"
CLEAN=0
RUN_TEST=0
INSTALL=0
INSTALL_PREFIX="/usr/local"
BUILD_DIR="build"

# 解析命令行参数
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -c|--clean)
            CLEAN=1
            shift
            ;;
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -r|--release)
            BUILD_TYPE="Release"
            shift
            ;;
        -t|--test)
            RUN_TEST=1
            shift
            ;;
        -i|--install)
            INSTALL=1
            shift
            ;;
        --prefix)
            INSTALL_PREFIX="$2"
            shift 2
            ;;
        --platform)
            PLATFORM="$2"
            shift 2
            ;;
        *)
            print_error "未知选项: $1"
            show_help
            exit 1
            ;;
    esac
done

# 验证平台参数
case $PLATFORM in
    native|generic-linux)
        ;;
    *)
        print_error "不支持的平台: $PLATFORM"
        print_error "支持的平台: native, generic-linux"
        exit 1
        ;;
esac

# 清理构建目录
if [ $CLEAN -eq 1 ]; then
    print_info "清理构建目录..."
    rm -rf "$BUILD_DIR"
    print_info "清理完成！"
    exit 0
fi

# 创建构建目录
if [ ! -d "$BUILD_DIR" ]; then
    print_info "创建构建目录: $BUILD_DIR"
    mkdir -p "$BUILD_DIR"
fi

# 进入构建目录
cd "$BUILD_DIR"

# 运行CMake配置
print_info "运行CMake配置 (平台: $PLATFORM, 构建类型: $BUILD_TYPE)..."
cmake .. \
    -DPLATFORM="$PLATFORM" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# 编译
print_info "开始编译..."
CPU_CORES=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
make -j"$CPU_CORES"

if [ $? -eq 0 ]; then
    print_info "编译成功！"
    print_info "可执行文件: $BUILD_DIR/bin/cspd-bsp"
else
    print_error "编译失败！"
    exit 1
fi

# 运行测试
if [ $RUN_TEST -eq 1 ]; then
    print_info "运行测试..."
    ctest --output-on-failure
fi

# 安装
if [ $INSTALL -eq 1 ]; then
    print_info "安装到: $INSTALL_PREFIX"
    sudo make install
    print_info "安装完成！"
fi

# 返回项目根目录
cd ..

print_info "构建完成！"
print_info ""
print_info "运行程序:"
print_info "  ./$BUILD_DIR/bin/cspd-bsp"
print_info ""
