# Unified Test Runner
# Aggregates all module tests into a single executable

if(NOT BUILD_TESTING)
    return()
endif()

# 目录变量
set(SRC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(unit-test
    ${SRC}/main.c
)

# Link all test libraries
target_link_libraries(unit-test
    pmc::libutest
    -Wl,--whole-archive
    pmc::osal_tests
    pmc::hal_tests
    pmc::pcl_tests
    pmc::pdl_tests
    -Wl,--no-whole-archive
    pmc::osal
    pmc::hal
    pmc::pcl
    pmc::pdl
    Threads::Threads
    rt
)

# Set output directory
set_target_properties(unit-test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_TARGET}/bin
)

# Custom test targets
add_custom_target(run_tests
    COMMAND ${OUTPUT_TARGET}/bin/unit-test --all
    DEPENDS unit-test
    COMMENT "Running all unit tests"
)

add_custom_target(run_tests_interactive
    COMMAND ${OUTPUT_TARGET}/bin/unit-test --interactive
    DEPENDS unit-test
    COMMENT "Running tests in interactive mode"
)
