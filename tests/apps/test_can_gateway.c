/************************************************************************
 * CAN网关单元测试
 ************************************************************************/

#include "../test_framework.h"
#include "can_gateway.h"
#include "can_protocol.h"
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
void test_CANGateway_Init_Success(void)
{
    int32 ret = CAN_Gateway_Init();
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
}

/* 测试用例2: 获取接收队列 */
void test_CANGateway_GetRxQueue(void)
{
    CAN_Gateway_Init();

    osal_id_t queue_id = CAN_Gateway_GetRxQueue();
    TEST_ASSERT_NOT_EQUAL(0, queue_id);
}

/* 测试用例3: 获取发送队列 */
void test_CANGateway_GetTxQueue(void)
{
    CAN_Gateway_Init();

    osal_id_t queue_id = CAN_Gateway_GetTxQueue();
    TEST_ASSERT_NOT_EQUAL(0, queue_id);
}

/* 测试用例4: 发送响应 */
void test_CANGateway_SendResponse(void)
{
    CAN_Gateway_Init();

    int32 ret = CAN_Gateway_SendResponse(1, STATUS_OK, 0x12345678);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
}

/* 测试用例5: 发送状态上报 */
void test_CANGateway_SendStatus(void)
{
    CAN_Gateway_Init();

    int32 ret = CAN_Gateway_SendStatus(0xABCDEF00);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
}

/* 测试用例6: 获取统计信息 */
void test_CANGateway_GetStats(void)
{
    CAN_Gateway_Init();

    uint32 rx_count = 0, tx_count = 0, err_count = 0;
    CAN_Gateway_GetStats(&rx_count, &tx_count, &err_count);

    /* 初始化后计数应该为0 */
    TEST_ASSERT_EQUAL(0, rx_count);
    TEST_ASSERT_EQUAL(0, tx_count);
    TEST_ASSERT_EQUAL(0, err_count);
}

/* 测试用例7: 发送响应后统计 */
void test_CANGateway_SendResponse_Stats(void)
{
    CAN_Gateway_Init();

    /* 发送几条响应 */
    CAN_Gateway_SendResponse(1, STATUS_OK, 0);
    CAN_Gateway_SendResponse(2, STATUS_OK, 0);

    uint32 rx_count = 0, tx_count = 0, err_count = 0;
    CAN_Gateway_GetStats(&rx_count, &tx_count, &err_count);

    /* 发送计数应该增加 */
    TEST_ASSERT_GREATER_OR_EQUAL(2, tx_count);
}

/* 测试用例8: 空指针保护 */
void test_CANGateway_GetStats_NullPointer(void)
{
    CAN_Gateway_Init();

    /* 传入NULL指针不应该崩溃 */
    CAN_Gateway_GetStats(NULL, NULL, NULL);

    /* 如果没有崩溃，测试通过 */
    TEST_ASSERT_TRUE(1);
}

/* 主测试运行器 */
int main(void)
{
    TEST_START("CAN Gateway Tests");

    RUN_TEST(test_CANGateway_Init_Success);
    RUN_TEST(test_CANGateway_GetRxQueue);
    RUN_TEST(test_CANGateway_GetTxQueue);
    RUN_TEST(test_CANGateway_SendResponse);
    RUN_TEST(test_CANGateway_SendStatus);
    RUN_TEST(test_CANGateway_GetStats);
    RUN_TEST(test_CANGateway_SendResponse_Stats);
    RUN_TEST(test_CANGateway_GetStats_NullPointer);

    return TEST_END();
}
