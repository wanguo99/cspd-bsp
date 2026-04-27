# PCL (Peripheral Configuration Library) 模块
# 参考cFS设计：纯数据配置库，类似设备树

# ============================================================================
# 1. 接口库 (pcl_public_api) - 仅头文件，供上层模块使用
# ============================================================================
add_library(pcl_public_api INTERFACE)

target_include_directories(pcl_public_api INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# ============================================================================
# 2. 实现库 (pcl) - 包含配置数据和API实现
# ============================================================================

# 核心源文件
set(PCL_SOURCES
    src/pcl_api.c
    src/pcl_register.c
)

# 平台配置源文件（递归查找所有平台配置）
file(GLOB_RECURSE PLATFORM_CONFIGS
    platform/ti/am6254/*/*.c
    platform/vendor_demo/*/*/*/*.c
    # 其他平台配置可以在这里添加
    # platform/xilinx/zynq/*/*.c
)

# 创建静态库
add_library(pcl STATIC
    ${PCL_SOURCES}
    ${PLATFORM_CONFIGS}
)

# 链接接口库（继承头文件路径）
target_link_libraries(pcl PUBLIC pcl_public_api)

# 内部需要访问自己的头文件（api/internal/peripheral子目录）
target_include_directories(pcl PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/api
    ${CMAKE_CURRENT_SOURCE_DIR}/include/internal
    ${CMAKE_CURRENT_SOURCE_DIR}/include/peripheral
)

# 依赖OSAL接口库（仅头文件依赖）
target_link_libraries(pcl PUBLIC pmc::osal_public_api)

# 链接OSAL实现库（运行时依赖）
target_link_libraries(pcl PRIVATE pmc::osal)

# 设置编译选项
set(CMAKE_C_STANDARD 99)
target_compile_options(pcl PRIVATE
    -Wall
    -Wextra
    -Werror
)

# ============================================================================
# 3. 安装规则
# ============================================================================
install(TARGETS pcl pcl_public_api
    EXPORT pcl-targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include/pcl
    FILES_MATCHING PATTERN "*.h"
)

install(EXPORT pcl-targets
    FILE pcl-targets.cmake
    NAMESPACE pmc::
    DESTINATION lib/cmake/pcl
)

# ============================================================================
# 4. 配置文件（支持find_package）
# ============================================================================
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/pcl-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/pcl-config.cmake
    INSTALL_DESTINATION lib/cmake/pcl
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/pcl-config.cmake
    DESTINATION lib/cmake/pcl
)

# ============================================================================
# 5. 示例程序（可选）
# ============================================================================
option(BUILD_PCL_EXAMPLES "Build pcl examples" OFF)

if(BUILD_PCL_EXAMPLES)
    add_executable(pcl_example
        examples/pcl_example.c
    )

    target_link_libraries(pcl_example
        pmc::pcl
        pmc::osal
        pthread
    )

    install(TARGETS pcl_example
        RUNTIME DESTINATION bin
    )
endif()

message(STATUS "PCL module configured")
message(STATUS "  - Interface library: pcl_public_api")
message(STATUS "  - Implementation library: pcl")
message(STATUS "  - Platform configs: ${PLATFORM_CONFIGS}")
