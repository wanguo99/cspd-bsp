/************************************************************************
 * OSAL消息队列单元测试
 ************************************************************************/

#include "../test_framework.h"
#include "osal.h"
#include <string.h>

/* 测试前初始化 */
void setUp(void)
{
    OS_API_Init();
}

/* 测试后清理 */
void tearDown(void)
{
    OS_API_Teardown();
}

/* 测试用例1: 队列创建成功 */
void test_OS_QueueCreate_Success(void)
{
    osal_id_t queue_id;

    int32 ret = OS_QueueCreate(&queue_id, "TEST_QUEUE",
                               10, 64, 0);

    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    TEST_ASSERT_NOT_EQUAL(OS_OBJECT_ID_UNDEFINED, queue_id);

    OS_QueueDelete(queue_id);
}

/* 测试用例2: 队列创建失败 - 空指针 */
void test_OS_QueueCreate_NullPointer(void)
{
    int32 ret = OS_QueueCreate(NULL, "TEST", 10, 64, 0);
    TEST_ASSERT_EQUAL(OS_INVALID_POINTER, ret);
}

/* 测试用例3: 队列创建失败 - 无效大小 */
void test_OS_QueueCreate_InvalidSize(void)
{
    osal_id_t queue_id;

    /* 深度为0 */
    int32 ret = OS_QueueCreate(&queue_id, "TEST", 0, 64, 0);
    TEST_ASSERT_EQUAL(OS_QUEUE_INVALID_SIZE, ret);

    /* 消息大小为0 */
    ret = OS_QueueCreate(&queue_id, "TEST", 10, 0, 0);
    TEST_ASSERT_EQUAL(OS_QUEUE_INVALID_SIZE, ret);
}

/* 测试用例4: 队列创建失败 - 名称重复 */
void test_OS_QueueCreate_NameTaken(void)
{
    osal_id_t queue_id1, queue_id2;

    int32 ret = OS_QueueCreate(&queue_id1, "DUP_QUEUE", 10, 64, 0);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = OS_QueueCreate(&queue_id2, "DUP_QUEUE", 10, 64, 0);
    TEST_ASSERT_EQUAL(OS_ERR_NAME_TAKEN, ret);

    OS_QueueDelete(queue_id1);
}

/* 测试用例5: 队列发送和接收 */
void test_OS_QueuePutGet_Success(void)
{
    osal_id_t queue_id;
    OS_QueueCreate(&queue_id, "TEST_Q", 10, 64, 0);

    uint8 send_data[64];
    uint8 recv_data[64];
    uint32 size_copied;

    strcpy((char *)send_data, "Hello World");

    /* 发送消息 */
    int32 ret = OS_QueuePut(queue_id, send_data, 64, 0);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 接收消息 */
    ret = OS_QueueGet(queue_id, recv_data, 64, &size_copied, 1000);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    TEST_ASSERT_EQUAL(64, size_copied);
    TEST_ASSERT_EQUAL_STRING("Hello World", (char *)recv_data);

    OS_QueueDelete(queue_id);
}

/* 测试用例6: 队列接收 - 空队列非阻塞 */
void test_OS_QueueGet_Empty(void)
{
    osal_id_t queue_id;
    OS_QueueCreate(&queue_id, "TEST_Q", 10, 64, 0);

    uint8 data[64];
    uint32 size;

    int32 ret = OS_QueueGet(queue_id, data, 64, &size, OS_CHECK);
    TEST_ASSERT_EQUAL(OS_QUEUE_EMPTY, ret);

    OS_QueueDelete(queue_id);
}

/* 测试用例7: 队列接收 - 超时 */
void test_OS_QueueGet_Timeout(void)
{
    osal_id_t queue_id;
    OS_QueueCreate(&queue_id, "TEST_Q", 10, 64, 0);

    uint8 data[64];
    uint32 size;
    uint32 start = OS_GetTickCount();

    int32 ret = OS_QueueGet(queue_id, data, 64, &size, 500);

    uint32 elapsed = OS_GetTickCount() - start;

    TEST_ASSERT_EQUAL(OS_QUEUE_TIMEOUT, ret);
    TEST_ASSERT_GREATER_OR_EQUAL(450, elapsed);
    TEST_ASSERT_LESS_OR_EQUAL(550, elapsed);

    (void)data;  /* 未使用，仅用于测试超时 */

    OS_QueueDelete(queue_id);
}

