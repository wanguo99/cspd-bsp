# OSAL (操作系统抽象层) 模块

# OSAL源文件
set(OSAL_SOURCES
    # IPC - 进程间通信
    src/posix/ipc/osal_task.c
    src/posix/ipc/osal_queue.c
    src/posix/ipc/osal_mutex.c
    src/posix/ipc/osal_atomic.c

    # SYS - 系统调用封装
    src/posix/sys/osal_clock.c
    src/posix/sys/osal_signal.c
    src/posix/sys/osal_file.c
    src/posix/sys/osal_select.c
    src/posix/sys/osal_env.c
    src/posix/sys/osal_time.c

    # NET - 网络相关
    src/posix/net/osal_socket.c
    src/posix/net/osal_termios.c

    # LIB - 标准库封装
    src/posix/lib/osal_heap.c
    src/posix/lib/osal_string.c
    src/posix/lib/osal_errno.c

    # UTIL - 工具类
    src/posix/util/osal_version.c
    src/posix/util/osal_log.c
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
