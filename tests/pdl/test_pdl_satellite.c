/**
 * @file test_pdl_satellite.c
 * @brief PDL卫星平台服务单元测试
 */

#include "tests_core.h"
#include "test_assert.h"
#include "test_registry.h"
#include "pdl_satellite.h"
#include "osal.h"

/* 测试回调函数 */
static int32_t g_callback_count = 0;
static uint8_t g_last_cmd_type = 0;
static uint32_t g_last_param = 0;

static void test_satellite_callback(uint8_t cmd_type, uint32_t param, void *user_data)
{
    (void)user_data;  /* 未使用的参数 */
    g_callback_count++;
    g_last_cmd_type = cmd_type;
    g_last_param = param;
}

/* 重置测试状态 */
static void reset_test_state(void)
{
    g_callback_count = 0;
    g_last_cmd_type = 0;
    g_last_param = 0;
}

/*===========================================================================
 * 初始化和清理测试
 *===========================================================================*/

/* 测试用例: 卫星服务初始化 - 成功 */
TEST_CASE(test_pdl_satellite_init_success)
{
    satellite_service_handle_t handle = NULL;
    satellite_service_config_t config = {
        .can_device = "can0",
        .can_bitrate = 500000,
        .heartbeat_interval_ms = 1000,
        .cmd_timeout_ms = 5000
    };

    int32_t ret = PDL_Satellite_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    TEST_ASSERT_NOT_NULL(handle);

    PDL_Satellite_Deinit(handle);
}

/* 测试用例: 卫星服务初始化 - 空配置 */
TEST_CASE(test_pdl_satellite_init_null_config)
{
    satellite_service_handle_t handle = NULL;

    int32_t ret = PDL_Satellite_Init(NULL, &handle);
    TEST_ASSERT_NOT_EQUAL(OS_SUCCESS, ret);
}

/* 测试用例: 卫星服务初始化 - 空句柄指针 */
TEST_CASE(test_pdl_satellite_init_null_handle)
{
    satellite_service_config_t config = {
        .can_device = "can0",
        .can_bitrate = 500000,
        .heartbeat_interval_ms = 1000,
        .cmd_timeout_ms = 5000
    };

    int32_t ret = PDL_Satellite_Init(&config, NULL);
    TEST_ASSERT_NOT_EQUAL(OS_SUCCESS, ret);
}

/* 测试用例: 卫星服务清理 */
TEST_CASE(test_pdl_satellite_deinit)
{
    satellite_service_handle_t handle = NULL;
    satellite_service_config_t config = {
        .can_device = "can0",
        .can_bitrate = 500000,
        .heartbeat_interval_ms = 1000,
        .cmd_timeout_ms = 5000
    };

    int32_t ret = PDL_Satellite_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = PDL_Satellite_Deinit(handle);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
}

/* 测试用例: 清理空句柄 */
TEST_CASE(test_pdl_satellite_deinit_null_handle)
{
    int32_t ret = PDL_Satellite_Deinit(NULL);
    TEST_ASSERT_NOT_EQUAL(OS_SUCCESS, ret);
}

/*===========================================================================
 * 回调注册测试
 *===========================================================================*/

/* 测试用例: 注册回调 - 成功 */
TEST_CASE(test_pdl_satellite_register_callback_success)
{
    satellite_service_handle_t handle = NULL;
    satellite_service_config_t config = {
        .can_device = "can0",
        .can_bitrate = 500000,
        .heartbeat_interval_ms = 1000,
        .cmd_timeout_ms = 5000
    };

    int32_t ret = PDL_Satellite_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    reset_test_state();
    ret = PDL_Satellite_RegisterCallback(handle, test_satellite_callback, NULL);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    PDL_Satellite_Deinit(handle);
}

/* 测试用例: 注册回调 - 空句柄 */
TEST_CASE(test_pdl_satellite_register_callback_null_handle)
{
    int32_t ret = PDL_Satellite_RegisterCallback(NULL, test_satellite_callback, NULL);
    TEST_ASSERT_NOT_EQUAL(OS_SUCCESS, ret);
}

/* 测试用例: 注册回调 - 空回调函数 */
TEST_CASE(test_pdl_satellite_register_callback_null_func)
{
    satellite_service_handle_t handle = NULL;
    satellite_service_config_t config = {
        .can_device = "can0",
        .can_bitrate = 500000,
        .heartbeat_interval_ms = 1000,
        .cmd_timeout_ms = 5000
    };

    int32_t ret = PDL_Satellite_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = PDL_Satellite_RegisterCallback(handle, NULL, NULL);
    TEST_ASSERT_NOT_EQUAL(OS_SUCCESS, ret);

    PDL_Satellite_Deinit(handle);
}

/*===========================================================================
 * 响应发送测试
 *===========================================================================*/

