# libutest - Unit Test Framework
# 通用单元测试框架，仅依赖OSAL，提供给所有模块使用

cmake_minimum_required(VERSION 3.10)

add_library(libutest STATIC
    src/test_runner.c
    src/test_registry.c
    src/test_menu.c
)

# 设置输出名称为libutest.a
set_target_properties(libutest PROPERTIES
    OUTPUT_NAME utest
)

target_include_directories(libutest PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Public dependency: OSAL public API (headers only)
target_link_libraries(libutest PUBLIC
    pmc::osal_public_api
)

# Private dependency: OSAL implementation
target_link_libraries(libutest PRIVATE
    pmc::osal
)

# Export for use by other modules
add_library(pmc::libutest ALIAS libutest)

# Installation
install(TARGETS libutest
    EXPORT libutest-targets
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include/libutest
    FILES_MATCHING PATTERN "*.h"
)
