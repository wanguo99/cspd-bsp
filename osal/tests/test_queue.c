/**
 * @file test_queue.c
 * @brief OSAL消息队列单元测试
 *
 * 使用新的libtest框架，测试自动注册
 */

#include "libutest.h"
#include "test_assert.h"
#include "test_registry.h"
#include "osal.h"

/* 测试用例1: 队列创建成功 */
TEST_CASE(test_queue_create_success)
{
    osal_id_t queue_id;

    int32_t ret = OSAL_QueueCreate(&queue_id, "TEST_QUEUE", 10, 64, 0);

    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    TEST_ASSERT_NOT_EQUAL(OS_OBJECT_ID_UNDEFINED, queue_id);

    OSAL_QueueDelete(queue_id);
}

/* 测试用例2: 队列创建失败 - 空指针 */
TEST_CASE(test_queue_create_nullpointer)
{
    int32_t ret = OSAL_QueueCreate(NULL, "TEST", 10, 64, 0);
    TEST_ASSERT_EQUAL(OS_INVALID_POINTER, ret);
}

/* 测试用例3: 队列创建失败 - 无效大小 */
TEST_CASE(test_queue_create_invalidsize)
{
    osal_id_t queue_id;

    /* 深度为0 */
    int32_t ret = OSAL_QueueCreate(&queue_id, "TEST", 0, 64, 0);
    TEST_ASSERT_EQUAL(OS_QUEUE_INVALID_SIZE, ret);

    /* 消息大小为0 */
    ret = OSAL_QueueCreate(&queue_id, "TEST2", 10, 0, 0);
    TEST_ASSERT_EQUAL(OS_QUEUE_INVALID_SIZE, ret);
}

/* 测试用例4: 队列创建失败 - 名称重复 */
TEST_CASE(test_queue_create_nametaken)
{
    osal_id_t queue_id1, queue_id2;

    int32_t ret = OSAL_QueueCreate(&queue_id1, "DUP_QUEUE", 10, 64, 0);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = OSAL_QueueCreate(&queue_id2, "DUP_QUEUE", 10, 64, 0);
    TEST_ASSERT_EQUAL(OS_ERR_NAME_TAKEN, ret);

    OSAL_QueueDelete(queue_id1);
}

/* 测试用例5: 队列发送和接收 */
TEST_CASE(test_queue_putget_success)
{
    osal_id_t queue_id;
    OSAL_QueueCreate(&queue_id, "TEST_Q", 10, 64, 0);

    uint8_t send_data[64];
    uint8_t recv_data[64];
    uint32_t size_copied;

    OSAL_Strcpy((str_t *)send_data, "Hello World");

    /* 发送消息 */
    int32_t ret = OSAL_QueuePut(queue_id, send_data, 64, 0);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 接收消息 */
    ret = OSAL_QueueGet(queue_id, recv_data, 64, &size_copied, 1000);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    TEST_ASSERT_EQUAL(64, size_copied);
    TEST_ASSERT_STRING_EQUAL("Hello World", (str_t *)recv_data);

    OSAL_QueueDelete(queue_id);
}

/* 测试用例6: 队列接收 - 空队列非阻塞 */
TEST_CASE(test_queue_get_empty)
{
    osal_id_t queue_id;
    OSAL_QueueCreate(&queue_id, "TEST_Q", 10, 64, 0);

    uint8_t data[64];
    uint32_t size;

    int32_t ret = OSAL_QueueGet(queue_id, data, 64, &size, OS_CHECK);
    TEST_ASSERT_EQUAL(OS_QUEUE_EMPTY, ret);

    OSAL_QueueDelete(queue_id);
}

/* 测试用例7: 队列接收 - 超时 */
TEST_CASE(test_queue_get_timeout)
{
    osal_id_t queue_id;
    OSAL_QueueCreate(&queue_id, "TEST_Q", 10, 64, 0);

    uint8_t data[64];
    uint32_t size;
    uint32_t start = OS_GetTickCount();

    int32_t ret = OSAL_QueueGet(queue_id, data, 64, &size, 500);

    uint32_t elapsed = OS_GetTickCount() - start;

    TEST_ASSERT_EQUAL(OS_QUEUE_TIMEOUT, ret);
    TEST_ASSERT_GREATER_OR_EQUAL(450, elapsed);
    TEST_ASSERT_LESS_OR_EQUAL(550, elapsed);

    OSAL_QueueDelete(queue_id);
}

