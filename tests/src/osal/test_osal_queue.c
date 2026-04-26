/************************************************************************
 * OSAL消息队列单元测试
 ************************************************************************/

#include "test_framework.h"
#ifndef STANDALONE_TEST
#include "test_runner.h"
#endif
#include "osal.h"

/* 测试前初始化 */
__attribute__((unused)) static void setUp(void)
{
    
}

/* 测试后清理 */
__attribute__((unused)) static void tearDown(void)
{
    
}

/* 测试用例1: 队列创建成功 */
void test_osal_queue_create_success(void)
{
    setUp();
    osal_id_t queue_id;

    int32_t ret = OSAL_QueueCreate(&queue_id, "TEST_QUEUE",
                               10, 64, 0);

    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    TEST_ASSERT_NOT_EQUAL(OS_OBJECT_ID_UNDEFINED, queue_id);

    OSAL_QueueDelete(queue_id);
    tearDown();
}

/* 测试用例2: 队列创建失败 - 空指针 */
void test_osal_queue_create_nullpointer(void)
{
    setUp();
    int32_t ret = OSAL_QueueCreate(NULL, "TEST", 10, 64, 0);
    TEST_ASSERT_EQUAL(OS_INVALID_POINTER, ret);
    tearDown();
}

/* 测试用例3: 队列创建失败 - 无效大小 */
void test_osal_queue_create_invalidsize(void)
{
    setUp();
    osal_id_t queue_id;

    /* 深度为0 */
    int32_t ret = OSAL_QueueCreate(&queue_id, "TEST", 0, 64, 0);
    TEST_ASSERT_EQUAL(OS_QUEUE_INVALID_SIZE, ret);

    /* 消息大小为0 */
    ret = OSAL_QueueCreate(&queue_id, "TEST", 10, 0, 0);
    TEST_ASSERT_EQUAL(OS_QUEUE_INVALID_SIZE, ret);
    tearDown();
}

/* 测试用例4: 队列创建失败 - 名称重复 */
void test_osal_queue_create_nametaken(void)
{
    setUp();
    osal_id_t queue_id1, queue_id2;

    int32_t ret = OSAL_QueueCreate(&queue_id1, "DUP_QUEUE", 10, 64, 0);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = OSAL_QueueCreate(&queue_id2, "DUP_QUEUE", 10, 64, 0);
    TEST_ASSERT_EQUAL(OS_ERR_NAME_TAKEN, ret);

    OSAL_QueueDelete(queue_id1);
    tearDown();
}

/* 测试用例5: 队列发送和接收 */
void test_osal_queue_putget_Success(void)
{
    setUp();
    osal_id_t queue_id;
    OSAL_QueueCreate(&queue_id, "TEST_Q", 10, 64, 0);

    uint8_t send_data[64];
    uint8_t recv_data[64];
    uint32_t size_copied;

    OSAL_Strcpy((char *)send_data, "Hello World");

    /* 发送消息 */
    int32_t ret = OSAL_QueuePut(queue_id, send_data, 64, 0);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 接收消息 */
    ret = OSAL_QueueGet(queue_id, recv_data, 64, &size_copied, 1000);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    TEST_ASSERT_EQUAL(64, size_copied);
    TEST_ASSERT_EQUAL_STRING("Hello World", (char *)recv_data);

    OSAL_QueueDelete(queue_id);
    tearDown();
}

/* 测试用例6: 队列接收 - 空队列非阻塞 */
void test_osal_queue_get_empty(void)
{
    setUp();
    osal_id_t queue_id;
    OSAL_QueueCreate(&queue_id, "TEST_Q", 10, 64, 0);

    uint8_t data[64];
    uint32_t size;

    int32_t ret = OSAL_QueueGet(queue_id, data, 64, &size, OS_CHECK);
    TEST_ASSERT_EQUAL(OS_QUEUE_EMPTY, ret);

    OSAL_QueueDelete(queue_id);
    tearDown();
}

/* 测试用例7: 队列接收 - 超时 */
void test_osal_queue_get_timeout(void)
{
    setUp();
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

    (void)data;  /* 未使用，仅用于测试超时 */

    OSAL_QueueDelete(queue_id);
    tearDown();
}

/* 测试用例8: 队列满 */
void test_osal_queue_put_full(void)
{
    setUp();
    osal_id_t queue_id;
    OSAL_QueueCreate(&queue_id, "TEST_Q", 2, 64, 0);

    uint8_t data[64] = "Test";

    /* 填满队列 */
    TEST_ASSERT_EQUAL(OS_SUCCESS, OSAL_QueuePut(queue_id, data, 64, 0));
    TEST_ASSERT_EQUAL(OS_SUCCESS, OSAL_QueuePut(queue_id, data, 64, 0));

    /* 队列已满，这个操作会阻塞，我们不测试阻塞情况 */

    OSAL_QueueDelete(queue_id);
    tearDown();
}

