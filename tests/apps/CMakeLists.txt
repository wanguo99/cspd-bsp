# CAN网关测试
add_executable(test_can_gateway test_can_gateway.c)
target_include_directories(test_can_gateway PRIVATE
    ${CMAKE_SOURCE_DIR}/apps/can_gateway
    ${CMAKE_SOURCE_DIR}/apps/protocol_converter
    ${CMAKE_SOURCE_DIR}/osal/inc
    ${CMAKE_SOURCE_DIR}/hal/inc
    ${CMAKE_SOURCE_DIR}/service/inc
    ${CMAKE_SOURCE_DIR}/common/inc
)
target_link_libraries(test_can_gateway PRIVATE osal hal service pthread)
add_test(NAME test_can_gateway COMMAND test_can_gateway)

# 协议转换测试
add_executable(test_protocol_converter test_protocol_converter.c)
target_include_directories(test_protocol_converter PRIVATE
    ${CMAKE_SOURCE_DIR}/apps/protocol_converter
    ${CMAKE_SOURCE_DIR}/osal/inc
    ${CMAKE_SOURCE_DIR}/hal/inc
    ${CMAKE_SOURCE_DIR}/service/inc
    ${CMAKE_SOURCE_DIR}/common/inc
)
target_link_libraries(test_protocol_converter PRIVATE osal hal service pthread)
add_test(NAME test_protocol_converter COMMAND test_protocol_converter)
