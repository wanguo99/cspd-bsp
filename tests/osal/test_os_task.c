/************************************************************************
 * OSAL任务管理单元测试
 ************************************************************************/

#include "../core/unittest_framework.h"
#ifndef STANDALONE_TEST
#include "../core/unittest_runner.h"
#endif
#include "osal.h"
#include <unistd.h>

/* 测试任务函数 */
static void test_task_func(void *arg)
{
    int *counter = (int *)arg;
    if (counter == NULL) {
        /* 如果没有传递计数器，只是简单地延时后退出 */
        OS_TaskDelay(100);
        return;
    }
    while (*counter < 10) {
        (*counter)++;
        OS_TaskDelay(100);
    }
}

/* 测试前初始化 */
__attribute__((unused)) static void setUp(void)
{
    OS_API_Init();
}

/* 测试后清理 */
__attribute__((unused)) static void tearDown(void)
{
    OS_API_Teardown();
}

/* 测试用例1: 任务创建成功 */
void test_OS_TaskCreate_Success(void)
{
    setUp();
    osal_id_t task_id;
    int counter = 0;

    int32 ret = OS_TaskCreate(&task_id, "TEST_TASK",
                              test_task_func, (uint32 *)&counter,
                              32 * 1024, 100, 0);

    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    TEST_ASSERT_NOT_EQUAL(OS_OBJECT_ID_UNDEFINED, task_id);

    /* 等待任务执行 */
    OS_TaskDelay(1500);
    TEST_ASSERT_EQUAL(10, counter);

    OS_TaskDelete(task_id);
    tearDown();
}

/* 测试用例2: 任务创建失败 - 空指针 */
void test_OS_TaskCreate_NullPointer(void)
{
    setUp();
    int32 ret = OS_TaskCreate(NULL, "TEST", test_task_func,
                              NULL, 4096, 100, 0);
    TEST_ASSERT_EQUAL(OS_INVALID_POINTER, ret);
    tearDown();
}

/* 测试用例3: 任务创建失败 - 名称过长 */
void test_OS_TaskCreate_NameTooLong(void)
{
    setUp();
    osal_id_t task_id;
    char long_name[OS_MAX_API_NAME + 10];
    memset(long_name, 'A', sizeof(long_name));
    long_name[sizeof(long_name) - 1] = '\0';

    int32 ret = OS_TaskCreate(&task_id, long_name,
                              test_task_func, NULL,
                              4096, 100, 0);
    TEST_ASSERT_EQUAL(OS_ERR_NAME_TOO_LONG, ret);
    tearDown();
}

/* 测试用例4: 任务创建失败 - 无效优先级 */
void test_OS_TaskCreate_InvalidPriority(void)
{
    setUp();
    osal_id_t task_id;

    int32 ret = OS_TaskCreate(&task_id, "TEST", test_task_func,
                              NULL, 4096, 0, 0);  /* 优先级0无效 */
    TEST_ASSERT_EQUAL(OS_ERR_INVALID_PRIORITY, ret);

    ret = OS_TaskCreate(&task_id, "TEST", test_task_func,
                        NULL, 4096, 300, 0);  /* 优先级300无效 */
    TEST_ASSERT_EQUAL(OS_ERR_INVALID_PRIORITY, ret);
    tearDown();
}

/* 测试用例5: 任务创建失败 - 名称重复 */
void test_OS_TaskCreate_NameTaken(void)
{
    setUp();
    osal_id_t task_id1, task_id2;

    int32 ret = OS_TaskCreate(&task_id1, "DUPLICATE", test_task_func,
                              NULL, 4096, 100, 0);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = OS_TaskCreate(&task_id2, "DUPLICATE", test_task_func,
                        NULL, 4096, 100, 0);
    TEST_ASSERT_EQUAL(OS_ERR_NAME_TAKEN, ret);

    OS_TaskDelete(task_id1);
    tearDown();
}

/* 测试用例6: 任务删除成功 */
void test_OS_TaskDelete_Success(void)
{
    setUp();
    osal_id_t task_id;
    int counter = 0;

    OS_TaskCreate(&task_id, "TEST", test_task_func,
                  (uint32 *)&counter, 4096, 100, 0);

    int32 ret = OS_TaskDelete(task_id);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    tearDown();
}

/* 测试用例7: 任务删除失败 - 无效ID */
void test_OS_TaskDelete_InvalidId(void)
{
    setUp();
    int32 ret = OS_TaskDelete(9999);
    TEST_ASSERT_EQUAL(OS_ERR_INVALID_ID, ret);
    tearDown();
}