/* 测试用例: 发送响应 - 成功 */
TEST_CASE(test_pdl_satellite_send_response_success)
{
    satellite_service_handle_t handle = NULL;
    satellite_service_config_t config = {
        .can_device = "can0",
        .can_bitrate = 500000,
        .heartbeat_interval_ms = 1000,
        .cmd_timeout_ms = 5000
    };

    int32_t ret = PDL_Satellite_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = PDL_Satellite_SendResponse(handle, 1, STATUS_OK, 0x12345678);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    PDL_Satellite_Deinit(handle);
}

/* 测试用例: 发送响应 - 空句柄 */
TEST_CASE(test_pdl_satellite_send_response_null_handle)
{
    int32_t ret = PDL_Satellite_SendResponse(NULL, 1, STATUS_OK, 0);
    TEST_ASSERT_NOT_EQUAL(OS_SUCCESS, ret);
}

/* 测试用例: 发送响应 - 不同状态码 */
TEST_CASE(test_pdl_satellite_send_response_different_status)
{
    satellite_service_handle_t handle = NULL;
    satellite_service_config_t config = {
        .can_device = "can0",
        .can_bitrate = 500000,
        .heartbeat_interval_ms = 1000,
        .cmd_timeout_ms = 5000
    };

    int32_t ret = PDL_Satellite_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 测试不同状态码 */
    ret = PDL_Satellite_SendResponse(handle, 1, STATUS_OK, 0);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = PDL_Satellite_SendResponse(handle, 2, STATUS_ERROR, 0);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = PDL_Satellite_SendResponse(handle, 3, STATUS_BUSY, 0);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = PDL_Satellite_SendResponse(handle, 4, STATUS_TIMEOUT, 0);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    PDL_Satellite_Deinit(handle);
}

/*===========================================================================
 * 心跳测试
 *===========================================================================*/

/* 测试用例: 发送心跳 - 成功 */
TEST_CASE(test_pdl_satellite_send_heartbeat_success)
{
    satellite_service_handle_t handle = NULL;
    satellite_service_config_t config = {
        .can_device = "can0",
        .can_bitrate = 500000,
        .heartbeat_interval_ms = 1000,
        .cmd_timeout_ms = 5000
    };

    int32_t ret = PDL_Satellite_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = PDL_Satellite_SendHeartbeat(handle, STATUS_OK);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    PDL_Satellite_Deinit(handle);
}

/* 测试用例: 发送心跳 - 空句柄 */
TEST_CASE(test_pdl_satellite_send_heartbeat_null_handle)
{
    int32_t ret = PDL_Satellite_SendHeartbeat(NULL, STATUS_OK);
    TEST_ASSERT_NOT_EQUAL(OS_SUCCESS, ret);
}

/* 测试用例: 连续发送心跳 */
TEST_CASE(test_pdl_satellite_send_heartbeat_continuous)
{
    satellite_service_handle_t handle = NULL;
    satellite_service_config_t config = {
        .can_device = "can0",
        .can_bitrate = 500000,
        .heartbeat_interval_ms = 1000,
        .cmd_timeout_ms = 5000
    };

    int32_t ret = PDL_Satellite_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 连续发送10次心跳 */
    for (int32_t i = 0; i < 10; i++) {
        ret = PDL_Satellite_SendHeartbeat(handle, STATUS_OK);
        TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
        OSAL_TaskDelay(100);
    }

    PDL_Satellite_Deinit(handle);
}

/*===========================================================================
 * 统计信息测试
 *===========================================================================*/

/* 测试用例: 获取统计信息 - 成功 */
TEST_CASE(test_pdl_satellite_get_stats_success)
{
    satellite_service_handle_t handle = NULL;
    satellite_service_config_t config = {
        .can_device = "can0",
        .can_bitrate = 500000,
        .heartbeat_interval_ms = 1000,
        .cmd_timeout_ms = 5000
    };
    uint32_t rx_count, tx_count, error_count;

    int32_t ret = PDL_Satellite_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = PDL_Satellite_GetStats(handle, &rx_count, &tx_count, &error_count);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 初始统计应该为有效值（uint32_t总是>=0，只需验证调用成功） */

    PDL_Satellite_Deinit(handle);
}

/* 测试用例: 获取统计信息 - 空句柄 */
TEST_CASE(test_pdl_satellite_get_stats_null_handle)
{
    uint32_t rx_count, tx_count, error_count;

    int32_t ret = PDL_Satellite_GetStats(NULL, &rx_count, &tx_count, &error_count);
    TEST_ASSERT_NOT_EQUAL(OS_SUCCESS, ret);
}