/* 测试用例8: 队列满 */
TEST_CASE(test_queue_put_full)
{
    osal_id_t queue_id;
    OSAL_QueueCreate(&queue_id, "TEST_Q", 2, 64, 0);

    uint8_t data[64] = "Test";

    /* 填满队列 */
    TEST_ASSERT_EQUAL(OS_SUCCESS, OSAL_QueuePut(queue_id, data, 64, 0));
    TEST_ASSERT_EQUAL(OS_SUCCESS, OSAL_QueuePut(queue_id, data, 64, 0));

    OSAL_QueueDelete(queue_id);
}

/* 测试用例9: 多消息发送接收 */
TEST_CASE(test_queue_putget_multiple)
{
    osal_id_t queue_id;
    OSAL_QueueCreate(&queue_id, "TEST_Q", 5, 64, 0);

    uint8_t send_data[5][64];
    uint8_t recv_data[64];
    uint32_t size;

    /* 发送5条消息 */
    for (int32_t i = 0; i < 5; i++) {
        OSAL_Sprintf((str_t *)send_data[i], "Message %d", i);
        TEST_ASSERT_EQUAL(OS_SUCCESS,
                         OSAL_QueuePut(queue_id, send_data[i], 64, 0));
    }

    /* 接收5条消息 */
    for (int32_t i = 0; i < 5; i++) {
        TEST_ASSERT_EQUAL(OS_SUCCESS,
                         OSAL_QueueGet(queue_id, recv_data, 64, &size, 1000));

        str_t expected[64];
        OSAL_Sprintf(expected, "Message %d", i);
        TEST_ASSERT_STRING_EQUAL(expected, (str_t *)recv_data);
    }

    OSAL_QueueDelete(queue_id);
}

/* 测试用例10: 根据名称获取队列ID */
TEST_CASE(test_queue_getid_by_name_success)
{
    osal_id_t queue_id1, queue_id2;

    OSAL_QueueCreate(&queue_id1, "NAMED_Q", 10, 64, 0);

    int32_t ret = OSAL_QueueGetIdByName(&queue_id2, "NAMED_Q");

    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    TEST_ASSERT_EQUAL(queue_id1, queue_id2);

    OSAL_QueueDelete(queue_id1);
}

/* 测试用例11: 根据名称获取队列ID - 未找到 */
TEST_CASE(test_queue_getid_by_name_not_found)
{
    osal_id_t queue_id;

    int32_t ret = OSAL_QueueGetIdByName(&queue_id, "NONEXISTENT");
    TEST_ASSERT_EQUAL(OS_ERR_NAME_NOT_FOUND, ret);
}

/* 测试用例12: 队列删除 */
TEST_CASE(test_queue_delete_success)
{
    osal_id_t queue_id;
    OSAL_QueueCreate(&queue_id, "TEST_Q", 10, 64, 0);

    int32_t ret = OSAL_QueueDelete(queue_id);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
}

/* 测试用例13: 队列删除 - 无效ID */
TEST_CASE(test_queue_delete_invalidid)
{
    int32_t ret = OSAL_QueueDelete(9999);
    TEST_ASSERT_EQUAL(OS_ERR_INVALID_ID, ret);
}

/* 注册测试套件 - 自动注册 */
TEST_SUITE_BEGIN(osal_queue, "osal", "OSAL")
    TEST_CASE_REF(test_queue_create_success)
    TEST_CASE_REF(test_queue_create_nullpointer)
    TEST_CASE_REF(test_queue_create_invalidsize)
    TEST_CASE_REF(test_queue_create_nametaken)
    TEST_CASE_REF(test_queue_putget_success)
    TEST_CASE_REF(test_queue_get_empty)
    TEST_CASE_REF(test_queue_get_timeout)
    TEST_CASE_REF(test_queue_put_full)
    TEST_CASE_REF(test_queue_putget_multiple)
    TEST_CASE_REF(test_queue_getid_by_name_success)
    TEST_CASE_REF(test_queue_getid_by_name_not_found)
    TEST_CASE_REF(test_queue_delete_success)
    TEST_CASE_REF(test_queue_delete_invalidid)
TEST_SUITE_END(osal_queue, "osal", "OSAL")
