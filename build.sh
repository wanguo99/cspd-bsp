#!/bin/bash

# PMC-BSP 构建脚本

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

show_help() {
    cat << EOF
用法: $0 [选项]

选项:
    -h, --help          显示此帮助信息
    -c, --clean         清理构建目录
    -d, --debug         Debug模式编译
    -r, --release       Release模式编译 (默认)
    -a, --arch ARCH     指定目标架构 (默认: native)
                        - native:       本地架构 (自动检测)
                        - x86_64:       x86_64 Linux
                        - arm32:        ARM32 Linux (ARMv7-A)
                        - arm64:        ARM64 Linux (ARMv8-A)
                        - riscv64:      RISC-V 64 Linux
    --target TARGET     指定构建目标 (默认: all)
                        - all: 构建所有库和应用
                        - can_gateway: 仅构建CAN网关应用
                        - protocol_converter: 仅构建协议转换应用

示例:
    $0                              # 本地架构 Release 模式构建
    $0 -d                           # 本地架构 Debug 模式构建
    $0 -a arm64                     # ARM64 交叉编译
    $0 -a riscv64 -d                # RISC-V 64 Debug 交叉编译
    $0 --target can_gateway         # 仅构建CAN网关应用
    $0 -c                           # 清理构建目录

支持的交叉编译工具链:
    - ARM32:    arm-linux-gnueabihf-gcc
    - ARM64:    aarch64-linux-gnu-gcc
    - RISC-V64: riscv64-linux-gnu-gcc

EOF
}

# 默认参数
BUILD_TYPE="Release"
CLEAN=0
OUTPUT_DIR="output"
BUILD_TARGET="all"
ARCH="native"

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
        -a|--arch)
            ARCH="$2"
            shift 2
            ;;
        --target)
            BUILD_TARGET="$2"
            shift 2
            ;;
        *)
            print_error "未知选项: $1"
            show_help
            exit 1
            ;;
    esac
done

# 验证架构参数
case $ARCH in
    native|x86_64|arm32|arm64|riscv64)
        ;;
    *)
        print_error "不支持的架构: $ARCH"
        print_error "支持的架构: native, x86_64, arm32, arm64, riscv64"
        exit 1
        ;;
esac

# 映射架构到 CMake PLATFORM 参数
case $ARCH in
    native)
        CMAKE_PLATFORM="native"
        ;;
    x86_64)
        CMAKE_PLATFORM="native"
        ;;
    arm32)
        CMAKE_PLATFORM="arm32-linux"
        ;;
    arm64)
        CMAKE_PLATFORM="arm64-linux"
        ;;
    riscv64)
        CMAKE_PLATFORM="riscv64-linux"
        ;;
esac

# 清理输出目录
if [ $CLEAN -eq 1 ]; then
    print_info "清理输出目录..."
    rm -rf "$OUTPUT_DIR"
    print_info "清理完成！"
    exit 0
fi

# 创建输出目录
mkdir -p "$OUTPUT_DIR/build"
cd "$OUTPUT_DIR/build"

# 构建日志文件
BUILD_LOG="../build.log"

# 检查交叉编译工具链
if [ "$ARCH" != "native" ] && [ "$ARCH" != "x86_64" ]; then
    case $ARCH in
        arm32)
            TOOLCHAIN_CC="arm-linux-gnueabihf-gcc"
            ;;
        arm64)
            TOOLCHAIN_CC="aarch64-linux-gnu-gcc"
            ;;
        riscv64)
            TOOLCHAIN_CC="riscv64-linux-gnu-gcc"
            ;;
    esac

    if ! command -v "$TOOLCHAIN_CC" &> /dev/null; then
        print_error "交叉编译工具链未找到: $TOOLCHAIN_CC"
        print_error "请安装对应的工具链:"
        case $ARCH in
            arm32)
                print_error "  Ubuntu/Debian: sudo apt-get install gcc-arm-linux-gnueabihf"
                ;;
            arm64)
                print_error "  Ubuntu/Debian: sudo apt-get install gcc-aarch64-linux-gnu"
                ;;
            riscv64)
                print_error "  Ubuntu/Debian: sudo apt-get install gcc-riscv64-linux-gnu"
                ;;
        esac
        cd ../..
        exit 1
    fi
    print_info "使用交叉编译工具链: $TOOLCHAIN_CC"
fi

# 运行CMake配置
print_info "运行CMake配置 (架构: $ARCH, 构建类型: $BUILD_TYPE)..."
{
    cmake ../.. \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DPLATFORM="$CMAKE_PLATFORM" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -DBUILD_TESTING=ON
} 2>&1 | tee "$BUILD_LOG"

if [ ${PIPESTATUS[0]} -ne 0 ]; then
    print_error "CMake配置失败，详见 $OUTPUT_DIR/build.log"
    cd ../..
    exit 1
fi

# 编译
print_info "开始编译..."
CPU_CORES=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

if [ "$BUILD_TARGET" = "all" ]; then
    make -j"$CPU_CORES" 2>&1 | tee -a "$BUILD_LOG"
    BUILD_STATUS=${PIPESTATUS[0]}
else
    print_info "构建目标: $BUILD_TARGET"
    make -j"$CPU_CORES" "$BUILD_TARGET" 2>&1 | tee -a "$BUILD_LOG"
    BUILD_STATUS=${PIPESTATUS[0]}
fi

cd ../..

if [ $BUILD_STATUS -ne 0 ]; then
    print_error "编译失败，详见 $OUTPUT_DIR/build.log"
    exit 1
fi

print_info "编译成功！"
print_info ""
print_info "构建信息:"
print_info "  目标架构: $ARCH"
print_info "  构建类型: $BUILD_TYPE"
print_info ""
print_info "输出目录:"
print_info "  构建日志: output/build.log"
print_info "  编译文件: output/build/"
print_info "  目标产物: output/target/"
print_info ""
if [ "$BUILD_TARGET" = "all" ]; then
    print_info "可执行文件:"
    print_info "  ./output/target/bin/sample_app"
    print_info "  ./output/target/bin/unit-test"
    print_info ""
    if [ "$ARCH" != "native" ] && [ "$ARCH" != "x86_64" ]; then
        print_info "注意: 这是交叉编译的二进制文件，需要在目标平台上运行"
    fi
fi
print_info ""