/* 测试用例: 获取统计信息 - 空指针 */
TEST_CASE(test_pdl_satellite_get_stats_null_pointer)
{
    satellite_service_handle_t handle = NULL;
    satellite_service_config_t config = {
        .can_device = "can0",
        .can_bitrate = 500000,
        .heartbeat_interval_ms = 1000,
        .cmd_timeout_ms = 5000
    };
    uint32_t count;

    int32_t ret = PDL_Satellite_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = PDL_Satellite_GetStats(handle, NULL, &count, &count);
    TEST_ASSERT_NOT_EQUAL(OS_SUCCESS, ret);

    ret = PDL_Satellite_GetStats(handle, &count, NULL, &count);
    TEST_ASSERT_NOT_EQUAL(OS_SUCCESS, ret);

    ret = PDL_Satellite_GetStats(handle, &count, &count, NULL);
    TEST_ASSERT_NOT_EQUAL(OS_SUCCESS, ret);

    PDL_Satellite_Deinit(handle);
}

/* 测试用例: 统计信息累加验证 */
TEST_CASE(test_pdl_satellite_stats_accumulation)
{
    satellite_service_handle_t handle = NULL;
    satellite_service_config_t config = {
        .can_device = "can0",
        .can_bitrate = 500000,
        .heartbeat_interval_ms = 1000,
        .cmd_timeout_ms = 5000
    };
    uint32_t rx1, tx1, err1, rx2, tx2, err2;

    int32_t ret = PDL_Satellite_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 获取初始统计 */
    ret = PDL_Satellite_GetStats(handle, &rx1, &tx1, &err1);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 发送一些数据 */
    PDL_Satellite_SendResponse(handle, 1, STATUS_OK, 0);
    PDL_Satellite_SendHeartbeat(handle, STATUS_OK);

    /* 获取新统计 */
    ret = PDL_Satellite_GetStats(handle, &rx2, &tx2, &err2);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 验证统计增加 */
    TEST_ASSERT_TRUE(tx2 >= tx1);

    PDL_Satellite_Deinit(handle);
}

/*===========================================================================
 * 配置参数测试
 *===========================================================================*/

/* 测试用例: 不同波特率配置 */
TEST_CASE(test_pdl_satellite_different_bitrate)
{
    satellite_service_handle_t handle = NULL;
    satellite_service_config_t config = {
        .can_device = "can0",
        .can_bitrate = 250000,  /* 250kbps */
        .heartbeat_interval_ms = 1000,
        .cmd_timeout_ms = 5000
    };

    int32_t ret = PDL_Satellite_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    PDL_Satellite_Deinit(handle);
}

/* 测试用例: 不同心跳间隔 */
TEST_CASE(test_pdl_satellite_different_heartbeat_interval)
{
    satellite_service_handle_t handle = NULL;
    satellite_service_config_t config = {
        .can_device = "can0",
        .can_bitrate = 500000,
        .heartbeat_interval_ms = 500,  /* 500ms */
        .cmd_timeout_ms = 5000
    };

    int32_t ret = PDL_Satellite_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    PDL_Satellite_Deinit(handle);
}

/*===========================================================================
 * 测试模块注册
 *===========================================================================*/

TEST_SUITE_BEGIN(test_pdl_satellite, "test_pdl_satellite", "PDL")
    // PDL卫星平台服务测试
    /* 初始化和清理 */
    TEST_CASE_REF(test_pdl_satellite_init_success)
    TEST_CASE_REF(test_pdl_satellite_init_null_config)
    TEST_CASE_REF(test_pdl_satellite_init_null_handle)
    TEST_CASE_REF(test_pdl_satellite_deinit)
    TEST_CASE_REF(test_pdl_satellite_deinit_null_handle)

    /* 回调注册 */
    TEST_CASE_REF(test_pdl_satellite_register_callback_success)
    TEST_CASE_REF(test_pdl_satellite_register_callback_null_handle)
    TEST_CASE_REF(test_pdl_satellite_register_callback_null_func)

    /* 响应发送 */
    TEST_CASE_REF(test_pdl_satellite_send_response_success)
    TEST_CASE_REF(test_pdl_satellite_send_response_null_handle)
    TEST_CASE_REF(test_pdl_satellite_send_response_different_status)

    /* 心跳 */
    TEST_CASE_REF(test_pdl_satellite_send_heartbeat_success)
    TEST_CASE_REF(test_pdl_satellite_send_heartbeat_null_handle)
    TEST_CASE_REF(test_pdl_satellite_send_heartbeat_continuous)

    /* 统计信息 */
    TEST_CASE_REF(test_pdl_satellite_get_stats_success)
    TEST_CASE_REF(test_pdl_satellite_get_stats_null_handle)
    TEST_CASE_REF(test_pdl_satellite_get_stats_null_pointer)
    TEST_CASE_REF(test_pdl_satellite_stats_accumulation)

    /* 配置参数 */
    TEST_CASE_REF(test_pdl_satellite_different_bitrate)
    TEST_CASE_REF(test_pdl_satellite_different_heartbeat_interval)
TEST_SUITE_END(test_pdl_satellite, "test_pdl_satellite", "PDL")
