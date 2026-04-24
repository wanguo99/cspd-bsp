/************************************************************************
 * 协议转换测试模块包装
 * 用于统一测试入口
 ************************************************************************/

#include "unittest_runner.h"
#include "unittest_framework.h"
#include "protocol_converter.h"
#include "osal.h"

void test_ProtocolConverter_Init_Success(void)
{
    OS_API_Init();

    int32 ret = Protocol_Converter_Init();

    if (ret != OS_SUCCESS) {
        TEST_WARNING("Protocol Converter init failed, hardware not available");
    }

    /* 无论成功或失败都进行断言，让测试真实反映结果 */
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    OS_API_Teardown();
}

void test_ProtocolConverter_GetStats(void)
{
    OS_API_Init();

    int32 ret = Protocol_Converter_Init();

    if (ret != OS_SUCCESS) {
        TEST_WARNING("Protocol Converter init failed, hardware not available");
    }

    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    uint32 cmd_count, success, fail, timeout;
    Protocol_Converter_GetStats(&cmd_count, &success, &fail, &timeout);
    TEST_ASSERT_EQUAL(0, cmd_count);

    OS_API_Teardown();
}

/* 注册测试模块 */
TEST_MODULE_BEGIN(test_protocol_converter)
    TEST_CASE(test_ProtocolConverter_Init_Success)
    TEST_CASE(test_ProtocolConverter_GetStats)
TEST_MODULE_END(test_protocol_converter)
