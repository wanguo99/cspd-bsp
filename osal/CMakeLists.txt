# OSAL (操作系统抽象层) 模块

# OSAL源文件
set(OSAL_SOURCES
    linux/os_task.c
    linux/os_queue.c
    linux/os_mutex.c
    linux/os_clock.c
    linux/os_heap.c
    linux/os_error.c
    linux/os_init.c
    linux/os_log.c
)

# 创建OSAL静态库
add_library(osal STATIC ${OSAL_SOURCES})

# 包含目录
target_include_directories(osal PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
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
