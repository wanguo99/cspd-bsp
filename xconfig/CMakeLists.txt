# XConfig (硬件配置库) 模块
# 参考cFS设计：纯数据配置库，类似设备树

# ============================================================================
# 1. 接口库 (xconfig_public_api) - 仅头文件，供上层模块使用
# ============================================================================
add_library(xconfig_public_api INTERFACE)

target_include_directories(xconfig_public_api INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# ============================================================================
# 2. 实现库 (xconfig) - 包含配置数据和API实现
# ============================================================================

# 核心源文件
set(XCONFIG_SOURCES
    src/xconfig_api.c
    src/xconfig_register.c
)

# 平台配置源文件（递归查找所有平台配置）
file(GLOB_RECURSE PLATFORM_CONFIGS
    platform/ti/am6254/*/*.c
    platform/vendor_demo/*/*/*/*.c
    # 其他平台配置可以在这里添加
    # platform/xilinx/zynq/*/*.c
)

# 创建静态库
add_library(xconfig STATIC
    ${XCONFIG_SOURCES}
    ${PLATFORM_CONFIGS}
)

# 链接接口库（继承头文件路径）
target_link_libraries(xconfig PUBLIC xconfig_public_api)

# 内部需要访问自己的头文件（api/internal/peripheral子目录）
target_include_directories(xconfig PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/api
    ${CMAKE_CURRENT_SOURCE_DIR}/include/internal
    ${CMAKE_CURRENT_SOURCE_DIR}/include/peripheral
)

# 依赖OSAL接口库（仅头文件依赖）
target_link_libraries(xconfig PUBLIC pmc::osal_public_api)

# 链接OSAL实现库（运行时依赖）
target_link_libraries(xconfig PRIVATE pmc::osal)

# 设置编译选项
set(CMAKE_C_STANDARD 99)
target_compile_options(xconfig PRIVATE
    -Wall
    -Wextra
    -Werror
)

# ============================================================================
# 3. 安装规则
# ============================================================================
install(TARGETS xconfig xconfig_public_api
    EXPORT xconfig-targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include/xconfig
    FILES_MATCHING PATTERN "*.h"
)

install(EXPORT xconfig-targets
    FILE xconfig-targets.cmake
    NAMESPACE pmc::
    DESTINATION lib/cmake/xconfig
)

# ============================================================================
# 4. 配置文件（支持find_package）
# ============================================================================
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/xconfig-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/xconfig-config.cmake
    INSTALL_DESTINATION lib/cmake/xconfig
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/xconfig-config.cmake
    DESTINATION lib/cmake/xconfig
)

# ============================================================================
# 5. 示例程序（可选）
# ============================================================================
option(BUILD_XCONFIG_EXAMPLES "Build xconfig examples" OFF)

if(BUILD_XCONFIG_EXAMPLES)
    add_executable(xconfig_example
        examples/xconfig_example.c
    )

    target_link_libraries(xconfig_example
        pmc::xconfig
        pmc::osal
        pthread
    )

    install(TARGETS xconfig_example
        RUNTIME DESTINATION bin
    )
endif()

message(STATUS "XConfig module configured")
message(STATUS "  - Interface library: xconfig_public_api")
message(STATUS "  - Implementation library: xconfig")
message(STATUS "  - Platform configs: ${PLATFORM_CONFIGS}")
