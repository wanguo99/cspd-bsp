# PDL Module Tests
# Tests for PDL layer - automatically registered with libtest

if(NOT BUILD_TESTING)
    return()
endif()

# Create PDL test library (placeholder - no tests yet)
add_library(pdl_tests STATIC
    # Add test files here when available
    # test_satellite.c
    # test_payload.c
)

# Placeholder: Create empty source file to satisfy CMake
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/pdl_tests_placeholder.c
    "/* Placeholder for PDL tests - add actual tests later */\n"
)

target_sources(pdl_tests PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/pdl_tests_placeholder.c
)

# Include directories
target_include_directories(pdl_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# Link dependencies
target_link_libraries(pdl_tests
    PUBLIC pmc::libtest
    PRIVATE pmc::pdl pmc::hal pmc::osal
)

# Export for test runner
add_library(pmc::pdl_tests ALIAS pdl_tests)
