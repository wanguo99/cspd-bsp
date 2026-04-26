# 测试框架
# 参考cFS ut_assert设计

# ============================================================================
# 测试运行器库
# ============================================================================
add_library(test_runner STATIC
    src/test_runner.c
)

target_include_directories(test_runner PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 依赖OSAL接口库（需要osal_types.h等头文件）
target_link_libraries(test_runner PUBLIC
    pmc::osal_public_api
)

# ============================================================================
# 统一测试主程序
# ============================================================================
add_executable(unit-test
    src/test_entry.c
    src/osal/test_osal_task.c
    src/osal/test_osal_queue.c
    src/osal/test_osal_mutex.c
    src/osal/test_osal_signal.c
    src/hal/test_hal_can.c
)

# 链接依赖（使用命名空间别名）
target_link_libraries(unit-test
    test_runner
    pmc::pdl
    pmc::hal
    pmc::osal
    Threads::Threads
    rt
)

# 包含测试框架头文件
target_include_directories(unit-test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 添加编译定义（usleep需要）
target_compile_definitions(unit-test PRIVATE
    _DEFAULT_SOURCE
    _BSD_SOURCE
)

# 设置输出目录
set_target_properties(unit-test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_TARGET}/bin
)

# ============================================================================
# 代码覆盖率（可选）
# ============================================================================
option(ENABLE_COVERAGE "Enable code coverage" OFF)

if(ENABLE_COVERAGE)
    message(STATUS "Code coverage enabled")
    target_compile_options(unit-test PRIVATE --coverage)
    target_link_options(unit-test PRIVATE --coverage)

    find_program(LCOV_PATH lcov)
    find_program(GENHTML_PATH genhtml)

    if(LCOV_PATH AND GENHTML_PATH)
        # 覆盖率报告目标
        add_custom_target(coverage
            # 清零计数器
            COMMAND ${LCOV_PATH} --directory . --zerocounters

            # 运行测试
            COMMAND ${OUTPUT_TARGET}/bin/unit-test -a

            # 捕获覆盖率数据
            COMMAND ${LCOV_PATH} --directory . --capture --output-file coverage.info

            # 过滤系统文件和测试文件
            COMMAND ${LCOV_PATH} --remove coverage.info '/usr/*' '*/tests/*' --output-file coverage.info.cleaned

            # 生成HTML报告
            COMMAND ${GENHTML_PATH} -o coverage_html coverage.info.cleaned

            DEPENDS unit-test
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

# ============================================================================
# 测试目标
# ============================================================================

# 运行所有测试
add_custom_target(run_tests
    COMMAND ${OUTPUT_TARGET}/bin/unit-test -a
    DEPENDS unit-test
    COMMENT "Running all unit tests"
    WORKING_DIRECTORY ${OUTPUT_BUILD}
)

# ============================================================================
# 安装规则
# ============================================================================
install(TARGETS unit-test
    RUNTIME DESTINATION bin
)

# ============================================================================
# 打印测试配置信息
# ============================================================================
message(STATUS "")
message(STATUS "========================================")
message(STATUS "  PMC-BSP Unit Tests Configuration")
message(STATUS "========================================")
message(STATUS "  Test framework: Custom (test_framework.h + test_runner.h)")
message(STATUS "  Total test modules: 5")
message(STATUS "    - OSAL tests: 4 modules (task, queue, mutex, signal)")
message(STATUS "    - HAL tests: 1 module (CAN driver)")
message(STATUS "")
message(STATUS "  Unified test entry: ./output/target/bin/unit-test")
message(STATUS "    ./output/target/bin/unit-test -i          - Interactive menu")
message(STATUS "    ./output/target/bin/unit-test -a          - Run all tests")
message(STATUS "    ./output/target/bin/unit-test -l          - List all tests")
message(STATUS "    ./output/target/bin/unit-test -L OSAL     - Run OSAL layer tests")
message(STATUS "    ./output/target/bin/unit-test -m <module> - Run module tests")
message(STATUS "    ./output/target/bin/unit-test -t <module> <test> - Run single test")
message(STATUS "")
message(STATUS "  Build targets:")
message(STATUS "    make unit-test     - Build unit-test")
message(STATUS "")
message(STATUS "  Run targets:")
message(STATUS "    make run_tests     - Build and run all tests")
message(STATUS "")
if(ENABLE_COVERAGE)
message(STATUS "  Coverage:")
message(STATUS "    make coverage      - Generate coverage report")
endif()
message(STATUS "========================================")
message(STATUS "")
