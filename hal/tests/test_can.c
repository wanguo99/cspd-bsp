/**
 * @file test_can.c
 * @brief HAL CAN驱动单元测试
 *
 * 使用新的libtest框架，测试自动注册
 */

#include "libutest.h"
#include "test_assert.h"
#include "test_registry.h"
#include "hal_can.h"
#include "osal.h"

static hal_can_handle_t test_handle;

/* 测试用例1: CAN初始化成功 */
TEST_CASE(test_can_init_success)
{
    hal_can_config_t config = {
        .interface = "vcan0",
        .baudrate = 500000,
        .rx_timeout = 1000,
        .tx_timeout = 1000
    };

    int32_t ret = HAL_CAN_Init(&config, &test_handle);

    if (ret != OS_SUCCESS) {
        TEST_SKIP_IF(true, "vcan0 not available");
    }

    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    TEST_ASSERT_NOT_NULL(test_handle);
    HAL_CAN_Deinit(test_handle);
}

/* 测试用例2: CAN初始化失败 - 空配置 */
TEST_CASE(test_can_init_null_config)
{
    int32_t ret = HAL_CAN_Init(NULL, &test_handle);
    TEST_ASSERT_NOT_EQUAL(OS_SUCCESS, ret);
}

/* 测试用例3: CAN初始化失败 - 空句柄 */
TEST_CASE(test_can_init_null_handle)
{
    hal_can_config_t config = {
        .interface = "vcan0",
        .baudrate = 500000,
        .rx_timeout = 1000,
        .tx_timeout = 1000
    };

    int32_t ret = HAL_CAN_Init(&config, NULL);
    TEST_ASSERT_NOT_EQUAL(OS_SUCCESS, ret);
}

/* 注册测试套件 - 自动注册 */
TEST_SUITE_BEGIN(hal_can, "hal", "HAL")
    TEST_CASE_REF(test_can_init_success)
    TEST_CASE_REF(test_can_init_null_config)
    TEST_CASE_REF(test_can_init_null_handle)
TEST_SUITE_END(hal_can, "hal", "HAL")
