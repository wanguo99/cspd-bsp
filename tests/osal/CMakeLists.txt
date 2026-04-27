# OSAL Module Tests
# Tests for OSAL layer - automatically registered with tests_core

if(NOT BUILD_TESTING)
    return()
endif()

# Create OSAL test library
add_library(osal_tests STATIC
    test_task.c
    test_queue.c
    test_mutex.c
    test_signal.c
)

# Include directories
target_include_directories(osal_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../osal/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/include
)

# Link dependencies
target_link_libraries(osal_tests
    PUBLIC pmc::osal_public_api
    PRIVATE pmc::osal
)

# Export for test runner
add_library(pmc::osal_tests ALIAS osal_tests)
