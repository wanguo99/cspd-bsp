# PDL Module Tests
# Tests for PDL layer - automatically registered with tests_framework

if(NOT BUILD_TESTING)
    return()
endif()

# Create PDL test library
# Placeholder: Create empty source file to satisfy CMake
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/pdl_tests_placeholder.c
    "/* Placeholder for PDL tests - add actual tests later */\n"
)

add_library(pdl_tests STATIC
    ${CMAKE_CURRENT_BINARY_DIR}/pdl_tests_placeholder.c
    # Add test files here when available
    # test_satellite.c
    # test_payload.c
)

# Include directories
target_include_directories(pdl_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# Link dependencies
target_link_libraries(pdl_tests
    PUBLIC pmc::tests_framework
    PRIVATE pmc::pdl pmc::hal pmc::osal
)

# Export for test runner
add_library(pmc::pdl_tests ALIAS pdl_tests)
