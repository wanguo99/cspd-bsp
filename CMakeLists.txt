cmake_minimum_required(VERSION 3.10)

project(CSPD-BSP VERSION 1.0.0 LANGUAGES C)

# 设置C标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 编译平台选择
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

# 编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -D_POSIX_C_SOURCE=200809L")
set(CMAKE_C_FLAGS_DEBUG "-g -O0")
set(CMAKE_C_FLAGS_RELEASE "-O2")

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/osal/inc
    ${CMAKE_SOURCE_DIR}/hal/inc
    ${CMAKE_SOURCE_DIR}/config
)

# 查找依赖库
find_package(Threads REQUIRED)

# 添加子目录
add_subdirectory(osal)
add_subdirectory(hal)
add_subdirectory(service)
add_subdirectory(apps)

# 添加测试（可选，通过BUILD_TESTING控制）
option(BUILD_TESTING "Build unit tests" ON)
if(BUILD_TESTING)
    message(STATUS "Unit tests enabled")
    add_subdirectory(tests)
else()
    message(STATUS "Unit tests disabled")
endif()

# 主程序
add_executable(cspd-bsp
    main.c
)

# 链接库
target_link_libraries(cspd-bsp
    osal
    hal
    service
    apps
    Threads::Threads
    rt
)

# 安装规则
install(TARGETS cspd-bsp
    RUNTIME DESTINATION bin
)

# 打印配置信息
message(STATUS "")
message(STATUS "========================================")
message(STATUS "  CSPD-BSP Configuration")
message(STATUS "========================================")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Platform: ${PLATFORM}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "  C Flags: ${CMAKE_C_FLAGS}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
if(BUILD_TESTING)
message(STATUS "  Unit tests: Enabled")
else()
message(STATUS "  Unit tests: Disabled")
endif()
message(STATUS "========================================")
message(STATUS "")
