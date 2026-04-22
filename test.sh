#!/bin/bash

# CSPD-BSP 测试构建和运行脚本

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_header() {
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}========================================${NC}"
}

# 显示帮助
show_help() {
    cat << EOF
用法: $0 [选项]

选项:
    -h, --help          显示帮助信息
    -c, --clean         清理构建目录
    -b, --build         仅构建测试
    -r, --run           构建并运行测试
    -v, --verbose       详细输出
    --coverage          生成覆盖率报告
    --osal              仅运行OSAL层测试
    --hal               仅运行HAL层测试
    --service           仅运行Service层测试
    --apps              仅运行Apps层测试

示例:
    $0 -r               # 构建并运行所有测试
    $0 -r --osal        # 仅运行OSAL层测试
    $0 --coverage       # 生成覆盖率报告
    $0 -c -r            # 清理后重新构建并运行

EOF
}

# 默认参数
CLEAN=0
BUILD=1
RUN=0
VERBOSE=0
COVERAGE=0
TEST_FILTER=""
BUILD_DIR="build"

# 解析参数
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
        -b|--build)
            BUILD=1
            RUN=0
            shift
            ;;
        -r|--run)
            BUILD=1
            RUN=1
            shift
            ;;
        -v|--verbose)
            VERBOSE=1
            shift
            ;;
        --coverage)
            COVERAGE=1
            shift
            ;;
        --osal)
            TEST_FILTER="test_os_*"
            shift
            ;;
        --hal)
            TEST_FILTER="test_hal_*"
            shift
            ;;
        --service)
            TEST_FILTER="test_payload_*"
            shift
            ;;
        --apps)
            TEST_FILTER="test_can_*"
            shift
            ;;
        *)
            print_error "未知选项: $1"
            show_help
            exit 1
            ;;
    esac
done

# 清理
if [ $CLEAN -eq 1 ]; then
    print_info "清理构建目录..."
    rm -rf "$BUILD_DIR"
    print_info "清理完成"
fi

# 创建构建目录
if [ ! -d "$BUILD_DIR" ]; then
    print_info "创建构建目录: $BUILD_DIR"
    mkdir -p "$BUILD_DIR"
fi

cd "$BUILD_DIR"

# 构建
if [ $BUILD -eq 1 ]; then
    print_header "配置CMake"

    CMAKE_ARGS=(
        -DCMAKE_BUILD_TYPE=Debug
        -DBUILD_TESTING=ON
    )

    if [ $COVERAGE -eq 1 ]; then
        CMAKE_ARGS+=(-DENABLE_COVERAGE=ON)
        print_info "代码覆盖率已启用"
    fi

    cmake .. "${CMAKE_ARGS[@]}"

    print_header "构建测试"
    CPU_CORES=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    make all_tests -j"$CPU_CORES"

    if [ $? -eq 0 ]; then
        print_info "测试构建成功"
    else
        print_error "测试构建失败"
        exit 1
    fi
fi

# 运行测试
if [ $RUN -eq 1 ]; then
    print_header "运行测试"

    CTEST_ARGS=(--output-on-failure)

    if [ $VERBOSE -eq 1 ]; then
        CTEST_ARGS+=(-V)
    fi

    if [ -n "$TEST_FILTER" ]; then
        CTEST_ARGS+=(-R "$TEST_FILTER")
        print_info "测试过滤器: $TEST_FILTER"
    fi

    ctest "${CTEST_ARGS[@]}"

    TEST_RESULT=$?

    if [ $TEST_RESULT -eq 0 ]; then
        print_info "所有测试通过 ✓"
    else
        print_error "部分测试失败 ✗"
        exit 1
    fi

    # 生成覆盖率报告
    if [ $COVERAGE -eq 1 ]; then
        print_header "生成覆盖率报告"
        make coverage

        if [ $? -eq 0 ]; then
            print_info "覆盖率报告已生成"
            print_info "查看报告: firefox $BUILD_DIR/coverage_html/index.html"
        else
            print_warn "覆盖率报告生成失败"
        fi
    fi
fi

cd ..

print_header "完成"
print_info "测试可执行文件位于: $BUILD_DIR/"
print_info ""
print_info "手动运行测试:"
print_info "  cd $BUILD_DIR && ctest --output-on-failure"
print_info ""
print_info "运行单个测试:"
print_info "  ./$BUILD_DIR/test_os_task"
print_info "  ./$BUILD_DIR/test_hal_can"
print_info ""
