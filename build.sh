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
用法: $0 [选项] [目标]

选项:
    -h, --help          显示此帮助信息
    -c, --clean         清理构建目录
    -d, --debug         Debug模式编译
    -r, --release       Release模式编译 (默认)
    -t, --test          编译后运行测试
    --no-tests          禁用测试编译
    --coverage          启用代码覆盖率 (需要Debug模式)
    -i, --install       安装到系统
    --prefix PATH       设置安装路径 (默认: /usr/local)
    --platform PLATFORM 目标平台 (默认: native)
                        - native: 本地编译
                        - generic-linux: 标准Linux交叉编译
    --target TARGET     指定构建目标 (默认: all)
                        - all: 构建所有库和应用
                        - osal: 仅构建OSAL库
                        - hal: 仅构建HAL库
                        - service: 仅构建Service库
                        - can_gateway: 仅构建CAN网关应用
                        - protocol_converter: 仅构建协议转换应用
                        - tests: 仅构建测试

示例:
    $0                              # 构建所有库和应用（含测试）
    $0 -d                           # Debug模式构建所有
    $0 --target can_gateway         # 仅构建CAN网关应用
    $0 --target protocol_converter  # 仅构建协议转换应用
    $0 -d --coverage -t             # Debug模式+覆盖率+运行测试
    $0 --no-tests                   # 构建所有但不包含测试
    $0 --platform generic-linux     # 交叉编译到Linux
    $0 -c                           # 清理构建目录

EOF
}

# 默认参数
BUILD_TYPE="Release"
PLATFORM="native"
CLEAN=0
RUN_TEST=0
BUILD_TESTING=1
ENABLE_COVERAGE=0
INSTALL=0
INSTALL_PREFIX="/usr/local"
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
        -t|--test)
            RUN_TEST=1
            shift
            ;;
        --no-tests)
            BUILD_TESTING=0
            shift
            ;;
        --coverage)
            ENABLE_COVERAGE=1
            BUILD_TYPE="Debug"  # 覆盖率需要Debug模式
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

# 清理输出目录
if [ $CLEAN -eq 1 ]; then
    print_info "清理输出目录..."
    rm -rf "$OUTPUT_DIR"
    print_info "清理完成！"
    exit 0
fi

# 创建输出目录
if [ ! -d "$OUTPUT_DIR" ]; then
    print_info "创建输出目录: $OUTPUT_DIR"
    mkdir -p "$OUTPUT_DIR"
fi

# 创建build子目录用于CMake构建
BUILD_SUBDIR="$OUTPUT_DIR/build"
if [ ! -d "$BUILD_SUBDIR" ]; then
    mkdir -p "$BUILD_SUBDIR"
fi

# 进入build子目录
cd "$BUILD_SUBDIR"

# 运行CMake配置
print_info "运行CMake配置 (平台: $PLATFORM, 构建类型: $BUILD_TYPE)..."
CMAKE_ARGS=(
    -DPLATFORM="$PLATFORM"
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX"
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    -DBUILD_TESTING=$BUILD_TESTING
)

if [ $ENABLE_COVERAGE -eq 1 ]; then
    CMAKE_ARGS+=(-DENABLE_COVERAGE=ON)
    print_info "代码覆盖率已启用"
fi

cmake ../.. "${CMAKE_ARGS[@]}"

# 编译
print_info "开始编译..."
CPU_CORES=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

if [ "$BUILD_TARGET" = "all" ]; then
    make -j"$CPU_CORES"
else
    print_info "构建目标: $BUILD_TARGET"
    make -j"$CPU_CORES" "$BUILD_TARGET"
fi

if [ $? -eq 0 ]; then
    print_info "编译成功！"
    if [ "$BUILD_TARGET" = "all" ]; then
        print_info "目标产物: $OUTPUT_DIR/target/"
        print_info "  应用程序: $OUTPUT_DIR/target/bin/"
        print_info "  库文件:   $OUTPUT_DIR/target/lib/"
    else
        print_info "目标 $BUILD_TARGET 构建完成"
        if [ "$BUILD_TARGET" = "can_gateway" ] || [ "$BUILD_TARGET" = "protocol_converter" ]; then
            print_info "可执行文件: $OUTPUT_DIR/target/bin/$BUILD_TARGET"
        fi
    fi
else
    print_error "编译失败！"
    exit 1
fi

# 运行测试
if [ $RUN_TEST -eq 1 ]; then
    if [ $BUILD_TESTING -eq 1 ]; then
        print_info "运行测试..."
        ../target/bin/unit-test -a

        # 如果启用了覆盖率，生成报告
        if [ $ENABLE_COVERAGE -eq 1 ]; then
            print_info "生成覆盖率报告..."
            make coverage
            if [ $? -eq 0 ]; then
                print_info "覆盖率报告已生成: $OUTPUT_DIR/build/coverage_html/index.html"
            fi
        fi
    else
        print_warn "测试已禁用，跳过测试运行"
    fi
fi

# 安装
if [ $INSTALL -eq 1 ]; then
    print_info "安装到: $INSTALL_PREFIX"
    sudo make install
    print_info "安装完成！"
fi

# 返回项目根目录
cd ../..

print_info "构建完成！"
print_info ""
if [ "$BUILD_TARGET" = "all" ]; then
    print_info "输出目录结构:"
    print_info "  编译文件: $OUTPUT_DIR/build/"
    print_info "  目标产物: $OUTPUT_DIR/target/"
    print_info "    - 应用程序: target/bin/"
    print_info "    - 库文件:   target/lib/"
    print_info ""
    print_info "运行应用:"
    print_info "  ./$OUTPUT_DIR/target/bin/can_gateway"
    print_info "  ./$OUTPUT_DIR/target/bin/protocol_converter"
fi
if [ $BUILD_TESTING -eq 1 ]; then
    print_info ""
    print_info "运行测试:"
    print_info "  ./$OUTPUT_DIR/target/bin/unit-test -a    # 运行所有测试"
    print_info "  ./$OUTPUT_DIR/target/bin/unit-test -i    # 交互式菜单"
fi
if [ $ENABLE_COVERAGE -eq 1 ]; then
    print_info ""
    print_info "查看覆盖率报告:"
    print_info "  firefox $BUILD_DIR/coverage_html/index.html"
fi
print_info ""
