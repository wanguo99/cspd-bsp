# Peripheral Driver Layer (PDL) CMakeLists.txt

# 收集所有PDL接口头文件
set(PDL_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 根据平台选择实现
if(PLATFORM STREQUAL "native" OR PLATFORM STREQUAL "generic-linux")
    # Linux平台实现
    set(PDL_SOURCES
        # 外设管理核心
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pdl_core/pdl_peripheral_core.c

        # 外设驱动实现（按外设类型分类）
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pdl_mcu/pdl_peripheral_mcu.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pdl_satellite/pdl_satellite.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pdl_payload_bmc/pdl_payload_bmc.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pdl_payload_os/pdl_payload_os.c

        # 通用服务
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pdl_common/pdl_watchdog.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pdl_common/pdl_persistent_queue.c
    )
elseif(PLATFORM STREQUAL "rtems")
    set(PDL_IMPL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/rtems)

    # RTEMS平台实现（预留）
    set(PDL_SOURCES
        # TODO: RTEMS实现
    )
else()
    message(FATAL_ERROR "Unsupported platform: ${PLATFORM}")
endif()

# 创建PDL静态库
add_library(pdl STATIC ${PDL_SOURCES})

# 包含目录（include包含接口和config）
target_include_directories(pdl PUBLIC
    ${PDL_INC_DIR}
)

# 私有包含下层模块（实现依赖，不暴露给上层）
target_include_directories(pdl PRIVATE
    ${CMAKE_SOURCE_DIR}/osal/include
    ${CMAKE_SOURCE_DIR}/hal/include
    ${CMAKE_SOURCE_DIR}/apps/can_gateway/include
)

# 链接依赖
target_link_libraries(pdl
    osal
    hal
)

# 编译选项
target_compile_options(pdl PRIVATE
    -Wall
    -Wextra
    -Werror
)

message(STATUS "Peripheral Driver Layer (PDL) configured for platform: ${PLATFORM}")
message(STATUS "  - Peripheral management framework enabled")
message(STATUS "  - Peripheral drivers: MCU, Satellite, BMC, Linux")
message(STATUS "  - Common services: Watchdog, Persistent Queue")
