cmake_minimum_required(VERSION 3.10)

# 测试项目配置
project(CSPD_BSP_Tests C)

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

# 包含源代码目录
set(CSPD_BSP_ROOT ${CMAKE_SOURCE_DIR})
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CSPD_BSP_ROOT}/osal/inc
    ${CSPD_BSP_ROOT}/hal/inc
    ${CSPD_BSP_ROOT}/service/inc
    ${CSPD_BSP_ROOT}/apps/protocol_converter
    ${CSPD_BSP_ROOT}/apps/can_gateway
    ${CSPD_BSP_ROOT}/config
)

# 查找pthread库
find_package(Threads REQUIRED)

# 收集源文件（排除_fixed文件）
file(GLOB OSAL_SOURCES ${CSPD_BSP_ROOT}/osal/linux/*.c)
list(FILTER OSAL_SOURCES EXCLUDE REGEX ".*_fixed\\.c$")

file(GLOB HAL_SOURCES ${CSPD_BSP_ROOT}/hal/linux/*.c)
list(FILTER HAL_SOURCES EXCLUDE REGEX ".*_fixed\\.c$")

file(GLOB SERVICE_SOURCES ${CSPD_BSP_ROOT}/service/linux/*.c)

file(GLOB APPS_GATEWAY_SOURCES ${CSPD_BSP_ROOT}/apps/can_gateway/*.c)
list(FILTER APPS_GATEWAY_SOURCES EXCLUDE REGEX ".*_fixed\\.c$")

file(GLOB APPS_PROTOCOL_SOURCES ${CSPD_BSP_ROOT}/apps/protocol_converter/*.c)
list(FILTER APPS_PROTOCOL_SOURCES EXCLUDE REGEX ".*_fixed\\.c$")

# 启用测试
enable_testing()

# ============================================
# 统一测试入口
# ============================================

# 测试运行器库
add_library(test_runner STATIC
    test_runner.c
)
target_include_directories(test_runner PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# 统一测试主程序
add_executable(test_main
    osal/test_main.c
    osal/test_os_task.c
    osal/test_os_queue.c
    osal/test_os_mutex.c
    osal/test_os_file.c
    osal/test_os_network.c
    osal/test_os_signal.c
    ${OSAL_SOURCES}
)
target_link_libraries(test_main
    test_runner
    Threads::Threads
    rt
)
add_test(NAME test_main_all COMMAND test_main -a)

# ============================================
# OSAL层测试（独立可执行文件，保留向后兼容）
# ============================================

# 测试：任务管理
add_executable(test_os_task
    osal/test_os_task.c
    ${OSAL_SOURCES}
)
target_compile_definitions(test_os_task PRIVATE STANDALONE_TEST)
target_link_libraries(test_os_task
    Threads::Threads
    rt
)
add_test(NAME test_os_task COMMAND test_os_task)

# 测试：消息队列
add_executable(test_os_queue
    osal/test_os_queue.c
    ${OSAL_SOURCES}
)
target_compile_definitions(test_os_queue PRIVATE STANDALONE_TEST)
target_link_libraries(test_os_queue
    Threads::Threads
    rt
)
add_test(NAME test_os_queue COMMAND test_os_queue)

# 测试：互斥锁
add_executable(test_os_mutex
    osal/test_os_mutex.c
    ${OSAL_SOURCES}
)
target_compile_definitions(test_os_mutex PRIVATE STANDALONE_TEST)
target_link_libraries(test_os_mutex
    Threads::Threads
    rt
)
add_test(NAME test_os_mutex COMMAND test_os_mutex)

# 测试：文件I/O
add_executable(test_os_file
    osal/test_os_file.c
    ${OSAL_SOURCES}
)
target_compile_definitions(test_os_file PRIVATE STANDALONE_TEST)
target_link_libraries(test_os_file
    Threads::Threads
    rt
)
add_test(NAME test_os_file COMMAND test_os_file)

# 测试：网络通信
add_executable(test_os_network
    osal/test_os_network.c
    ${OSAL_SOURCES}
)
target_compile_definitions(test_os_network PRIVATE STANDALONE_TEST)
target_link_libraries(test_os_network
    Threads::Threads
    rt
)
add_test(NAME test_os_network COMMAND test_os_network)

# 测试：信号处理
add_executable(test_os_signal
    osal/test_os_signal.c
    ${OSAL_SOURCES}
)
target_compile_definitions(test_os_signal PRIVATE STANDALONE_TEST)
target_link_libraries(test_os_signal
    Threads::Threads
    rt
)
add_test(NAME test_os_signal COMMAND test_os_signal)

# ============================================
# HAL层测试
# ============================================

# 测试：CAN驱动
add_executable(test_hal_can
    hal/test_hal_can.c
    ${HAL_SOURCES}
    ${OSAL_SOURCES}
)
target_link_libraries(test_hal_can
    Threads::Threads
    rt
)
add_test(NAME test_hal_can COMMAND test_hal_can)

# ============================================
# Service层测试
# ============================================

# 测试：载荷服务
add_executable(test_payload_service
    service/test_payload_service.c
    ${APPS_PROTOCOL_SOURCES}
    ${APPS_GATEWAY_SOURCES}
    ${SERVICE_SOURCES}
    ${HAL_SOURCES}
    ${OSAL_SOURCES}
)
target_link_libraries(test_payload_service
    Threads::Threads
    rt
)
add_test(NAME test_payload_service COMMAND test_payload_service)

# ============================================
# Apps层测试
# ============================================

# 测试：CAN网关
add_executable(test_can_gateway
    apps/test_can_gateway.c
    ${APPS_GATEWAY_SOURCES}
    ${SERVICE_SOURCES}
    ${HAL_SOURCES}
    ${OSAL_SOURCES}
)
target_link_libraries(test_can_gateway
    Threads::Threads
    rt
)
add_test(NAME test_can_gateway COMMAND test_can_gateway)

# 测试：协议转换器
add_executable(test_protocol_converter
    apps/test_protocol_converter.c
    ${APPS_PROTOCOL_SOURCES}
    ${APPS_GATEWAY_SOURCES}
    ${SERVICE_SOURCES}
    ${HAL_SOURCES}
    ${OSAL_SOURCES}
)
target_link_libraries(test_protocol_converter
    Threads::Threads
    rt
)
add_test(NAME test_protocol_converter COMMAND test_protocol_converter)

# ============================================
# 测试套件和目标
# ============================================

# 创建测试套件分组
add_custom_target(osal_tests
    DEPENDS test_os_task test_os_queue test_os_mutex test_os_file test_os_network test_os_signal
    COMMENT "Building OSAL layer tests"
)

add_custom_target(hal_tests
    DEPENDS test_hal_can
    COMMENT "Building HAL layer tests"
)

add_custom_target(service_tests
    DEPENDS test_payload_service
    COMMENT "Building Service layer tests"
)

add_custom_target(apps_tests
    DEPENDS test_can_gateway test_protocol_converter
    COMMENT "Building Apps layer tests"
)

# 所有测试目标
add_custom_target(all_tests
    DEPENDS osal_tests hal_tests service_tests apps_tests
    COMMENT "Building all tests"
)

# 运行所有测试
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS all_tests
    COMMENT "Running all unit tests"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
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
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating code coverage report"
        )

        message(STATUS "Code coverage target 'coverage' available")
        message(STATUS "  Run: make coverage")
        message(STATUS "  Report: ${CMAKE_BINARY_DIR}/coverage_html/index.html")
    else()
        message(STATUS "lcov/genhtml not found. Coverage target not available.")
        message(STATUS "  Install: sudo apt-get install lcov")
    endif()
endif()

# ============================================
# 安装测试可执行文件（可选）
# ============================================

install(TARGETS
    test_os_task test_os_queue test_os_mutex test_os_file test_os_network test_os_signal
    test_hal_can
    test_payload_service
    test_can_gateway test_protocol_converter
    RUNTIME DESTINATION tests
    OPTIONAL
)

# ============================================
# 打印测试配置信息
# ============================================

message(STATUS "")
message(STATUS "========================================")
message(STATUS "  CSPD-BSP Unit Tests Configuration")
message(STATUS "========================================")
message(STATUS "  Test framework: Custom (test_framework.h + test_runner.h)")
message(STATUS "  Total tests: 67")
message(STATUS "    - OSAL tests: 67 (task, queue, mutex, file, network, signal)")
message(STATUS "    - HAL tests: 10 (CAN driver)")
message(STATUS "    - Service tests: 10 (payload service)")
message(STATUS "    - Apps tests: 18 (CAN gateway, protocol converter)")
message(STATUS "")
message(STATUS "  Unified test entry:")
message(STATUS "    ./test_main -i          - Interactive menu")
message(STATUS "    ./test_main -a          - Run all tests")
message(STATUS "    ./test_main -l          - List all modules")
message(STATUS "    ./test_main -m <module> - Run module tests")
message(STATUS "    ./test_main -t <module> <test> - Run single test")
message(STATUS "")
message(STATUS "  Build targets:")
message(STATUS "    make all_tests     - Build all tests")
message(STATUS "    make test_main     - Build unified test entry")
message(STATUS "    make osal_tests    - Build OSAL tests")
message(STATUS "    make hal_tests     - Build HAL tests")
message(STATUS "    make service_tests - Build Service tests")
message(STATUS "    make apps_tests    - Build Apps tests")
message(STATUS "")
message(STATUS "  Run targets:")
message(STATUS "    make run_tests     - Build and run all tests")
message(STATUS "    ctest              - Run tests with CTest")
message(STATUS "    ctest -V           - Run tests with verbose output")
if(ENABLE_COVERAGE)
message(STATUS "")
message(STATUS "  Coverage:")
message(STATUS "    make coverage      - Generate coverage report")
endif()
message(STATUS "========================================")
message(STATUS "")