/* 测试用例8: 队列满 */
void test_OS_QueuePut_Full(void)
{
    osal_id_t queue_id;
    OS_QueueCreate(&queue_id, "TEST_Q", 2, 64, 0);

    uint8 data[64] = "Test";

    /* 填满队列 */
    TEST_ASSERT_EQUAL(OS_SUCCESS, OS_QueuePut(queue_id, data, 64, 0));
    TEST_ASSERT_EQUAL(OS_SUCCESS, OS_QueuePut(queue_id, data, 64, 0));

    /* 队列已满，这个操作会阻塞，我们不测试阻塞情况 */

    OS_QueueDelete(queue_id);
}

/* 测试用例9: 多消息发送接收 */
void test_OS_QueuePutGet_Multiple(void)
{
    osal_id_t queue_id;
    OS_QueueCreate(&queue_id, "TEST_Q", 5, 64, 0);

    uint8 send_data[5][64];
    uint8 recv_data[64];
    uint32 size;

    /* 发送5条消息 */
    for (int i = 0; i < 5; i++) {
        sprintf((char *)send_data[i], "Message %d", i);
        TEST_ASSERT_EQUAL(OS_SUCCESS,
                         OS_QueuePut(queue_id, send_data[i], 64, 0));
    }

    /* 接收5条消息 */
    for (int i = 0; i < 5; i++) {
        TEST_ASSERT_EQUAL(OS_SUCCESS,
                         OS_QueueGet(queue_id, recv_data, 64, &size, 1000));

        char expected[64];
        sprintf(expected, "Message %d", i);
        TEST_ASSERT_EQUAL_STRING(expected, (char *)recv_data);
    }

    OS_QueueDelete(queue_id);
}

/* 测试用例10: 根据名称获取队列ID */
void test_OS_QueueGetIdByName_Success(void)
{
    osal_id_t queue_id1, queue_id2;

    OS_QueueCreate(&queue_id1, "NAMED_Q", 10, 64, 0);

    int32 ret = OS_QueueGetIdByName(&queue_id2, "NAMED_Q");

    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    TEST_ASSERT_EQUAL(queue_id1, queue_id2);

    OS_QueueDelete(queue_id1);
}

/* 测试用例11: 根据名称获取队列ID - 未找到 */
void test_OS_QueueGetIdByName_NotFound(void)
{
    osal_id_t queue_id;

    int32 ret = OS_QueueGetIdByName(&queue_id, "NONEXISTENT");
    TEST_ASSERT_EQUAL(OS_ERR_NAME_NOT_FOUND, ret);
}

/* 测试用例12: 队列删除 */
void test_OS_QueueDelete_Success(void)
{
    osal_id_t queue_id;
    OS_QueueCreate(&queue_id, "TEST_Q", 10, 64, 0);

    int32 ret = OS_QueueDelete(queue_id);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
}

/* 测试用例13: 队列删除 - 无效ID */
void test_OS_QueueDelete_InvalidId(void)
{
    int32 ret = OS_QueueDelete(9999);
    TEST_ASSERT_EQUAL(OS_ERR_INVALID_ID, ret);
}

/* 主测试运行器 */
int main(void)
{
    TEST_BEGIN();

    RUN_TEST(test_OS_QueueCreate_Success);
    RUN_TEST(test_OS_QueueCreate_NullPointer);
    RUN_TEST(test_OS_QueueCreate_InvalidSize);
    RUN_TEST(test_OS_QueueCreate_NameTaken);
    RUN_TEST(test_OS_QueuePutGet_Success);
    RUN_TEST(test_OS_QueueGet_Empty);
    RUN_TEST(test_OS_QueueGet_Timeout);
    RUN_TEST(test_OS_QueuePut_Full);
    RUN_TEST(test_OS_QueuePutGet_Multiple);
    RUN_TEST(test_OS_QueueGetIdByName_Success);
    RUN_TEST(test_OS_QueueGetIdByName_NotFound);
    RUN_TEST(test_OS_QueueDelete_Success);
    RUN_TEST(test_OS_QueueDelete_InvalidId);

    TEST_END();
}
