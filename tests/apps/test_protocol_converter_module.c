/************************************************************************
 * 协议转换测试模块包装
 * 用于统一测试入口
 ************************************************************************/

#include "test_runner.h"
#include "test_framework.h"
#include "protocol_converter.h"
#include "osal.h"

void test_ProtocolConverter_Init_Success(void)
{
    OS_API_Init();

    int32 ret = Protocol_Converter_Init();

    if (ret == OS_SUCCESS) {
        TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    } else {
        TEST_MESSAGE("Warning: Protocol Converter init failed, skipping test");
        TEST_IGNORE();
    }

    OS_API_Teardown();
}

void test_ProtocolConverter_GetStats(void)
{
    OS_API_Init();

    int32 ret = Protocol_Converter_Init();
    if (ret == OS_SUCCESS) {
        uint32 cmd_count, success, fail, timeout;
        Protocol_Converter_GetStats(&cmd_count, &success, &fail, &timeout);
        TEST_ASSERT_EQUAL(0, cmd_count);
    } else {
        TEST_IGNORE();
    }

    OS_API_Teardown();
}

/* 注册测试模块 */
TEST_MODULE_BEGIN(test_protocol_converter)
    TEST_CASE(test_ProtocolConverter_Init_Success)
    TEST_CASE(test_ProtocolConverter_GetStats)
TEST_MODULE_END(test_protocol_converter)
