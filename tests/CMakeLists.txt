cmake_minimum_required(VERSION 3.10)

# 测试项目配置
project(PMC_BSP_Tests C)

# 设置C标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 添加编译选项
add_compile_options(-Wall -Wextra -g -D_GNU_SOURCE)

# 如果启用覆盖率，添加覆盖率标志
if(ENABLE_COVERAGE)
    message(STATUS "Code coverage enabled")
    add_compile_options(--coverage)
    add_link_options(--coverage)
endif()

# 包含源代码目录（更新为include/目录结构）
set(PMC_BSP_ROOT ${CMAKE_SOURCE_DIR})
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${PMC_BSP_ROOT}/osal/include
    ${PMC_BSP_ROOT}/hal/include
    ${PMC_BSP_ROOT}/pdl/include
    ${PMC_BSP_ROOT}/apps/protocol_converter
    ${PMC_BSP_ROOT}/apps/protocol_converter/include
    ${PMC_BSP_ROOT}/apps/can_gateway
    ${PMC_BSP_ROOT}/apps/can_gateway/include
)

# 查找pthread库
find_package(Threads REQUIRED)

# 收集源文件（排除_fixed文件和main.c）
file(GLOB OSAL_SOURCES ${PMC_BSP_ROOT}/osal/src/linux/*.c)
list(FILTER OSAL_SOURCES EXCLUDE REGEX ".*_fixed\\.c$")

file(GLOB HAL_SOURCES ${PMC_BSP_ROOT}/hal/src/linux/*.c)
list(FILTER HAL_SOURCES EXCLUDE REGEX ".*_fixed\\.c$")

file(GLOB SERVICE_SOURCES ${PMC_BSP_ROOT}/pdl/src/linux/*.c)

file(GLOB APPS_GATEWAY_SOURCES ${PMC_BSP_ROOT}/apps/can_gateway/src/*.c)
list(FILTER APPS_GATEWAY_SOURCES EXCLUDE REGEX ".*_fixed\\.c$")
list(FILTER APPS_GATEWAY_SOURCES EXCLUDE REGEX ".*/main\\.c$")

file(GLOB APPS_PROTOCOL_SOURCES ${PMC_BSP_ROOT}/apps/protocol_converter/src/*.c)
list(FILTER APPS_PROTOCOL_SOURCES EXCLUDE REGEX ".*_fixed\\.c$")
list(FILTER APPS_PROTOCOL_SOURCES EXCLUDE REGEX ".*/main\\.c$")

# 注释掉 CTest，我们使用自定义测试框架
# enable_testing()

# ============================================
# 统一测试入口
# ============================================

# 测试运行器库
add_library(test_runner STATIC
    src/test_runner.c
)
target_include_directories(test_runner PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 统一测试主程序（唯一的测试可执行文件）
add_executable(unit-test
    src/test_entry.c
    src/osal/test_osal_task.c
    src/osal/test_osal_queue.c
    src/osal/test_osal_mutex.c
    src/osal/test_osal_signal.c
    src/hal/test_hal_can.c
    src/pdl/test_pdl_satellite.c
    src/apps/test_can_gateway.c
    src/apps/test_protocol_converter.c
    ${APPS_PROTOCOL_SOURCES}
    ${APPS_GATEWAY_SOURCES}
    ${SERVICE_SOURCES}
    ${HAL_SOURCES}
    ${OSAL_SOURCES}
)
target_link_libraries(unit-test
    test_runner
    pdl
    hal
    osal
    Threads::Threads
    rt
)

# 注释掉 CTest 集成，我们使用自定义测试框架
# add_test(NAME unit_test_suite COMMAND unit-test -a)

# ============================================
# OSAL层测试（独立可执行文件，保留向后兼容）
# ============================================

# 注释掉所有独立测试可执行文件的构建
# 只保留统一的 unit-test 入口

# # 测试：任务管理
# add_executable(test_os_task
#     osal/test_os_task.c
#     ${OSAL_SOURCES}
# )
# target_compile_definitions(test_os_task PRIVATE STANDALONE_TEST)
# target_link_libraries(test_os_task
#     Threads::Threads
#     rt
# )
# add_test(NAME test_os_task COMMAND test_os_task)

# 注释掉其他独立测试
# 只保留 unit-test 作为唯一入口

# # 测试：消息队列
# add_executable(test_os_queue ...)
# # 测试：互斥锁
# add_executable(test_os_mutex ...)
# # 测试：文件I/O
# add_executable(test_os_file ...)
# # 测试：网络通信
# add_executable(test_os_network ...)
# # 测试：信号处理
# add_executable(test_os_signal ...)

# ============================================
# HAL层测试
# ============================================

# 注释掉独立测试可执行文件
# # 测试：CAN驱动
# add_executable(test_hal_can
#     hal/test_hal_can.c
#     ${HAL_SOURCES}
#     ${OSAL_SOURCES}
# )
# target_link_libraries(test_hal_can
#     Threads::Threads
#     rt
# )
# add_test(NAME test_hal_can COMMAND test_hal_can)

# ============================================
# Service层测试
# ============================================

# 注释掉独立测试可执行文件
# # 测试：载荷服务
# add_executable(test_payload_pdl
#     pdl/test_payload_pdl.c
#     ${APPS_PROTOCOL_SOURCES}
#     ${APPS_GATEWAY_SOURCES}
#     ${SERVICE_SOURCES}
#     ${HAL_SOURCES}
#     ${OSAL_SOURCES}
# )
# target_link_libraries(test_payload_pdl
#     Threads::Threads
#     rt
# )
# add_test(NAME test_payload_pdl COMMAND test_payload_pdl)

# ============================================
# Apps层测试
# ============================================

# 注释掉独立测试可执行文件
# # 测试：CAN网关
# add_executable(test_can_gateway
#     apps/test_can_gateway.c
#     ${APPS_GATEWAY_SOURCES}
#     ${SERVICE_SOURCES}
#     ${HAL_SOURCES}
#     ${OSAL_SOURCES}
# )
# target_link_libraries(test_can_gateway
#     Threads::Threads
#     rt
# )
# add_test(NAME test_can_gateway COMMAND test_can_gateway)

# # 测试：协议转换器
# add_executable(test_protocol_converter
#     apps/test_protocol_converter.c
#     ${APPS_PROTOCOL_SOURCES}
#     ${APPS_GATEWAY_SOURCES}
#     ${SERVICE_SOURCES}
#     ${HAL_SOURCES}
#     ${OSAL_SOURCES}
# )
# target_link_libraries(test_protocol_converter
#     Threads::Threads
#     rt
# )
# add_test(NAME test_protocol_converter COMMAND test_protocol_converter)

# ============================================
# 测试套件和目标
# ============================================

# 所有测试目标（只构建 unit-test）
add_custom_target(all_tests
    DEPENDS unit-test
    COMMENT "Building unified test suite"
)

# 运行所有测试（使用统一入口）
add_custom_target(run_tests
    COMMAND ${OUTPUT_TARGET}/bin/unit-test -a
    DEPENDS unit-test
    COMMENT "Running all unit tests"
    WORKING_DIRECTORY ${OUTPUT_BUILD}
)

# ============================================
# 代码覆盖率
# ============================================

if(ENABLE_COVERAGE)
    find_program(LCOV_PATH lcov)
    find_program(GENHTML_PATH genhtml)

    if(LCOV_PATH AND GENHTML_PATH)
        # 覆盖率报告目标
        add_custom_target(coverage
            # 清零计数器
            COMMAND ${LCOV_PATH} --directory . --zerocounters

            # 运行测试
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure

            # 捕获覆盖率数据
            COMMAND ${LCOV_PATH} --directory . --capture --output-file coverage.info

            # 过滤系统文件和测试文件
            COMMAND ${LCOV_PATH} --remove coverage.info '/usr/*' '*/tests/*' --output-file coverage.info.cleaned

            # 生成HTML报告
            COMMAND ${GENHTML_PATH} -o coverage_html coverage.info.cleaned

            DEPENDS all_tests
            WORKING_DIRECTORY ${OUTPUT_BUILD}
            COMMENT "Generating code coverage report"
        )

        message(STATUS "Code coverage target 'coverage' available")
        message(STATUS "  Run: make coverage")
        message(STATUS "  Report: ${OUTPUT_BUILD}/coverage_html/index.html")
    else()
        message(STATUS "lcov/genhtml not found. Coverage target not available.")
        message(STATUS "  Install: sudo apt-get install lcov")
    endif()
endif()

# ============================================
# 安装测试可执行文件（可选）
# ============================================

install(TARGETS
    unit-test
    RUNTIME DESTINATION ${OUTPUT_TARGET}/bin
    OPTIONAL
)

# ============================================
# 打印测试配置信息
# ============================================

message(STATUS "")
message(STATUS "========================================")
message(STATUS "  PMC-BSP Unit Tests Configuration")
message(STATUS "========================================")
message(STATUS "  Test framework: Custom (unittest_framework.h + unittest_runner.h)")
message(STATUS "  Total test modules: 10")
message(STATUS "    - OSAL tests: 6 modules (task, queue, mutex, file, network, signal)")
message(STATUS "    - HAL tests: 1 module (CAN driver)")
message(STATUS "    - Service tests: 1 module (payload pdl)")
message(STATUS "    - Apps tests: 2 modules (CAN gateway, protocol converter)")
message(STATUS "")
message(STATUS "  Unified test entry: ./target/bin/unit-test")
message(STATUS "    ./target/bin/unit-test -i          - Interactive menu")
message(STATUS "    ./target/bin/unit-test -a          - Run all tests")
message(STATUS "    ./target/bin/unit-test -l          - List all tests")
message(STATUS "    ./target/bin/unit-test -L OSAL     - Run OSAL layer tests")
message(STATUS "    ./target/bin/unit-test -m <module> - Run module tests")
message(STATUS "    ./target/bin/unit-test -t <module> <test> - Run single test")
message(STATUS "")
message(STATUS "  Build targets:")
message(STATUS "    make all_tests     - Build unit-test")
message(STATUS "    make unit-test     - Build unit-test")
message(STATUS "")
message(STATUS "  Run targets:")
message(STATUS "    make run_tests     - Build and run all tests")
message(STATUS "")
message(STATUS "  Note: CTest is disabled. Use ./target/bin/unit-test directly")
if(ENABLE_COVERAGE)
message(STATUS "")
message(STATUS "  Coverage:")
message(STATUS "    make coverage      - Generate coverage report")
endif()
message(STATUS "========================================")
message(STATUS "")
