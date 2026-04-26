# Peripheral Driver Layer (PDL) 模块
# 参考cFS应用模块设计：统一外设管理层

# ============================================================================
# 1. 接口库 (pdl_public_api) - 仅头文件，供上层模块使用
# ============================================================================
add_library(pdl_public_api INTERFACE)

target_include_directories(pdl_public_api INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# ============================================================================
# 2. 实现库 (pdl) - 包含外设驱动实现
# ============================================================================

# 根据平台选择实现
if(PLATFORM STREQUAL "native" OR PLATFORM STREQUAL "generic-linux")
    # Linux平台实现
    set(PDL_SOURCES
        # MCU外设驱动（独立封装）
        src/pdl_mcu/pdl_mcu.c
        src/pdl_mcu/pdl_mcu_can.c
        src/pdl_mcu/pdl_mcu_serial.c
        src/pdl_mcu/pdl_mcu_protocol.c

        # 卫星平台服务（独立封装）
        src/pdl_satellite/pdl_satellite.c
        src/pdl_satellite/pdl_satellite_can.c

        # BMC服务（独立封装）
        src/pdl_bmc/pdl_bmc.c
        src/pdl_bmc/pdl_bmc_redfish.c
        src/pdl_bmc/pdl_bmc_ipmi.c
    )
elseif(PLATFORM STREQUAL "rtems")
    # RTEMS平台实现（预留）
    set(PDL_SOURCES
        # TODO: RTEMS实现
    )
else()
    message(FATAL_ERROR "Unsupported platform: ${PLATFORM}")
endif()

# 创建PDL静态库
add_library(pdl STATIC ${PDL_SOURCES})

# 链接接口库（继承头文件路径）
target_link_libraries(pdl PUBLIC pdl_public_api)

# 依赖下层接口库（仅头文件依赖）
target_link_libraries(pdl PUBLIC
    pmc::osal_public_api
    pmc::hal_public_api
    pmc::xconfig_public_api
)

# 链接下层实现库（运行时依赖）
target_link_libraries(pdl PRIVATE
    pmc::osal
    pmc::hal
    pmc::xconfig
)

# 内部头文件目录（私有）
target_include_directories(pdl PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pdl_mcu
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pdl_satellite
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pdl_bmc
)

# 编译选项
target_compile_options(pdl PRIVATE
    -Wall
    -Wextra
    -Werror
)

# ============================================================================
# 3. 安装规则
# ============================================================================
install(TARGETS pdl pdl_public_api
    EXPORT pdl-targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include/pdl
    FILES_MATCHING PATTERN "*.h"
)

install(EXPORT pdl-targets
    FILE pdl-targets.cmake
    NAMESPACE pmc::
    DESTINATION lib/cmake/pdl
)

# ============================================================================
# 4. 配置文件（支持find_package）
# ============================================================================
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/pdl-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/pdl-config.cmake
    INSTALL_DESTINATION lib/cmake/pdl
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/pdl-config.cmake
    DESTINATION lib/cmake/pdl
)

message(STATUS "Peripheral Driver Layer (PDL) configured for platform: ${PLATFORM}")
message(STATUS "  - Interface library: pdl_public_api")
message(STATUS "  - Implementation library: pdl")
message(STATUS "  - MCU driver: pdl_mcu.c + pdl_mcu_can.c + pdl_mcu_serial.c + pdl_mcu_protocol.c")
message(STATUS "  - Satellite driver: pdl_satellite.c + pdl_satellite_can.c")
message(STATUS "  - BMC driver: pdl_bmc.c + pdl_bmc_redfish.c + pdl_bmc_ipmi.c")
