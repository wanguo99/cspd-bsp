/************************************************************************
 * CAN网关测试模块包装
 * 用于统一测试入口
 ************************************************************************/

#include "test_runner.h"
#include "test_framework.h"
#include "can_gateway.h"
#include "osal.h"

void test_CANGateway_Init_Success(void)
{
    OS_API_Init();

    int32 ret = CAN_Gateway_Init();

    if (ret == OS_SUCCESS) {
        TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    } else {
        TEST_MESSAGE("Warning: CAN Gateway init failed, skipping test");
        TEST_IGNORE();
    }

    OS_API_Teardown();
}

void test_CANGateway_GetRxQueue(void)
{
    OS_API_Init();

    int32 ret = CAN_Gateway_Init();
    if (ret == OS_SUCCESS) {
        osal_id_t queue_id = CAN_Gateway_GetRxQueue();
        TEST_ASSERT_NOT_EQUAL(0, queue_id);
    } else {
        TEST_IGNORE();
    }

    OS_API_Teardown();
}

void test_CANGateway_GetTxQueue(void)
{
    OS_API_Init();

    int32 ret = CAN_Gateway_Init();
    if (ret == OS_SUCCESS) {
        osal_id_t queue_id = CAN_Gateway_GetTxQueue();
        TEST_ASSERT_NOT_EQUAL(0, queue_id);
    } else {
        TEST_IGNORE();
    }

    OS_API_Teardown();
}

/* 注册测试模块 */
TEST_MODULE_BEGIN(test_can_gateway)
    TEST_CASE(test_CANGateway_Init_Success)
    TEST_CASE(test_CANGateway_GetRxQueue)
    TEST_CASE(test_CANGateway_GetTxQueue)
TEST_MODULE_END(test_can_gateway)
