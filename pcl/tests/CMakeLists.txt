# PCL单元测试

# 测试可执行文件
add_executable(test_pcl
    main.c
    test_pcl_api.c
)

# 包含目录
target_include_directories(test_pcl PRIVATE
    ${CMAKE_SOURCE_DIR}/pcl/include
    ${CMAKE_SOURCE_DIR}/pcl/include/api
    ${CMAKE_SOURCE_DIR}/pcl/include/internal
    ${CMAKE_SOURCE_DIR}/pcl/include/peripheral
    ${CMAKE_SOURCE_DIR}/osal/include
    ${CMAKE_SOURCE_DIR}/libutest/include
)

# 链接库
target_link_libraries(test_pcl
    pmc::pcl
    pmc::osal
    pmc::libutest
    pthread
)

# 安装到bin目录
install(TARGETS test_pcl
    RUNTIME DESTINATION bin
)