/* 测试用例9: 多消息发送接收 */
void test_osal_queue_putget_Multiple(void)
{
    setUp();
    osal_id_t queue_id;
    OSAL_QueueCreate(&queue_id, "TEST_Q", 5, 64, 0);

    uint8_t send_data[5][64];
    uint8_t recv_data[64];
    uint32_t size;

    /* 发送5条消息 */
    for (int i = 0; i < 5; i++) {
        OSAL_Sprintf((char *)send_data[i], "Message %d", i);
        TEST_ASSERT_EQUAL(OS_SUCCESS,
                         OSAL_QueuePut(queue_id, send_data[i], 64, 0));
    }

    /* 接收5条消息 */
    for (int i = 0; i < 5; i++) {
        TEST_ASSERT_EQUAL(OS_SUCCESS,
                         OSAL_QueueGet(queue_id, recv_data, 64, &size, 1000));

        str_t expected[64];
        OSAL_Sprintf(expected, "Message %d", i);
        TEST_ASSERT_EQUAL_STRING(expected, (char *)recv_data);
    }

    OSAL_QueueDelete(queue_id);
    tearDown();
}

/* 测试用例10: 根据名称获取队列ID */
void test_osal_queue_getidByName_Success(void)
{
    setUp();
    osal_id_t queue_id1, queue_id2;

    OSAL_QueueCreate(&queue_id1, "NAMED_Q", 10, 64, 0);

    int32_t ret = OSAL_QueueGetIdByName(&queue_id2, "NAMED_Q");

    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    TEST_ASSERT_EQUAL(queue_id1, queue_id2);

    OSAL_QueueDelete(queue_id1);
    tearDown();
}

/* 测试用例11: 根据名称获取队列ID - 未找到 */
void test_osal_queue_getidByName_NotFound(void)
{
    setUp();
    osal_id_t queue_id;

    int32_t ret = OSAL_QueueGetIdByName(&queue_id, "NONEXISTENT");
    TEST_ASSERT_EQUAL(OS_ERR_NAME_NOT_FOUND, ret);
    tearDown();
}

/* 测试用例12: 队列删除 */
void test_osal_queue_delete_success(void)
{
    setUp();
    osal_id_t queue_id;
    OSAL_QueueCreate(&queue_id, "TEST_Q", 10, 64, 0);

    int32_t ret = OSAL_QueueDelete(queue_id);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    tearDown();
}

/* 测试用例13: 队列删除 - 无效ID */
void test_osal_queue_delete_invalidid(void)
{
    setUp();
    int32_t ret = OSAL_QueueDelete(9999);
    TEST_ASSERT_EQUAL(OS_ERR_INVALID_ID, ret);
    tearDown();
}

/* 模块注册 */
#include "test_runner.h"

TEST_MODULE_BEGIN(test_osal_queue)
    TEST_CASE(test_osal_queue_create_success)
    TEST_CASE(test_osal_queue_create_nullpointer)
    TEST_CASE(test_osal_queue_create_invalidsize)
    TEST_CASE(test_osal_queue_create_nametaken)
    TEST_CASE(test_osal_queue_putget_Success)
    TEST_CASE(test_osal_queue_get_empty)
    TEST_CASE(test_osal_queue_get_timeout)
    TEST_CASE(test_osal_queue_put_full)
    TEST_CASE(test_osal_queue_putget_Multiple)
    TEST_CASE(test_osal_queue_getidByName_Success)
    TEST_CASE(test_osal_queue_getidByName_NotFound)
    TEST_CASE(test_osal_queue_delete_success)
    TEST_CASE(test_osal_queue_delete_invalidid)
TEST_MODULE_END(test_osal_queue)

/* 独立运行时的主函数 */
#ifdef STANDALONE_TEST
int main(void)
{
    TEST_BEGIN();

    RUN_TEST(test_OSAL_QueueCreate_Success);
    RUN_TEST(test_OSAL_QueueCreate_NullPointer);
    RUN_TEST(test_OSAL_QueueCreate_InvalidSize);
    RUN_TEST(test_OSAL_QueueCreate_NameTaken);
    RUN_TEST(test_OSAL_QueuePutGet_Success);
    RUN_TEST(test_OSAL_QueueGet_Empty);
    RUN_TEST(test_OSAL_QueueGet_Timeout);
    RUN_TEST(test_OSAL_QueuePut_Full);
    RUN_TEST(test_OSAL_QueuePutGet_Multiple);
    RUN_TEST(test_OSAL_QueueGetIdByName_Success);
    RUN_TEST(test_OSAL_QueueGetIdByName_NotFound);
    RUN_TEST(test_OSAL_QueueDelete_Success);
    RUN_TEST(test_OSAL_QueueDelete_InvalidId);

    TEST_END();
}
#endif
