# tests_framework - Unit Test Framework
# 通用单元测试框架，仅依赖OSAL，提供给所有模块使用

cmake_minimum_required(VERSION 3.10)

# 目录变量
set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(tests_framework STATIC
    ${SRC}/test_runner.c
    ${SRC}/test_registry.c
    ${SRC}/test_menu.c
)

# 设置输出名称为tests_framework.a
set_target_properties(tests_framework PROPERTIES
    OUTPUT_NAME tests_framework
)

target_include_directories(tests_framework PUBLIC
    $<BUILD_INTERFACE:${INC}>
    $<INSTALL_INTERFACE:include>
)

# Public dependency: OSAL public API (headers only)
target_link_libraries(tests_framework PUBLIC
    pmc::osal_public_api
)

# Private dependency: OSAL implementation
target_link_libraries(tests_framework PRIVATE
    pmc::osal
)

# Export for use by other modules
add_library(pmc::tests_framework ALIAS tests_framework)

# Installation
install(TARGETS tests_framework
    EXPORT tests_framework-targets
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include/tests_framework
    FILES_MATCHING PATTERN "*.h"
)
