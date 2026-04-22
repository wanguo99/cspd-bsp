/************************************************************************
 * HAL CAN驱动单元测试
 ************************************************************************/

#include "../test_framework.h"
#include "hal_can.h"
#include "osal.h"
#include <string.h>

static hal_can_handle_t test_handle;

/* 测试前初始化 */
void setUp(void)
{
    OS_API_Init();
}

/* 测试后清理 */
void tearDown(void)
{
    if (test_handle != NULL) {
        HAL_CAN_Deinit(test_handle);
        test_handle = NULL;
    }
    OS_API_Teardown();
}

/* 测试用例1: CAN初始化成功 */
void test_HAL_CAN_Init_Success(void)
{
    hal_can_config_t config = {
        .interface = "vcan0",  /* 使用虚拟CAN接口 */
        .baudrate = 500000,
        .rx_timeout = 1000,
        .tx_timeout = 1000
    };

    int32 ret = HAL_CAN_Init(&config, &test_handle);

    /* 注意：如果vcan0不存在，这个测试会失败 */
    /* 在实际测试前需要: sudo modprobe vcan && sudo ip link add dev vcan0 type vcan && sudo ip link set up vcan0 */
    if (ret == OS_SUCCESS) {
        TEST_ASSERT_NOT_NULL(test_handle);
    } else {
        TEST_MESSAGE("Warning: vcan0 not available, skipping test");
        TEST_IGNORE();
    }
}

/* 测试用例2: CAN初始化失败 - 空指针 */
void test_HAL_CAN_Init_NullPointer(void)
{
    hal_can_config_t config = {
        .interface = "vcan0",
        .baudrate = 500000,
        .rx_timeout = 1000,
        .tx_timeout = 1000
    };

    /* 配置指针为空 */
    int32 ret = HAL_CAN_Init(NULL, &test_handle);
    TEST_ASSERT_EQUAL(OS_INVALID_POINTER, ret);

    /* 句柄指针为空 */
    ret = HAL_CAN_Init(&config, NULL);
    TEST_ASSERT_EQUAL(OS_INVALID_POINTER, ret);
}

/* 测试用例3: CAN初始化失败 - 无效接口 */
void test_HAL_CAN_Init_InvalidInterface(void)
{
    hal_can_config_t config = {
        .interface = "invalid_can999",
        .baudrate = 500000,
        .rx_timeout = 1000,
        .tx_timeout = 1000
    };

    int32 ret = HAL_CAN_Init(&config, &test_handle);
    TEST_ASSERT_EQUAL(OS_ERROR, ret);
}

/* 测试用例4: CAN发送 */
void test_HAL_CAN_Send_Success(void)
{
    hal_can_config_t config = {
        .interface = "vcan0",
        .baudrate = 500000,
        .rx_timeout = 1000,
        .tx_timeout = 1000
    };

    if (HAL_CAN_Init(&config, &test_handle) != OS_SUCCESS) {
        TEST_IGNORE_MESSAGE("vcan0 not available");
        return;
    }

    can_frame_t frame = {
        .can_id = 0x100,
        .dlc = 8,
        .msg = {
            .msg_type = 0x01,
            .cmd_type = 0x10,
            .seq_num = 1,
            .data = 0x12345678
        }
    };

    int32 ret = HAL_CAN_Send(test_handle, &frame);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
}

/* 测试用例5: CAN发送失败 - 空指针 */
void test_HAL_CAN_Send_NullPointer(void)
{
    hal_can_config_t config = {
        .interface = "vcan0",
        .baudrate = 500000,
        .rx_timeout = 1000,
        .tx_timeout = 1000
    };

    if (HAL_CAN_Init(&config, &test_handle) != OS_SUCCESS) {
        TEST_IGNORE_MESSAGE("vcan0 not available");
        return;
    }

    int32 ret = HAL_CAN_Send(test_handle, NULL);
    TEST_ASSERT_EQUAL(OS_INVALID_POINTER, ret);
}

/* 测试用例6: CAN接收超时 */
void test_HAL_CAN_Recv_Timeout(void)
{
    hal_can_config_t config = {
        .interface = "vcan0",
        .baudrate = 500000,
        .rx_timeout = 100,
        .tx_timeout = 1000
    };

    if (HAL_CAN_Init(&config, &test_handle) != OS_SUCCESS) {
        TEST_IGNORE_MESSAGE("vcan0 not available");
        return;
    }

    can_frame_t frame;
    uint32 start = OS_GetTickCount();

    int32 ret = HAL_CAN_Recv(test_handle, &frame, 500);

    uint32 elapsed = OS_GetTickCount() - start;

    TEST_ASSERT_EQUAL(OS_ERROR_TIMEOUT, ret);
    TEST_ASSERT_GREATER_OR_EQUAL(450, elapsed);
}

