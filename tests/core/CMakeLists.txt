# tests_core - Unit Test Framework
# 通用单元测试框架，仅依赖OSAL，提供给所有模块使用

cmake_minimum_required(VERSION 3.10)

# 目录变量
set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(tests_core STATIC
    ${SRC}/test_runner.c
    ${SRC}/test_registry.c
    ${SRC}/test_menu.c
)

# 设置输出名称为tests_core.a
set_target_properties(tests_core PROPERTIES
    OUTPUT_NAME tests_core
)

target_include_directories(tests_core PUBLIC
    $<BUILD_INTERFACE:${INC}>
    $<INSTALL_INTERFACE:include>
)

# Public dependency: OSAL public API (headers only)
target_link_libraries(tests_core PUBLIC
    pmc::osal_public_api
)

# Private dependency: OSAL implementation
target_link_libraries(tests_core PRIVATE
    pmc::osal
)

# Export for use by other modules
add_library(pmc::tests_core ALIAS tests_core)

# Installation
install(TARGETS tests_core
    EXPORT tests_core-targets
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include/tests_core
    FILES_MATCHING PATTERN "*.h"
)
