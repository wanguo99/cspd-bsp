/************************************************************************
 * OSAL互斥锁单元测试
 ************************************************************************/

#include "../core/unittest_framework.h"
#ifndef STANDALONE_TEST
#include "../core/unittest_runner.h"
#endif
#include "osal.h"
#include <pthread.h>

static int shared_counter = 0;

/* 测试前初始化 */
__attribute__((unused)) static void setUp(void)
{
    OS_API_Init();
    shared_counter = 0;
}

/* 测试后清理 */
__attribute__((unused)) static void tearDown(void)
{
    OS_API_Teardown();
}

/* 测试用例1: 互斥锁创建成功 */
void test_OS_MutexCreate_Success(void)
{
    setUp();
    osal_id_t mutex_id;

    int32 ret = OS_MutexCreate(&mutex_id, "TEST_MUTEX", 0);

    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    TEST_ASSERT_NOT_EQUAL(OS_OBJECT_ID_UNDEFINED, mutex_id);

    OS_MutexDelete(mutex_id);
    tearDown();
}

/* 测试用例2: 互斥锁创建失败 - 空指针 */
void test_OS_MutexCreate_NullPointer(void)
{
    setUp();
    int32 ret = OS_MutexCreate(NULL, "TEST", 0);
    TEST_ASSERT_EQUAL(OS_INVALID_POINTER, ret);
    tearDown();
}

/* 测试用例3: 互斥锁创建失败 - 名称重复 */
void test_OS_MutexCreate_NameTaken(void)
{
    setUp();
    osal_id_t mutex_id1, mutex_id2;

    int32 ret = OS_MutexCreate(&mutex_id1, "DUP_MUTEX", 0);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = OS_MutexCreate(&mutex_id2, "DUP_MUTEX", 0);
    TEST_ASSERT_EQUAL(OS_ERR_NAME_TAKEN, ret);

    OS_MutexDelete(mutex_id1);
    tearDown();
}

/* 测试用例4: 互斥锁加锁解锁 */
void test_OS_MutexLockUnlock_Success(void)
{
    setUp();
    osal_id_t mutex_id;
    OS_MutexCreate(&mutex_id, "TEST_MTX", 0);

    int32 ret = OS_MutexLock(mutex_id);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = OS_MutexUnlock(mutex_id);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    OS_MutexDelete(mutex_id);
    tearDown();
}

/* 测试用例5: 互斥锁加锁失败 - 无效ID */
void test_OS_MutexLock_InvalidId(void)
{
    setUp();
    int32 ret = OS_MutexLock(9999);
    TEST_ASSERT_EQUAL(OS_ERR_INVALID_ID, ret);
    tearDown();
}

/* 测试用例6: 互斥锁解锁失败 - 无效ID */
void test_OS_MutexUnlock_InvalidId(void)
{
    setUp();
    int32 ret = OS_MutexUnlock(9999);
    TEST_ASSERT_EQUAL(OS_ERR_INVALID_ID, ret);
    tearDown();
}

/* 测试用例7: 根据名称获取互斥锁ID */
void test_OS_MutexGetIdByName_Success(void)
{
    setUp();
    osal_id_t mutex_id1, mutex_id2;

    OS_MutexCreate(&mutex_id1, "NAMED_MTX", 0);

    int32 ret = OS_MutexGetIdByName(&mutex_id2, "NAMED_MTX");

    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    TEST_ASSERT_EQUAL(mutex_id1, mutex_id2);

    OS_MutexDelete(mutex_id1);
    tearDown();
}

/* 测试用例8: 根据名称获取互斥锁ID - 未找到 */
void test_OS_MutexGetIdByName_NotFound(void)
{
    setUp();
    osal_id_t mutex_id;

    int32 ret = OS_MutexGetIdByName(&mutex_id, "NONEXISTENT");
    TEST_ASSERT_EQUAL(OS_ERR_NAME_NOT_FOUND, ret);
    tearDown();
}

/* 测试用例9: 互斥锁删除 */
void test_OS_MutexDelete_Success(void)
{
    setUp();
    osal_id_t mutex_id;
    OS_MutexCreate(&mutex_id, "TEST_MTX", 0);

    int32 ret = OS_MutexDelete(mutex_id);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    tearDown();
}

/* 测试用例10: 互斥锁删除失败 - 无效ID */
void test_OS_MutexDelete_InvalidId(void)
{
    setUp();
    int32 ret = OS_MutexDelete(9999);
    TEST_ASSERT_EQUAL(OS_ERR_INVALID_ID, ret);
    tearDown();
}

/* 线程函数 - 用于测试互斥锁保护 */
static void* increment_thread(void *arg)
{
    osal_id_t *mutex_id = (osal_id_t *)arg;

    for (int i = 0; i < 1000; i++) {
        OS_MutexLock(*mutex_id);
        shared_counter++;
        OS_MutexUnlock(*mutex_id);
    }

    return NULL;
}

/* 测试用例11: 互斥锁保护共享资源 */
void test_OS_Mutex_ProtectSharedResource(void)
{
    setUp();
    osal_id_t mutex_id;
    OS_MutexCreate(&mutex_id, "PROTECT_MTX", 0);

    pthread_t thread1, thread2;

    /* 创建两个线程同时增加计数器 */
    pthread_create(&thread1, NULL, increment_thread, &mutex_id);
    pthread_create(&thread2, NULL, increment_thread, &mutex_id);

    /* 等待线程完成 */
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    /* 验证计数器正确 */
    TEST_ASSERT_EQUAL(2000, shared_counter);

    OS_MutexDelete(mutex_id);
    tearDown();
}

/* 模块注册 */
#include "../core/unittest_runner.h"

TEST_MODULE_BEGIN(test_os_mutex)
    TEST_CASE(test_OS_MutexCreate_Success)
    TEST_CASE(test_OS_MutexCreate_NullPointer)
    TEST_CASE(test_OS_MutexCreate_NameTaken)
    TEST_CASE(test_OS_MutexLockUnlock_Success)
    TEST_CASE(test_OS_MutexLock_InvalidId)
    TEST_CASE(test_OS_MutexUnlock_InvalidId)
    TEST_CASE(test_OS_MutexGetIdByName_Success)
    TEST_CASE(test_OS_MutexGetIdByName_NotFound)
    TEST_CASE(test_OS_MutexDelete_Success)
    TEST_CASE(test_OS_MutexDelete_InvalidId)
    TEST_CASE(test_OS_Mutex_ProtectSharedResource)
TEST_MODULE_END(test_os_mutex)

/* 独立运行时的主函数 */
#ifdef STANDALONE_TEST
int main(void)
{
    TEST_BEGIN();

    RUN_TEST(test_OS_MutexCreate_Success);
    RUN_TEST(test_OS_MutexCreate_NullPointer);
    RUN_TEST(test_OS_MutexCreate_NameTaken);
    RUN_TEST(test_OS_MutexLockUnlock_Success);
    RUN_TEST(test_OS_MutexLock_InvalidId);
    RUN_TEST(test_OS_MutexUnlock_InvalidId);
    RUN_TEST(test_OS_MutexGetIdByName_Success);
    RUN_TEST(test_OS_MutexGetIdByName_NotFound);
    RUN_TEST(test_OS_MutexDelete_Success);
    RUN_TEST(test_OS_MutexDelete_InvalidId);
    RUN_TEST(test_OS_Mutex_ProtectSharedResource);

    TEST_END();
}
#endif
