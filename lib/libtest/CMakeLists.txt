# Reusable Test Library (libtest)
# Only depends on OSAL for maximum portability

cmake_minimum_required(VERSION 3.10)

add_library(libtest STATIC
    src/test_runner.c
    src/test_registry.c
    src/test_menu.c
)

target_include_directories(libtest PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Public dependency: OSAL public API (headers only)
target_link_libraries(libtest PUBLIC
    pmc::osal_public_api
)

# Private dependency: OSAL implementation
target_link_libraries(libtest PRIVATE
    pmc::osal
)

# Export for use by other modules
add_library(pmc::libtest ALIAS libtest)

# Installation
install(TARGETS libtest
    EXPORT libtest-targets
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include/libtest
    FILES_MATCHING PATTERN "*.h"
)
