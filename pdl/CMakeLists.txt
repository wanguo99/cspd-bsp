# Peripheral Driver Layer (PDL) CMakeLists.txt

# 收集所有PDL接口头文件
set(PDL_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 根据平台选择实现
if(PLATFORM STREQUAL "native" OR PLATFORM STREQUAL "generic-linux")
    # Linux平台实现
    set(PDL_SOURCES
        # MCU外设驱动（独立封装）
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pdl_mcu/pdl_mcu.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pdl_mcu/pdl_mcu_can.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pdl_mcu/pdl_mcu_serial.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pdl_mcu/pdl_mcu_protocol.c

        # 卫星平台服务（独立封装）
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pdl_satellite/pdl_satellite.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pdl_satellite/pdl_satellite_can.c

        # BMC服务（独立封装）
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pdl_bmc/pdl_bmc.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pdl_bmc/pdl_bmc_redfish.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pdl_bmc/pdl_bmc_ipmi.c
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
    # 内部头文件目录
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pdl_mcu
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pdl_satellite
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pdl_bmc
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
message(STATUS "  - MCU driver: pdl_mcu.c + pdl_mcu_can.c + pdl_mcu_serial.c + pdl_mcu_protocol.c")
message(STATUS "  - Satellite driver: pdl_satellite.c + pdl_satellite_can.c")
message(STATUS "  - BMC driver: pdl_bmc.c + pdl_bmc_redfish.c + pdl_bmc_ipmi.c")
