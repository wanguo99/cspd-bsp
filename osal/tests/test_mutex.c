/**
 * @file test_mutex.c
 * @brief OSAL互斥锁单元测试
 *
 * 使用新的libtest框架，测试自动注册
 */

#include "tests_framework.h"
#include "test_assert.h"
#include "test_registry.h"
#include "osal.h"

static int32_t shared_counter = 0;

/* 测试用例1: 互斥锁创建成功 */
TEST_CASE(test_mutex_create_success)
{
    osal_id_t mutex_id;

    int32_t ret = OSAL_MutexCreate(&mutex_id, "TEST_MUTEX", 0);

    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    TEST_ASSERT_NOT_EQUAL(OS_OBJECT_ID_UNDEFINED, mutex_id);

    OSAL_MutexDelete(mutex_id);
}

/* 测试用例2: 互斥锁创建失败 - 空指针 */
TEST_CASE(test_mutex_create_nullpointer)
{
    int32_t ret = OSAL_MutexCreate(NULL, "TEST", 0);
    TEST_ASSERT_EQUAL(OS_INVALID_POINTER, ret);
}

/* 测试用例3: 互斥锁创建失败 - 名称重复 */
TEST_CASE(test_mutex_create_nametaken)
{
    osal_id_t mutex_id1, mutex_id2;

    int32_t ret = OSAL_MutexCreate(&mutex_id1, "DUP_MUTEX", 0);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = OSAL_MutexCreate(&mutex_id2, "DUP_MUTEX", 0);
    TEST_ASSERT_EQUAL(OS_ERR_NAME_TAKEN, ret);

    OSAL_MutexDelete(mutex_id1);
}

/* 测试用例4: 互斥锁加锁解锁 */
TEST_CASE(test_mutex_lockunlock_success)
{
    osal_id_t mutex_id;
    OSAL_MutexCreate(&mutex_id, "TEST_MTX", 0);

    int32_t ret = OSAL_MutexLock(mutex_id);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = OSAL_MutexUnlock(mutex_id);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    OSAL_MutexDelete(mutex_id);
}

/* 测试用例5: 互斥锁加锁失败 - 无效ID */
TEST_CASE(test_mutex_lock_invalidid)
{
    int32_t ret = OSAL_MutexLock(9999);
    TEST_ASSERT_EQUAL(OS_ERR_INVALID_ID, ret);
}

/* 测试用例6: 互斥锁解锁失败 - 无效ID */
TEST_CASE(test_mutex_unlock_invalidid)
{
    int32_t ret = OSAL_MutexUnlock(9999);
    TEST_ASSERT_EQUAL(OS_ERR_INVALID_ID, ret);
}

/* 测试用例7: 根据名称获取互斥锁ID */
TEST_CASE(test_mutex_getid_by_name_success)
{
    osal_id_t mutex_id1, mutex_id2;

    OSAL_MutexCreate(&mutex_id1, "NAMED_MTX", 0);

    int32_t ret = OSAL_MutexGetIdByName(&mutex_id2, "NAMED_MTX");

    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    TEST_ASSERT_EQUAL(mutex_id1, mutex_id2);

    OSAL_MutexDelete(mutex_id1);
}

/* 测试用例8: 根据名称获取互斥锁ID - 未找到 */
TEST_CASE(test_mutex_getid_by_name_not_found)
{
    osal_id_t mutex_id;

    int32_t ret = OSAL_MutexGetIdByName(&mutex_id, "NONEXISTENT");
    TEST_ASSERT_EQUAL(OS_ERR_NAME_NOT_FOUND, ret);
}

/* 测试用例9: 互斥锁删除 */
TEST_CASE(test_mutex_delete_success)
{
    osal_id_t mutex_id;
    OSAL_MutexCreate(&mutex_id, "TEST_MTX", 0);

    int32_t ret = OSAL_MutexDelete(mutex_id);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
}

/* 测试用例10: 互斥锁删除失败 - 无效ID */
TEST_CASE(test_mutex_delete_invalidid)
{
    int32_t ret = OSAL_MutexDelete(9999);
    TEST_ASSERT_EQUAL(OS_ERR_INVALID_ID, ret);
}

/* 线程函数 - 用于测试互斥锁保护 */
static void* increment_thread(void *arg)
{
    osal_id_t *mutex_id = (osal_id_t *)arg;

    for (int32_t i = 0; i < 1000; i++) {
        OSAL_MutexLock(*mutex_id);
        shared_counter++;
        OSAL_MutexUnlock(*mutex_id);
    }

    return NULL;
}

/* 测试用例11: 互斥锁保护共享资源 */
TEST_CASE(test_mutex_protect_shared_resource)
{
    shared_counter = 0;
    osal_id_t mutex_id;
    OSAL_MutexCreate(&mutex_id, "PROTECT_MTX", 0);

    osal_thread_t thread1, thread2;

    /* 创建两个线程同时增加计数器 */
    OSAL_pthread_create(&thread1, NULL, increment_thread, &mutex_id);
    OSAL_pthread_create(&thread2, NULL, increment_thread, &mutex_id);

    /* 等待线程完成 */
    OSAL_pthread_join(thread1, NULL);
    OSAL_pthread_join(thread2, NULL);

    /* 验证计数器正确 */
    TEST_ASSERT_EQUAL(2000, shared_counter);

    OSAL_MutexDelete(mutex_id);
}

/* 注册测试套件 - 自动注册 */
TEST_SUITE_BEGIN(osal_mutex, "osal", "OSAL")
    TEST_CASE_REF(test_mutex_create_success)
    TEST_CASE_REF(test_mutex_create_nullpointer)
    TEST_CASE_REF(test_mutex_create_nametaken)
    TEST_CASE_REF(test_mutex_lockunlock_success)
    TEST_CASE_REF(test_mutex_lock_invalidid)
    TEST_CASE_REF(test_mutex_unlock_invalidid)
    TEST_CASE_REF(test_mutex_getid_by_name_success)
    TEST_CASE_REF(test_mutex_getid_by_name_not_found)
    TEST_CASE_REF(test_mutex_delete_success)
    TEST_CASE_REF(test_mutex_delete_invalidid)
    TEST_CASE_REF(test_mutex_protect_shared_resource)
TEST_SUITE_END(osal_mutex, "osal", "OSAL")
