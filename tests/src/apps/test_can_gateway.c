/************************************************************************
 * CAN网关测试模块包装
 * 用于统一测试入口
 ************************************************************************/

#include "test_runner.h"
#include "test_framework.h"
#include "can_gateway.h"
#include "osal.h"

void test_can_gateway_init_success(void)
{
    OS_API_Init();

    int32 ret = CAN_Gateway_Init();

    if (ret != OS_SUCCESS) {
        TEST_WARNING("CAN Gateway init failed, hardware not available");
    }

    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    OS_API_Teardown();
}

void test_can_gateway_get_rx_queue(void)
{
    OS_API_Init();

    int32 ret = CAN_Gateway_Init();

    if (ret != OS_SUCCESS) {
        TEST_WARNING("CAN Gateway init failed, hardware not available");
    }

    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    osal_id_t queue_id = CAN_Gateway_GetRxQueue();
    TEST_ASSERT_NOT_EQUAL(0, queue_id);

    OS_API_Teardown();
}

void test_can_gateway_get_tx_queue(void)
{
    OS_API_Init();

    int32 ret = CAN_Gateway_Init();

    if (ret != OS_SUCCESS) {
        TEST_WARNING("CAN Gateway init failed, hardware not available");
    }

    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    osal_id_t queue_id = CAN_Gateway_GetTxQueue();
    TEST_ASSERT_NOT_EQUAL(0, queue_id);

    OS_API_Teardown();
}

/* 注册测试模块 */
TEST_MODULE_BEGIN(test_can_gateway)
    TEST_CASE(test_can_gateway_init_success)
    TEST_CASE(test_can_gateway_get_rx_queue)
    TEST_CASE(test_can_gateway_get_tx_queue)
TEST_MODULE_END(test_can_gateway)
