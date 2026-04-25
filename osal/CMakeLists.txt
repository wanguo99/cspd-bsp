# OSAL (操作系统抽象层) 模块

# OSAL源文件
set(OSAL_SOURCES
    src/linux/osal_task.c
    src/linux/osal_queue.c
    src/linux/osal_mutex.c
    src/linux/osal_clock.c
    src/linux/osal_heap.c
    src/linux/osal_error.c
    src/linux/osal_init.c
    src/linux/osal_log.c
    src/linux/osal_file.c
    src/linux/osal_network.c
    src/linux/osal_signal.c
    src/linux/osal_string.c
)

# 创建OSAL静态库
add_library(osal STATIC ${OSAL_SOURCES})

# 包含目录（include包含接口和config）
target_include_directories(osal PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 链接库
target_link_libraries(osal
    Threads::Threads
    rt
)

# 设置编译选项
target_compile_options(osal PRIVATE
    -Wall
    -Wextra
)

message(STATUS "OSAL module configured")
