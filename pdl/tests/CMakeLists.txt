# PDL Module Tests
# Tests for PDL layer - automatically registered with libutest
#
# 支持两种编译模式：
# 1. 独立编译：在pdl目录下单独编译测试程序
#    cd pdl && mkdir build && cd build && cmake .. && make
# 2. 集成编译：在根目录编译整个项目
#    cd pmc-bsp && ./build.sh

# 检测是否为独立编译模式
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    # 独立编译模式
    cmake_minimum_required(VERSION 3.10)
    project(pdl_tests C)

    set(CMAKE_C_STANDARD 99)
    set(CMAKE_C_STANDARD_REQUIRED ON)

    # 添加编译选项
    add_compile_options(-Wall -Wextra -Werror)

    # 添加依赖模块
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../libutest ${CMAKE_BINARY_DIR}/libutest)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../osal ${CMAKE_BINARY_DIR}/osal)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../hal ${CMAKE_BINARY_DIR}/hal)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xconfig ${CMAKE_BINARY_DIR}/xconfig)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_BINARY_DIR}/pdl)

    set(STANDALONE_BUILD TRUE)
else()
    # 集成编译模式
    if(NOT BUILD_TESTING)
        return()
    endif()
    set(STANDALONE_BUILD FALSE)
endif()

# 创建PDL测试库（包含所有测试用例）
# 注意：当前PDL没有测试用例，使用占位符
add_library(pdl_test_cases STATIC
    # Add test files here when available
    # test_satellite.c
    # test_payload.c
)

# Placeholder: Create empty source file to satisfy CMake
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/pdl_tests_placeholder.c
    "/* Placeholder for PDL tests - add actual tests later */\n"
)

target_sources(pdl_test_cases PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/pdl_tests_placeholder.c
)

# 包含目录
target_include_directories(pdl_test_cases PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# 链接依赖
target_link_libraries(pdl_test_cases
    PUBLIC pmc::libutest
    PRIVATE pmc::pdl pmc::hal pmc::osal
)

# 创建独立测试可执行文件
add_executable(pdl_test_runner
    main.c
)

# 链接测试库
target_link_libraries(pdl_test_runner
    PRIVATE pdl_test_cases
    PRIVATE pmc::pdl
    PRIVATE pmc::hal
    PRIVATE pmc::osal
    PRIVATE pmc::libutest
    PRIVATE pthread
)

# 设置输出名称
set_target_properties(pdl_test_runner PROPERTIES
    OUTPUT_NAME "pdl-test"
)

# 独立编译模式：设置输出目录
if(STANDALONE_BUILD)
    set_target_properties(pdl_test_runner PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )

    message(STATUS "PDL独立测试模式")
    message(STATUS "  测试程序: ${CMAKE_BINARY_DIR}/bin/pdl-test")
endif()

# 导出别名（用于集成编译）
add_library(pmc::pdl_tests ALIAS pdl_test_cases)
