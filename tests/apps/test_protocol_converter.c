/************************************************************************
 * 协议转换单元测试
 ************************************************************************/

#include "../test_framework.h"
#include "protocol_converter.h"
#include "payload_service.h"
#include "osal.h"

/* 测试前准备 */
void setUp(void)
{
    /* 每个测试前初始化 */
}

/* 测试后清理 */
void tearDown(void)
{
    /* 每个测试后清理 */
}

/* 测试用例1: 初始化成功 */
void test_ProtocolConverter_Init_Success(void)
{
    int32 ret = Protocol_Converter_Init();
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
}

/* 测试用例2: 获取统计信息 */
void test_ProtocolConverter_GetStats(void)
{
    Protocol_Converter_Init();

    uint32 cmd_count = 0, success_count = 0, fail_count = 0, timeout_count = 0;
    Protocol_Converter_GetStats(&cmd_count, &success_count, &fail_count, &timeout_count);

    /* 初始化后计数应该为0 */
    TEST_ASSERT_EQUAL(0, cmd_count);
    TEST_ASSERT_EQUAL(0, success_count);
    TEST_ASSERT_EQUAL(0, fail_count);
    TEST_ASSERT_EQUAL(0, timeout_count);
}

/* 测试用例3: 切换到以太网通道 */
void test_ProtocolConverter_SwitchChannel_Ethernet(void)
{
    Protocol_Converter_Init();

    int32 ret = Protocol_Converter_SwitchChannel(PAYLOAD_CHANNEL_ETHERNET);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    payload_channel_t channel = Protocol_Converter_GetChannel();
    TEST_ASSERT_EQUAL(PAYLOAD_CHANNEL_ETHERNET, channel);
}

/* 测试用例4: 切换到UART通道 */
void test_ProtocolConverter_SwitchChannel_UART(void)
{
    Protocol_Converter_Init();

    int32 ret = Protocol_Converter_SwitchChannel(PAYLOAD_CHANNEL_UART);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    payload_channel_t channel = Protocol_Converter_GetChannel();
    TEST_ASSERT_EQUAL(PAYLOAD_CHANNEL_UART, channel);
}

/* 测试用例5: 获取当前通道 */
void test_ProtocolConverter_GetChannel(void)
{
    Protocol_Converter_Init();

    payload_channel_t channel = Protocol_Converter_GetChannel();
    /* 默认应该是以太网通道 */
    TEST_ASSERT_EQUAL(PAYLOAD_CHANNEL_ETHERNET, channel);
}

/* 测试用例6: 空指针保护 */
void test_ProtocolConverter_GetStats_NullPointer(void)
{
    Protocol_Converter_Init();

    /* 传入NULL指针不应该崩溃 */
    Protocol_Converter_GetStats(NULL, NULL, NULL, NULL);

    /* 如果没有崩溃，测试通过 */
    TEST_ASSERT_TRUE(1);
}

/* 测试用例7: 多次切换通道 */
void test_ProtocolConverter_SwitchChannel_Multiple(void)
{
    Protocol_Converter_Init();

    /* 切换到UART通道 */
    Protocol_Converter_SwitchChannel(PAYLOAD_CHANNEL_UART);
    TEST_ASSERT_EQUAL(PAYLOAD_CHANNEL_UART, Protocol_Converter_GetChannel());

    /* 切换回以太网通道 */
    Protocol_Converter_SwitchChannel(PAYLOAD_CHANNEL_ETHERNET);
    TEST_ASSERT_EQUAL(PAYLOAD_CHANNEL_ETHERNET, Protocol_Converter_GetChannel());

    /* 再次切换到UART通道 */
    Protocol_Converter_SwitchChannel(PAYLOAD_CHANNEL_UART);
    TEST_ASSERT_EQUAL(PAYLOAD_CHANNEL_UART, Protocol_Converter_GetChannel());
}

/* 主测试运行器 */
int main(void)
{
    TEST_START("Protocol Converter Tests");

    RUN_TEST(test_ProtocolConverter_Init_Success);
    RUN_TEST(test_ProtocolConverter_GetStats);
    RUN_TEST(test_ProtocolConverter_SwitchChannel_Ethernet);
    RUN_TEST(test_ProtocolConverter_SwitchChannel_UART);
    RUN_TEST(test_ProtocolConverter_GetChannel);
    RUN_TEST(test_ProtocolConverter_GetStats_NullPointer);
    RUN_TEST(test_ProtocolConverter_SwitchChannel_Multiple);

    return TEST_END();
}
