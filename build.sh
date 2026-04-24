#!/bin/bash

# CSPD-BSP 构建脚本

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
    --target TARGET     指定构建目标 (默认: all)
                        - all: 构建所有库和应用
                        - can_gateway: 仅构建CAN网关应用
                        - protocol_converter: 仅构建协议转换应用

示例:
    $0                              # Release模式构建所有
    $0 -d                           # Debug模式构建所有
    $0 --target can_gateway         # 仅构建CAN网关应用
    $0 -c                           # 清理构建目录

EOF
}

# 默认参数
BUILD_TYPE="Release"
CLEAN=0
OUTPUT_DIR="output"
BUILD_TARGET="all"

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

# 运行CMake配置
print_info "运行CMake配置 (构建类型: $BUILD_TYPE)..."
{
    cmake ../.. \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
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
print_info "输出目录:"
print_info "  构建日志: output/build.log"
print_info "  编译文件: output/build/"
print_info "  目标产物: output/target/"
print_info ""
if [ "$BUILD_TARGET" = "all" ]; then
    print_info "可执行文件:"
    print_info "  ./output/target/bin/can_gateway"
    print_info "  ./output/target/bin/protocol_converter"
    print_info "  ./output/target/bin/unit-test"
fi
print_info ""
