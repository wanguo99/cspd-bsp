# Apps (应用层) 模块

# CAN网关应用
set(CAN_GATEWAY_SOURCES
    can_gateway/can_gateway.c
)

# 协议转换应用
set(PROTOCOL_CONVERTER_SOURCES
    protocol_converter/protocol_converter.c
    protocol_converter/payload_service.c
)

# 合并所有应用源文件
set(APPS_SOURCES
    ${CAN_GATEWAY_SOURCES}
    ${PROTOCOL_CONVERTER_SOURCES}
)

# 创建Apps静态库
add_library(apps STATIC ${APPS_SOURCES})

# 包含目录
target_include_directories(apps PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/can_gateway
    ${CMAKE_CURRENT_SOURCE_DIR}/protocol_converter
)

# 依赖OSAL、HAL和Service
target_link_libraries(apps
    osal
    hal
    service
)

# 设置编译选项
target_compile_options(apps PRIVATE
    -Wall
    -Wextra
)

message(STATUS "Apps module configured")
