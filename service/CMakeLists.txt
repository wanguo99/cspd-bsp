# Service Layer CMakeLists.txt

# 收集所有Service接口头文件
set(SERVICE_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 根据平台选择实现
if(PLATFORM STREQUAL "native" OR PLATFORM STREQUAL "generic-linux")
    set(SERVICE_IMPL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/linux)

    # Linux平台实现
    set(SERVICE_SOURCES
        ${SERVICE_IMPL_DIR}/service_satellite.c
        ${SERVICE_IMPL_DIR}/service_payload_bmc.c
        ${SERVICE_IMPL_DIR}/service_payload_linux.c
        ${SERVICE_IMPL_DIR}/service_power.c
        ${SERVICE_IMPL_DIR}/persistent_queue.c
    )
elseif(PLATFORM STREQUAL "rtems")
    set(SERVICE_IMPL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/rtems)

    # RTEMS平台实现（预留）
    set(SERVICE_SOURCES
        # TODO: RTEMS实现
    )
else()
    message(FATAL_ERROR "Unsupported platform: ${PLATFORM}")
endif()

# 创建Service静态库
add_library(service STATIC ${SERVICE_SOURCES})

# 包含目录（include包含接口和config）
target_include_directories(service PUBLIC
    ${SERVICE_INC_DIR}
)

# 私有包含下层模块（实现依赖，不暴露给上层）
target_include_directories(service PRIVATE
    ${CMAKE_SOURCE_DIR}/osal/include
    ${CMAKE_SOURCE_DIR}/hal/include
    ${CMAKE_SOURCE_DIR}/apps/can_gateway/include
)

# 链接依赖
target_link_libraries(service
    osal
    hal
)

# 编译选项
target_compile_options(service PRIVATE
    -Wall
    -Wextra
    -Werror
)

message(STATUS "Service layer configured for platform: ${PLATFORM}")
