cmake_minimum_required(VERSION 3.10)

project(PMC-BSP VERSION 1.0.0 LANGUAGES C)

# 设置C标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ============================================================================
# 构建选项
# ============================================================================
option(BUILD_TESTING "Build unit tests" ON)
option(BUILD_SHARED_LIBS "Build shared libraries instead of static" ON)

# ============================================================================
# 平台配置
# ============================================================================
if(NOT DEFINED PLATFORM)
    set(PLATFORM "native" CACHE STRING "Target platform: native, arm32-linux, arm64-linux, riscv64-linux")
endif()

message(STATUS "Target platform: ${PLATFORM}")

# 根据平台设置编译器和选项
if(PLATFORM STREQUAL "native")
    # 本地编译 - 使用系统默认编译器
    message(STATUS "Building for native platform")

elseif(PLATFORM STREQUAL "arm32-linux")
    # ARM32 Linux 交叉编译
    message(STATUS "Building for ARM32 Linux (cross-compile)")

    # 设置交叉编译工具链
    if(NOT DEFINED CMAKE_C_COMPILER)
        set(CMAKE_C_COMPILER "arm-linux-gnueabihf-gcc" CACHE STRING "C compiler")
    endif()

    # 设置系统信息
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR arm)

    # ARM32 特定编译选项
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv7-a -mfpu=neon-vfpv4 -mfloat-abi=hard")

    # 设置查找路径
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

elseif(PLATFORM STREQUAL "arm64-linux")
    # ARM64 Linux 交叉编译
    message(STATUS "Building for ARM64 Linux (cross-compile)")

    # 设置交叉编译工具链
    if(NOT DEFINED CMAKE_C_COMPILER)
        set(CMAKE_C_COMPILER "aarch64-linux-gnu-gcc" CACHE STRING "C compiler")
    endif()

    # 设置系统信息
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR aarch64)

    # ARM64 特定编译选项
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv8-a")

    # 设置查找路径
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

elseif(PLATFORM STREQUAL "riscv64-linux")
    # RISC-V 64 Linux 交叉编译
    message(STATUS "Building for RISC-V 64 Linux (cross-compile)")

    # 设置交叉编译工具链
    if(NOT DEFINED CMAKE_C_COMPILER)
        set(CMAKE_C_COMPILER "riscv64-linux-gnu-gcc" CACHE STRING "C compiler")
    endif()

    # 设置系统信息
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR riscv64)

    # RISC-V 64 特定编译选项
    # rv64imafdc = RV64I base + M(乘除法) + A(原子) + F(单精度浮点) + D(双精度浮点) + C(压缩指令)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=rv64imafdc -mabi=lp64d")

    # 设置查找路径
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

elseif(PLATFORM STREQUAL "generic-linux")
    # 保留旧的 generic-linux 选项以兼容现有脚本
    message(WARNING "PLATFORM=generic-linux is deprecated, use arm32-linux instead")
    message(STATUS "Building for generic Linux (cross-compile)")

    # 设置交叉编译工具链
    if(NOT DEFINED CMAKE_C_COMPILER)
        set(CMAKE_C_COMPILER "arm-linux-gnueabihf-gcc" CACHE STRING "C compiler")
    endif()

    # 设置系统信息
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR arm)

    # 设置查找路径
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

else()
    message(FATAL_ERROR "Unknown platform: ${PLATFORM}. Supported: native, arm32-linux, arm64-linux, riscv64-linux")
endif()

# ============================================================================
# 编译选项
# ============================================================================
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -D_POSIX_C_SOURCE=200809L")
set(CMAKE_C_FLAGS_DEBUG "-g -O0")
set(CMAKE_C_FLAGS_RELEASE "-O2")

# ============================================================================
# 输出目录配置（参考buildroot设计）
# ============================================================================
set(OUTPUT_BUILD ${CMAKE_BINARY_DIR})
set(OUTPUT_TARGET ${CMAKE_BINARY_DIR}/../target)

# CMake中间文件目录（当前构建目录）
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_BUILD}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_BUILD}/lib)

# 最终目标产物目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_TARGET}/bin)

# ============================================================================
# 查找依赖库
# ============================================================================
find_package(Threads REQUIRED)

# ============================================================================
# 模块化构建
# ============================================================================
# 依赖层次：OSAL → HAL → XConfig → PDL → Apps
#
# 每个模块提供两个库：
# 1. <module>_public_api - 接口库（仅头文件），供上层使用
# 2. <module> - 实现库（静态库），包含实际代码
#
# 上层模块：
# - PUBLIC依赖下层的接口库（继承头文件路径）
# - PRIVATE依赖下层的实现库（运行时链接）

# 1. OSAL层（操作系统抽象层）
add_subdirectory(osal)

# 为后续模块创建命名空间别名
add_library(pmc::osal_public_api ALIAS osal_public_api)
add_library(pmc::osal ALIAS osal)

# 1.5 Test Library（测试库 - 仅在启用测试时构建）
# 现在由统一的tests/CMakeLists.txt管理

# 2. HAL层（硬件抽象层）
add_subdirectory(hal)
add_library(pmc::hal_public_api ALIAS hal_public_api)
add_library(pmc::hal ALIAS hal)

# 3. XConfig层（硬件配置库）
add_subdirectory(pcl)
add_library(pmc::pcl_public_api ALIAS pcl_public_api)
add_library(pmc::pcl ALIAS pcl)

# 4. PDL层（外设驱动层）
add_subdirectory(pdl)
add_library(pmc::pdl_public_api ALIAS pdl_public_api)
add_library(pmc::pdl ALIAS pdl)

# 5. Apps层（应用层）
add_subdirectory(apps)

# ============================================================================
# 测试框架（可选）
# ============================================================================
if(BUILD_TESTING)
    # 统一测试框架（包含所有测试模块和运行器）
    add_subdirectory(tests)
endif()

# ============================================================================
# 创建输出目录结构
# ============================================================================
file(MAKE_DIRECTORY ${OUTPUT_BUILD}/lib)
file(MAKE_DIRECTORY ${OUTPUT_TARGET}/bin)
file(MAKE_DIRECTORY ${OUTPUT_TARGET}/lib)

# ============================================================================
# 复制最终库文件到target/lib（构建后执行）
# ============================================================================
add_custom_target(copy_libs ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${OUTPUT_BUILD}/lib ${OUTPUT_TARGET}/lib
    COMMENT "Copying libraries to target directory"
    DEPENDS osal hal pcl pdl
)

# ============================================================================
# 打印配置信息
# ============================================================================
message(STATUS "")
message(STATUS "========================================")
message(STATUS "PMC-BSP v${PROJECT_VERSION} | ${PLATFORM} | ${CMAKE_BUILD_TYPE}")
if(BUILD_TESTING)
message(STATUS "Tests: Enabled | Output: ${OUTPUT_TARGET}")
else()
message(STATUS "Tests: Disabled | Output: ${OUTPUT_TARGET}")
endif()
message(STATUS "========================================")
