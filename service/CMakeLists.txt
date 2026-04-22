# Service Layer CMakeLists.txt

# 收集所有Service接口头文件
set(SERVICE_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/inc)

# 根据平台选择实现
if(PLATFORM STREQUAL "native" OR PLATFORM STREQUAL "generic-linux")
    set(SERVICE_IMPL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/linux)

    # Linux平台实现
    set(SERVICE_SOURCES
        ${SERVICE_IMPL_DIR}/service_satellite.c
        ${SERVICE_IMPL_DIR}/service_payload_bmc.c
        ${SERVICE_IMPL_DIR}/service_payload_linux.c
        ${SERVICE_IMPL_DIR}/service_power.c
    )
elseif(PLATFORM STREQUAL "rtems")
    set(SERVICE_IMPL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/rtems)

    # RTEMS平台实现（预留）
    set(SERVICE_SOURCES
        # TODO: RTEMS实现
    )
else()
    message(FATAL_ERROR "Unsupported platform: ${PLATFORM}")
endif()

# 创建Service静态库
add_library(service STATIC ${SERVICE_SOURCES})

# 包含目录
target_include_directories(service PUBLIC
    ${SERVICE_INC_DIR}
    ${CMAKE_SOURCE_DIR}/osal/inc
    ${CMAKE_SOURCE_DIR}/hal/inc
    ${CMAKE_SOURCE_DIR}/config
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
