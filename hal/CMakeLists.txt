# HAL (硬件抽象层) 模块
# 参考cFS PSP设计：提供接口库和平台相关实现

# ============================================================================
# 1. 接口库 (hal_public_api) - 仅头文件，供上层模块使用
# ============================================================================
add_library(hal_public_api INTERFACE)

target_include_directories(hal_public_api INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# ============================================================================
# 2. 实现库 (hal) - 包含平台相关驱动
# ============================================================================

# HAL源文件（根据平台选择）
if(PLATFORM STREQUAL "native" OR PLATFORM STREQUAL "generic-linux")
    set(HAL_SOURCES
        src/linux/hal_can.c
        src/linux/hal_serial.c
    )
elseif(PLATFORM STREQUAL "rtems")
    # RTEMS平台实现（预留）
    set(HAL_SOURCES
        # TODO: RTEMS实现
    )
else()
    message(FATAL_ERROR "Unsupported platform: ${PLATFORM}")
endif()

# 创建HAL静态库
add_library(hal STATIC ${HAL_SOURCES})

# 链接接口库（继承头文件路径）
target_link_libraries(hal PUBLIC hal_public_api)

# 依赖OSAL接口库（仅头文件依赖）
target_link_libraries(hal PUBLIC pmc::osal_public_api)

# 链接OSAL实现库（运行时依赖）
target_link_libraries(hal PRIVATE pmc::osal)

# 设置编译选项
target_compile_options(hal PRIVATE
    -Wall
    -Wextra
)

# ============================================================================
# 3. 安装规则
# ============================================================================
install(TARGETS hal hal_public_api
    EXPORT hal-targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include/hal
    FILES_MATCHING PATTERN "*.h"
)

install(EXPORT hal-targets
    FILE hal-targets.cmake
    NAMESPACE pmc::
    DESTINATION lib/cmake/hal
)

message(STATUS "")
message(STATUS "HAL module configured for platform: ${PLATFORM}")
message(STATUS "  - Interface library: hal_public_api")
message(STATUS "  - Implementation library: hal")
message(STATUS "")
