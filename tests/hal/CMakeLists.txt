# HAL Module Tests
# Tests for HAL layer - automatically registered with tests_framework

if(NOT BUILD_TESTING)
    return()
endif()

# Create HAL test library
add_library(hal_tests STATIC
    test_can.c
)

# Include directories
target_include_directories(hal_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hal/include
)

# Link dependencies
target_link_libraries(hal_tests
    PUBLIC pmc::tests_framework
    PRIVATE pmc::hal pmc::osal
)

# Export for test runner
add_library(pmc::hal_tests ALIAS hal_tests)
