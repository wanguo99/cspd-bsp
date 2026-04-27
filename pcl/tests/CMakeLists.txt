# PCL Module Tests
# Tests for PCL layer - automatically registered with tests_framework

if(NOT BUILD_TESTING)
    return()
endif()

# Create PCL test library
add_library(pcl_tests STATIC
    test_pcl_api.c
)

# Include directories
target_include_directories(pcl_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/api
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/internal
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/peripheral
)

# Link dependencies
target_link_libraries(pcl_tests
    PUBLIC pmc::tests_framework
    PRIVATE pmc::pcl pmc::osal
)

# Export for test runner
add_library(pmc::pcl_tests ALIAS pcl_tests)
