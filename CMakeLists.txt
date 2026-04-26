cmake_minimum_required(VERSION 3.10)

project(PMC-BSP VERSION 1.0.0 LANGUAGES C)

# 设置C标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ============================================================================
# 构建选项
# ============================================================================
option(BUILD_TESTING "Build unit tests" ON)

# ============================================================================
# 平台配置
# ============================================================================
if(NOT DEFINED PLATFORM)
    set(PLATFORM "native" CACHE STRING "Target platform: native, generic-linux")
endif()

message(STATUS "Target platform: ${PLATFORM}")

# 根据平台设置编译器和选项
if(PLATFORM STREQUAL "native")
    # 本地编译 - 使用系统默认编译器
    message(STATUS "Building for native platform")

elseif(PLATFORM STREQUAL "generic-linux")
    # 标准Linux交叉编译
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
    message(FATAL_ERROR "Unknown platform: ${PLATFORM}. Supported: native, generic-linux")
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
# 模块化构建（参考cFS架构）
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
if(BUILD_TESTING)
    add_subdirectory(libutest)
endif()

# OSAL模块测试
if(BUILD_TESTING)
    add_subdirectory(osal/tests)
endif()

# 2. HAL层（硬件抽象层）
add_subdirectory(hal)
add_library(pmc::hal_public_api ALIAS hal_public_api)
add_library(pmc::hal ALIAS hal)

# HAL模块测试
if(BUILD_TESTING)
    add_subdirectory(hal/tests)
endif()

# 3. XConfig层（硬件配置库）
add_subdirectory(xconfig)
add_library(pmc::xconfig_public_api ALIAS xconfig_public_api)
add_library(pmc::xconfig ALIAS xconfig)

# 4. PDL层（外设驱动层）
add_subdirectory(pdl)
add_library(pmc::pdl_public_api ALIAS pdl_public_api)
add_library(pmc::pdl ALIAS pdl)

# PDL模块测试
if(BUILD_TESTING)
    add_subdirectory(pdl/tests)
endif()

# 5. Apps层（应用层）
add_subdirectory(apps)

# ============================================================================
# 测试框架（可选）
# ============================================================================
if(BUILD_TESTING)
    message(STATUS "Unit tests enabled")
    # 统一测试运行器
    add_subdirectory(apps/test_runner)
else()
    message(STATUS "Unit tests disabled")
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
    DEPENDS osal hal xconfig pdl
)

# ============================================================================
# 打印配置信息
# ============================================================================
message(STATUS "")
message(STATUS "========================================")
message(STATUS "  PMC-BSP Configuration")
message(STATUS "========================================")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Platform: ${PLATFORM}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "  C Flags: ${CMAKE_C_FLAGS}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Build directory: ${OUTPUT_BUILD}")
message(STATUS "  Target directory: ${OUTPUT_TARGET}")
if(BUILD_TESTING)
message(STATUS "  Unit tests: Enabled")
else()
message(STATUS "  Unit tests: Disabled")
endif()
message(STATUS "========================================")
message(STATUS "Module Dependency Chain:")
message(STATUS "  OSAL (OS Abstraction)")
message(STATUS "    ↓")
message(STATUS "  HAL (Hardware Drivers)")
message(STATUS "    ↓")
message(STATUS "  XConfig (Hardware Config)")
message(STATUS "    ↓")
message(STATUS "  PDL (Peripheral Drivers)")
message(STATUS "    ↓")
message(STATUS "  Apps (Applications)")
message(STATUS "========================================")
message(STATUS "")
