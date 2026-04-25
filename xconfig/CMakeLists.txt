cmake_minimum_required(VERSION 3.10)

# 硬件配置库
project(xconfig C)

# 编译选项
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")

# 包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/api
    ${CMAKE_CURRENT_SOURCE_DIR}/include/internal
    ${CMAKE_CURRENT_SOURCE_DIR}/include/peripheral
    ${CMAKE_SOURCE_DIR}/osal/include
    ${CMAKE_SOURCE_DIR}/osal/include/config
    ${CMAKE_SOURCE_DIR}/hal/include
    ${CMAKE_SOURCE_DIR}/hal/include/config
    ${CMAKE_SOURCE_DIR}/pdl/include
)

# 源文件
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

# 链接依赖
target_link_libraries(xconfig
    osal
)

# 安装头文件（分层安装）
# 对外API（供PDL层使用，仅xconfig_api.h）
install(FILES
    include/api/xconfig_api.h
    DESTINATION include/xconfig/api
)

# 内部公共头文件（含xconfig.h总头文件）
install(FILES
    include/internal/xconfig.h
    include/internal/xconfig_common.h
    include/internal/xconfig_board.h
    DESTINATION include/xconfig/internal
)

# 外设私有头文件
install(FILES
    include/peripheral/xconfig_hardware_interface.h
    include/peripheral/xconfig_mcu.h
    include/peripheral/xconfig_bmc.h
    include/peripheral/xconfig_satellite.h
    include/peripheral/xconfig_sensor.h
    include/peripheral/xconfig_storage.h
    include/peripheral/xconfig_app.h
    DESTINATION include/xconfig/peripheral
)

# 为了兼容性，在顶层include目录创建xconfig_api.h的符号链接
install(CODE "
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        xconfig/api/xconfig_api.h
        \${CMAKE_INSTALL_PREFIX}/include/xconfig_api.h
    )
")

# 安装库文件
install(TARGETS xconfig
    ARCHIVE DESTINATION lib
)

# 示例程序（可选）
option(BUILD_XCONFIG_EXAMPLES "Build xconfig examples" OFF)

if(BUILD_XCONFIG_EXAMPLES)
    add_executable(xconfig_example
        examples/xconfig_example.c
    )

    target_link_libraries(xconfig_example
        xconfig
        pdl
        hal
        osal
        pthread
    )

    install(TARGETS xconfig_example
        RUNTIME DESTINATION bin
    )
endif()
