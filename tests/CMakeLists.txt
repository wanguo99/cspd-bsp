# Unified Test Framework
# All test modules in a single CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

if(NOT BUILD_TESTING)
    return()
endif()

# ============================================================================
# Test Framework Core (tests_core library)
# ============================================================================
add_library(tests_core STATIC
    core/test_runner.c
    core/test_registry.c
    core/test_menu.c
)

target_include_directories(tests_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(tests_core PUBLIC
    pmc::osal_public_api
)

target_link_libraries(tests_core PRIVATE
    pmc::osal
)

add_library(pmc::tests_core ALIAS tests_core)

# ============================================================================
# OSAL Tests
# ============================================================================
add_library(osal_tests STATIC
    osal/test_osal_task.c
    osal/test_osal_queue.c
    osal/test_osal_mutex.c
    osal/test_osal_signal.c
    osal/test_osal_atomic.c
    osal/test_osal_string.c
    osal/test_osal_file.c
    osal/test_osal_time.c
    osal/test_osal_heap.c
    osal/test_osal_log.c
)

target_include_directories(osal_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../osal/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(osal_tests
    PUBLIC pmc::osal_public_api
    PRIVATE pmc::osal
)

add_library(pmc::osal_tests ALIAS osal_tests)

# ============================================================================
# HAL Tests
# ============================================================================
add_library(hal_tests STATIC
    hal/test_hal_can.c
    hal/test_hal_serial.c
)

target_include_directories(hal_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../hal/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(hal_tests
    PUBLIC pmc::osal_public_api
    PRIVATE pmc::hal pmc::osal
)

add_library(pmc::hal_tests ALIAS hal_tests)

# ============================================================================
# PCL Tests
# ============================================================================
add_library(pcl_tests STATIC
    pcl/test_pcl_api.c
)

target_include_directories(pcl_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../pcl/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../pcl/include/api
    ${CMAKE_CURRENT_SOURCE_DIR}/../pcl/include/internal
    ${CMAKE_CURRENT_SOURCE_DIR}/../pcl/include/peripheral
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(pcl_tests
    PUBLIC pmc::osal_public_api
    PRIVATE pmc::pcl pmc::osal
)

add_library(pmc::pcl_tests ALIAS pcl_tests)

# ============================================================================
# PDL Tests
# ============================================================================
add_library(pdl_tests STATIC
    pdl/test_pdl_satellite.c
    pdl/test_pdl_mcu.c
    pdl/test_pdl_bmc.c
)

target_include_directories(pdl_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../pdl/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(pdl_tests
    PUBLIC pmc::osal_public_api
    PRIVATE pmc::pdl pmc::osal
)

add_library(pmc::pdl_tests ALIAS pdl_tests)

# ============================================================================
# Unified Test Runner Executable
# ============================================================================
add_executable(unit-test
    core/main.c
    core/test_runner.c
    core/test_registry.c
    core/test_menu.c
)

target_include_directories(unit-test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link all test libraries
target_link_libraries(unit-test
    pmc::osal_public_api
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

# ============================================================================
# Custom Test Targets
# ============================================================================
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

# ============================================================================
# Installation
# ============================================================================
install(TARGETS tests_core
    EXPORT tests_core-targets
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include/tests_core
    FILES_MATCHING PATTERN "*.h"
)
