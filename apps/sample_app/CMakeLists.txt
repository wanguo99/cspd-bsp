# Sample应用
# 

# 目录变量
set(SAMPLE_APP_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# 源文件
set(SAMPLE_APP_SOURCES
    ${SAMPLE_APP_SRC_DIR}/main.c
)

# 创建可执行文件
add_executable(sample_app ${SAMPLE_APP_SOURCES})

# 链接依赖（使用命名空间别名）
target_link_libraries(sample_app
    pmc::osal
    Threads::Threads
)

# 设置输出目录
set_target_properties(sample_app PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_TARGET}/bin
)

# 安装规则
install(TARGETS sample_app
    RUNTIME DESTINATION bin
)

message(STATUS "Sample app configured")