/* 测试用例8: 任务延时 */
void test_OS_TaskDelay_Success(void)
{
    setUp();
    uint32 start = OS_GetTickCount();

    int32 ret = OS_TaskDelay(500);

    uint32 elapsed = OS_GetTickCount() - start;

    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    TEST_ASSERT_GREATER_OR_EQUAL(450, elapsed);  /* 允许50ms误差 */
    tearDown();
}

/* 测试用例9: 获取当前任务ID */
void test_OS_TaskGetId_Success(void)
{
    setUp();
    osal_id_t task_id = OS_TaskGetId();
    /* 主线程应该返回有效ID或UNDEFINED */
    TEST_ASSERT_TRUE(task_id == OS_OBJECT_ID_UNDEFINED || task_id > 0);
    tearDown();
}

/* 测试用例10: 根据名称获取任务ID */
void test_OS_TaskGetIdByName_Success(void)
{
    setUp();
    osal_id_t task_id1, task_id2;

    OS_TaskCreate(&task_id1, "NAMED_TASK", test_task_func,
                  NULL, 4096, 100, 0);

    int32 ret = OS_TaskGetIdByName(&task_id2, "NAMED_TASK");

    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    TEST_ASSERT_EQUAL(task_id1, task_id2);

    OS_TaskDelete(task_id1);
    tearDown();
}

/* 测试用例11: 根据名称获取任务ID - 未找到 */
void test_OS_TaskGetIdByName_NotFound(void)
{
    setUp();
    osal_id_t task_id;

    int32 ret = OS_TaskGetIdByName(&task_id, "NONEXISTENT");
    TEST_ASSERT_EQUAL(OS_ERR_NAME_NOT_FOUND, ret);
    tearDown();
}

/* 测试用例12: 设置任务优先级 */
void test_OS_TaskSetPriority_Success(void)
{
    setUp();
    osal_id_t task_id;

    OS_TaskCreate(&task_id, "TEST", test_task_func,
                  NULL, 4096, 100, 0);

    int32 ret = OS_TaskSetPriority(task_id, 150);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    OS_TaskDelete(task_id);
    tearDown();
}

/* 测试用例13: 获取任务信息 */
void test_OS_TaskGetInfo_Success(void)
{
    setUp();
    osal_id_t task_id;
    OS_TaskProp_t task_prop;

    OS_TaskCreate(&task_id, "TEST", test_task_func,
                  NULL, 65536, 120, 0);

    int32 ret = OS_TaskGetInfo(task_id, &task_prop);

    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    TEST_ASSERT_EQUAL(120, task_prop.priority);
    TEST_ASSERT_EQUAL(65536, task_prop.stack_size);

    OS_TaskDelete(task_id);
    tearDown();
}

/* 模块注册 */
#include "../core/unittest_runner.h"

TEST_MODULE_BEGIN(test_os_task)
    TEST_CASE(test_OS_TaskCreate_Success)
    TEST_CASE(test_OS_TaskCreate_NullPointer)
    TEST_CASE(test_OS_TaskCreate_NameTooLong)
    TEST_CASE(test_OS_TaskCreate_InvalidPriority)
    TEST_CASE(test_OS_TaskCreate_NameTaken)
    TEST_CASE(test_OS_TaskDelete_Success)
    TEST_CASE(test_OS_TaskDelete_InvalidId)
    TEST_CASE(test_OS_TaskDelay_Success)
    TEST_CASE(test_OS_TaskGetId_Success)
    TEST_CASE(test_OS_TaskGetIdByName_Success)
    TEST_CASE(test_OS_TaskGetIdByName_NotFound)
    TEST_CASE(test_OS_TaskSetPriority_Success)
    TEST_CASE(test_OS_TaskGetInfo_Success)
TEST_MODULE_END(test_os_task)

/* 独立运行时的主函数 */
#ifdef STANDALONE_TEST
int main(void)
{
    TEST_BEGIN();

    RUN_TEST(test_OS_TaskCreate_Success);
    RUN_TEST(test_OS_TaskCreate_NullPointer);
    RUN_TEST(test_OS_TaskCreate_NameTooLong);
    RUN_TEST(test_OS_TaskCreate_InvalidPriority);
    RUN_TEST(test_OS_TaskCreate_NameTaken);
    RUN_TEST(test_OS_TaskDelete_Success);
    RUN_TEST(test_OS_TaskDelete_InvalidId);
    RUN_TEST(test_OS_TaskDelay_Success);
    RUN_TEST(test_OS_TaskGetId_Success);
    RUN_TEST(test_OS_TaskGetIdByName_Success);
    RUN_TEST(test_OS_TaskGetIdByName_NotFound);
    RUN_TEST(test_OS_TaskSetPriority_Success);
    RUN_TEST(test_OS_TaskGetInfo_Success);

    TEST_END();
}
#endif
