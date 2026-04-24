# Apps (应用层) 模块

# CAN网关库（不含main）
add_library(can_gateway_lib STATIC
    can_gateway/src/can_gateway.c
)

target_include_directories(can_gateway_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/can_gateway
    ${CMAKE_CURRENT_SOURCE_DIR}/can_gateway/include
)

# 私有包含下层模块
target_include_directories(can_gateway_lib PRIVATE
    ${CMAKE_SOURCE_DIR}/osal/include
    ${CMAKE_SOURCE_DIR}/hal/include
)

target_link_libraries(can_gateway_lib
    osal
    hal
)

target_compile_options(can_gateway_lib PRIVATE -Wall -Wextra)

# CAN网关应用（含main）
add_executable(can_gateway
    can_gateway/src/can_gateway.c
    can_gateway/src/main.c
)

target_include_directories(can_gateway PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/can_gateway
    ${CMAKE_CURRENT_SOURCE_DIR}/can_gateway/include
    ${CMAKE_SOURCE_DIR}/osal/include
    ${CMAKE_SOURCE_DIR}/hal/include
)

target_link_libraries(can_gateway
    osal
    hal
    Threads::Threads
    rt
)

target_compile_options(can_gateway PRIVATE -Wall -Wextra)

# 协议转换库（不含main）
add_library(protocol_converter_lib STATIC
    protocol_converter/src/protocol_converter.c
    protocol_converter/src/payload_pdl.c
)

target_include_directories(protocol_converter_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/protocol_converter
    ${CMAKE_CURRENT_SOURCE_DIR}/protocol_converter/include
    ${CMAKE_CURRENT_SOURCE_DIR}/can_gateway
    ${CMAKE_CURRENT_SOURCE_DIR}/can_gateway/include
)

# 私有包含下层模块
target_include_directories(protocol_converter_lib PRIVATE
    ${CMAKE_SOURCE_DIR}/osal/include
    ${CMAKE_SOURCE_DIR}/hal/include
    ${CMAKE_SOURCE_DIR}/pdl/include
)

target_link_libraries(protocol_converter_lib
    osal
    hal
    pdl
    can_gateway_lib
)

target_compile_options(protocol_converter_lib PRIVATE -Wall -Wextra)

# 协议转换应用（含main）
add_executable(protocol_converter
    protocol_converter/src/protocol_converter.c
    protocol_converter/src/payload_pdl.c
    protocol_converter/src/main.c
)

target_include_directories(protocol_converter PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/protocol_converter
    ${CMAKE_CURRENT_SOURCE_DIR}/protocol_converter/include
    ${CMAKE_CURRENT_SOURCE_DIR}/can_gateway
    ${CMAKE_CURRENT_SOURCE_DIR}/can_gateway/include
    ${CMAKE_SOURCE_DIR}/osal/include
    ${CMAKE_SOURCE_DIR}/hal/include
    ${CMAKE_SOURCE_DIR}/pdl/include
)

target_link_libraries(protocol_converter
    osal
    hal
    pdl
    can_gateway_lib
    Threads::Threads
    rt
)

target_compile_options(protocol_converter PRIVATE -Wall -Wextra)

# 安装规则 - 安装到target/bin
install(TARGETS can_gateway protocol_converter
    RUNTIME DESTINATION ${OUTPUT_TARGET}/bin
)

message(STATUS "Apps module configured: can_gateway, protocol_converter")
