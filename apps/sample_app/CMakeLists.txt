# Sample应用

# 源文件
set(SAMPLE_APP_SOURCES
    src/main.c
)

# 创建可执行文件
add_executable(sample_app ${SAMPLE_APP_SOURCES})

# 链接库
target_link_libraries(sample_app
    osal
    Threads::Threads
)

# 设置输出目录
set_target_properties(sample_app PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_TARGET}/bin
)
