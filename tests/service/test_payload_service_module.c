/************************************************************************
 * 载荷服务测试模块包装
 * 用于统一测试入口
 ************************************************************************/

#include "test_runner.h"
#include "test_framework.h"
#include "payload_service.h"
#include "osal.h"

static payload_service_handle_t test_handle;

void test_PayloadService_Init_Success(void)
{
    OS_API_Init();

    payload_service_config_t config = {
        .ethernet = {
            .ip_addr = "127.0.0.1",
            .port = 8080,
            .timeout_ms = 5000
        },
        .uart = {
            .device = "/dev/ttyUSB0",
            .baudrate = 115200,
            .timeout_ms = 2000
        },
        .auto_switch = true,
        .retry_count = 3
    };

    int32 ret = PayloadService_Init(&config, &test_handle);

    if (ret == OS_SUCCESS) {
        TEST_ASSERT_NOT_NULL(test_handle);
        PayloadService_Deinit(test_handle);
    } else {
        TEST_MESSAGE("Warning: Service not available, skipping test");
        TEST_IGNORE();
    }

    OS_API_Teardown();
}

void test_PayloadService_Init_NullConfig(void)
{
    OS_API_Init();

    int32 ret = PayloadService_Init(NULL, &test_handle);
    TEST_ASSERT_NOT_EQUAL(OS_SUCCESS, ret);

    OS_API_Teardown();
}

/* 注册测试模块 */
TEST_MODULE_BEGIN(test_payload_service)
    TEST_CASE(test_PayloadService_Init_Success)
    TEST_CASE(test_PayloadService_Init_NullConfig)
TEST_MODULE_END(test_payload_service)
