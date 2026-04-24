# HAL (硬件抽象层) 模块

# HAL源文件
set(HAL_SOURCES
    src/linux/hal_can.c
    src/linux/hal_serial.c
)

# 创建HAL静态库
add_library(hal STATIC ${HAL_SOURCES})

# 包含目录（include包含接口和config）
target_include_directories(hal PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 私有包含OSAL（实现依赖，不暴露给上层）
target_include_directories(hal PRIVATE
    ${CMAKE_SOURCE_DIR}/osal/include
)

# 依赖OSAL
target_link_libraries(hal
    osal
)

# 设置编译选项
target_compile_options(hal PRIVATE
    -Wall
    -Wextra
)

message(STATUS "HAL module configured")
