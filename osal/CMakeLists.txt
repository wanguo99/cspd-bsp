# OSAL (操作系统抽象层) 模块
# 参考cFS OSAL设计：提供接口库和实现库

# ============================================================================
# 1. 接口库 (osal_public_api) - 仅头文件，供上层模块使用
# ============================================================================
add_library(osal_public_api INTERFACE)

target_include_directories(osal_public_api INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# ============================================================================
# 2. 实现库 (osal) - 包含平台相关实现
# ============================================================================

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
    src/posix/sys/osal_termios.c
    src/posix/sys/osal_process.c
    src/posix/sys/osal_thread.c

    # NET - 网络相关
    src/posix/net/osal_socket.c

    # LIB - 标准库封装
    src/posix/lib/osal_heap.c
    src/posix/lib/osal_string.c
    src/posix/lib/osal_stdio.c
    src/posix/lib/osal_errno.c

    # UTIL - 工具类
    src/posix/util/osal_version.c
    src/posix/util/osal_log.c
)

# 创建OSAL静态库
add_library(osal STATIC ${OSAL_SOURCES})

# 链接接口库（继承头文件路径）
target_link_libraries(osal PUBLIC osal_public_api)

# 链接系统库
target_link_libraries(osal PUBLIC
    Threads::Threads
    rt
)

# 设置编译选项
target_compile_options(osal PRIVATE
    -Wall
    -Wextra
)

# ============================================================================
# 3. 安装规则
# ============================================================================
install(TARGETS osal osal_public_api
    EXPORT osal-targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include/osal
    FILES_MATCHING PATTERN "*.h"
)

install(EXPORT osal-targets
    FILE osal-targets.cmake
    NAMESPACE pmc::
    DESTINATION lib/cmake/osal
)

message(STATUS "OSAL configured")
