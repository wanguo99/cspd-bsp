# 载荷服务测试
add_executable(test_payload_service test_payload_service.c)
target_include_directories(test_payload_service PRIVATE
    ${CMAKE_SOURCE_DIR}/apps/protocol_converter
    ${CMAKE_SOURCE_DIR}/osal/inc
    ${CMAKE_SOURCE_DIR}/hal/inc
    ${CMAKE_SOURCE_DIR}/service/inc
    ${CMAKE_SOURCE_DIR}/common/inc
)
target_link_libraries(test_payload_service PRIVATE osal hal service pthread)
add_test(NAME test_payload_service COMMAND test_payload_service)
