#!/bin/bash

# CSPD-BSP 测试构建脚本

set -e

echo "========================================="
echo "  CSPD-BSP 测试构建脚本"
echo "========================================="

# 创建构建目录
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# 配置CMake
echo ""
echo "配置CMake..."
cmake -DBUILD_TESTING=ON ..

# 编译测试
echo ""
echo "编译测试..."
cmake --build . --target all_tests

echo ""
echo "========================================="
echo "  构建完成！"
echo "========================================="
echo ""
echo "运行测试："
echo "  cd build && ctest --output-on-failure"
echo ""
