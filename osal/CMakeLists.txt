# OSAL (操作系统抽象层) 模块

# OSAL源文件
set(OSAL_SOURCES
    # IPC - 进程间通信
    src/linux/ipc/osal_task.c
    src/linux/ipc/osal_queue.c
    src/linux/ipc/osal_mutex.c
    src/linux/ipc/osal_atomic.c

    # SYS - 系统调用封装
    src/linux/sys/osal_clock.c
    src/linux/sys/osal_signal.c
    src/linux/sys/osal_unistd.c
    src/linux/sys/osal_select.c
    src/linux/sys/osal_env.c

    # NET - 网络相关
    src/linux/net/osal_socket.c
    src/linux/net/osal_termios.c

    # LIB - 标准库封装
    src/linux/lib/osal_heap.c
    src/linux/lib/osal_string.c
    src/linux/lib/osal_errno.c

    # UTIL - 工具类
    src/linux/util/osal_error.c
    src/linux/util/osal_init.c
    src/linux/util/osal_log.c
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
