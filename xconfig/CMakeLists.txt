cmake_minimum_required(VERSION 3.10)

# 硬件配置库
project(xconfig C)

# 编译选项
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")

# 包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
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
    platform/ti/am625/*/*.c
    platform/nvidia/mcu_test/*/*.c
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

# 安装头文件
install(FILES
    include/xconfig.h
    include/xconfig_api.h
    include/xconfig_common.h
    include/xconfig_hardware_interface.h
    include/xconfig_mcu.h
    include/xconfig_bmc.h
    include/xconfig_satellite.h
    include/xconfig_sensor.h
    include/xconfig_storage.h
    include/xconfig_app.h
    include/xconfig_board.h
    DESTINATION include
)

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