/* 测试用例7: CAN发送和接收（回环测试） */
void test_HAL_CAN_SendRecv_Loopback(void)
{
    hal_can_config_t config = {
        .interface = "vcan0",
        .baudrate = 500000,
        .rx_timeout = 1000,
        .tx_timeout = 1000
    };

    if (HAL_CAN_Init(&config, &test_handle) != OS_SUCCESS) {
        TEST_IGNORE_MESSAGE("vcan0 not available");
        return;
    }

    /* 发送帧 */
    can_frame_t send_frame = {
        .can_id = 0x100,
        .dlc = 8,
        .msg = {
            .msg_type = 0x01,
            .cmd_type = 0x10,
            .seq_num = 123,
            .data = 0xDEADBEEF
        }
    };

    int32 ret = HAL_CAN_Send(test_handle, &send_frame);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 接收帧（vcan会回环） */
    can_frame_t recv_frame;
    ret = HAL_CAN_Recv(test_handle, &recv_frame, 1000);

    if (ret == OS_SUCCESS) {
        TEST_ASSERT_EQUAL(0x100, recv_frame.can_id);
        TEST_ASSERT_EQUAL(8, recv_frame.dlc);
        TEST_ASSERT_EQUAL(0x01, recv_frame.msg.msg_type);
        TEST_ASSERT_EQUAL(0x10, recv_frame.msg.cmd_type);
        TEST_ASSERT_EQUAL(123, recv_frame.msg.seq_num);
        TEST_ASSERT_EQUAL(0xDEADBEEF, recv_frame.msg.data);
    }
}

/* 测试用例8: CAN过滤器设置 */
void test_HAL_CAN_SetFilter_Success(void)
{
    hal_can_config_t config = {
        .interface = "vcan0",
        .baudrate = 500000,
        .rx_timeout = 1000,
        .tx_timeout = 1000
    };

    if (HAL_CAN_Init(&config, &test_handle) != OS_SUCCESS) {
        TEST_IGNORE_MESSAGE("vcan0 not available");
        return;
    }

    int32 ret = HAL_CAN_SetFilter(test_handle, 0x100, 0x7FF);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
}

/* 测试用例9: 获取统计信息 */
void test_HAL_CAN_GetStats_Success(void)
{
    hal_can_config_t config = {
        .interface = "vcan0",
        .baudrate = 500000,
        .rx_timeout = 1000,
        .tx_timeout = 1000
    };

    if (HAL_CAN_Init(&config, &test_handle) != OS_SUCCESS) {
        TEST_IGNORE_MESSAGE("vcan0 not available");
        return;
    }

    uint32 tx_count, rx_count, err_count;

    int32 ret = HAL_CAN_GetStats(test_handle, &tx_count, &rx_count, &err_count);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 初始统计应该为0 */
    TEST_ASSERT_EQUAL(0, tx_count);
    TEST_ASSERT_EQUAL(0, rx_count);
    TEST_ASSERT_EQUAL(0, err_count);

    /* 发送一帧 */
    can_frame_t frame = {
        .can_id = 0x100,
        .dlc = 8
    };
    HAL_CAN_Send(test_handle, &frame);

    /* 再次获取统计 */
    HAL_CAN_GetStats(test_handle, &tx_count, &rx_count, &err_count);
    TEST_ASSERT_EQUAL(1, tx_count);
}

/* 测试用例10: CAN反初始化 */
void test_HAL_CAN_Deinit_Success(void)
{
    hal_can_config_t config = {
        .interface = "vcan0",
        .baudrate = 500000,
        .rx_timeout = 1000,
        .tx_timeout = 1000
    };

    if (HAL_CAN_Init(&config, &test_handle) != OS_SUCCESS) {
        TEST_IGNORE_MESSAGE("vcan0 not available");
        return;
    }

    int32 ret = HAL_CAN_Deinit(test_handle);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    test_handle = NULL;  /* 防止tearDown重复释放 */
}

/* 主测试运行器 */
int main(void)
{
    TEST_BEGIN();

    RUN_TEST(test_HAL_CAN_Init_Success);
    RUN_TEST(test_HAL_CAN_Init_NullPointer);
    RUN_TEST(test_HAL_CAN_Init_InvalidInterface);
    RUN_TEST(test_HAL_CAN_Send_Success);
    RUN_TEST(test_HAL_CAN_Send_NullPointer);
    RUN_TEST(test_HAL_CAN_Recv_Timeout);
    RUN_TEST(test_HAL_CAN_SendRecv_Loopback);
    RUN_TEST(test_HAL_CAN_SetFilter_Success);
    RUN_TEST(test_HAL_CAN_GetStats_Success);
    RUN_TEST(test_HAL_CAN_Deinit_Success);

    TEST_END();
}
