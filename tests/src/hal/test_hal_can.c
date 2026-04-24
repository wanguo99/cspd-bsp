/************************************************************************
 * HAL CAN驱动测试模块包装
 * 用于统一测试入口
 ************************************************************************/

#include "test_runner.h"
#include "test_framework.h"
#include "hal_can.h"
#include "osal.h"
#include <string.h>

static hal_can_handle_t test_handle;

/* 测试用例 */
void test_HAL_CAN_Init_Success(void)
{
    OS_API_Init();

    hal_can_config_t config = {
        .interface = "vcan0",
        .baudrate = 500000,
        .rx_timeout = 1000,
        .tx_timeout = 1000
    };

    int32 ret = HAL_CanInit(&config, &test_handle);

    if (ret != OS_SUCCESS) {
        TEST_WARNING("vcan0 not available, test will fail");
    }

    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    TEST_ASSERT_NOT_NULL(test_handle);
    HAL_CanDeinit(test_handle);

    OS_API_Teardown();
}

void test_HAL_CAN_Init_NullConfig(void)
{
    OS_API_Init();

    int32 ret = HAL_CanInit(NULL, &test_handle);
    TEST_ASSERT_NOT_EQUAL(OS_SUCCESS, ret);

    OS_API_Teardown();
}

void test_HAL_CAN_Init_NullHandle(void)
{
    OS_API_Init();

    hal_can_config_t config = {
        .interface = "vcan0",
        .baudrate = 500000,
        .rx_timeout = 1000,
        .tx_timeout = 1000
    };

    int32 ret = HAL_CanInit(&config, NULL);
    TEST_ASSERT_NOT_EQUAL(OS_SUCCESS, ret);

    OS_API_Teardown();
}

/* 注册测试模块 */
TEST_MODULE_BEGIN(test_hal_can)
    TEST_CASE(test_HAL_CAN_Init_Success)
    TEST_CASE(test_HAL_CAN_Init_NullConfig)
    TEST_CASE(test_HAL_CAN_Init_NullHandle)
TEST_MODULE_END(test_hal_can)
