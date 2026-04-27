/**
 * @file test_task.c
 * @brief OSAL任务管理单元测试
 *
 * 使用新的libtest框架，测试自动注册
 */

#include "libutest.h"
#include "test_assert.h"
#include "test_registry.h"
#include "osal.h"

/* 测试任务函数 */
static void test_task_func(void *arg)
{
    int32_t *counter = (int32_t *)arg;
    if (NULL == counter) {
        OSAL_TaskDelay(100);
        return;
    }
    while (*counter < 10 && !OSAL_TaskShouldShutdown()) {
        (*counter)++;
        OSAL_TaskDelay(100);
    }
}

/* 测试用例1: 任务创建成功 */
TEST_CASE(test_task_create_success)
{
    osal_id_t task_id;
    int32_t counter = 0;

    int32_t ret = OSAL_TaskCreate(&task_id, "TEST_TASK",
                                 test_task_func, (uint32_t *)&counter,
                                 32 * 1024, 100, 0);

    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    TEST_ASSERT_NOT_EQUAL(OS_OBJECT_ID_UNDEFINED, task_id);

    /* 等待任务执行 */
    OSAL_TaskDelay(1500);
    TEST_ASSERT_EQUAL(10, counter);

    OSAL_TaskDelete(task_id);
}

/* 测试用例2: 任务创建失败 - 空指针 */
TEST_CASE(test_task_create_null_pointer)
{
    int32_t ret = OSAL_TaskCreate(NULL, "TEST", test_task_func,
                                 NULL, 4096, 100, 0);
    TEST_ASSERT_EQUAL(OS_INVALID_POINTER, ret);
}

/* 测试用例3: 任务创建失败 - 名称过长 */
TEST_CASE(test_task_create_name_too_long)
{
    osal_id_t task_id;
    str_t long_name[OS_MAX_API_NAME + 10];
    OSAL_Memset(long_name, 'A', sizeof(long_name));
    long_name[sizeof(long_name) - 1] = '\0';

    int32_t ret = OSAL_TaskCreate(&task_id, long_name,
                                 test_task_func, NULL,
                                 4096, 100, 0);
    TEST_ASSERT_EQUAL(OS_ERR_NAME_TOO_LONG, ret);
}

/* 测试用例4: 任务创建失败 - 无效优先级 */
TEST_CASE(test_task_create_invalid_priority)
{
    osal_id_t task_id;

    int32_t ret = OSAL_TaskCreate(&task_id, "TEST", test_task_func,
                                 NULL, 4096, 0, 0);
    TEST_ASSERT_EQUAL(OS_ERR_INVALID_PRIORITY, ret);

    ret = OSAL_TaskCreate(&task_id, "TEST2", test_task_func,
                          NULL, 4096, 300, 0);
    TEST_ASSERT_EQUAL(OS_ERR_INVALID_PRIORITY, ret);
}

/* 测试用例5: 任务创建失败 - 名称重复 */
TEST_CASE(test_task_create_name_taken)
{
    osal_id_t task_id1, task_id2;

    int32_t ret = OSAL_TaskCreate(&task_id1, "DUPLICATE", test_task_func,
                                 NULL, 4096, 100, 0);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = OSAL_TaskCreate(&task_id2, "DUPLICATE", test_task_func,
                          NULL, 4096, 100, 0);
    TEST_ASSERT_EQUAL(OS_ERR_NAME_TAKEN, ret);

    OSAL_TaskDelete(task_id1);
}

/* 测试用例6: 任务删除成功 */
TEST_CASE(test_task_delete_success)
{
    osal_id_t task_id;
    int32_t counter = 0;

    OSAL_TaskCreate(&task_id, "TEST", test_task_func,
                    (uint32_t *)&counter, 4096, 100, 0);

    int32_t ret = OSAL_TaskDelete(task_id);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
}

/* 测试用例7: 任务删除失败 - 无效ID */
TEST_CASE(test_task_delete_invalid_id)
{
    int32_t ret = OSAL_TaskDelete(9999);
    TEST_ASSERT_EQUAL(OS_ERR_INVALID_ID, ret);
}

/* 测试用例8: 任务延时 */
TEST_CASE(test_task_delay_success)
{
    uint32_t start = OS_GetTickCount();

    int32_t ret = OSAL_TaskDelay(500);

    uint32_t elapsed = OS_GetTickCount() - start;

    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    TEST_ASSERT_GREATER_OR_EQUAL(450, elapsed);
}

/* 测试用例9: 获取当前任务ID */
TEST_CASE(test_task_get_id_success)
{
    osal_id_t task_id = OSAL_TaskGetId();
    TEST_ASSERT_TRUE(task_id == OS_OBJECT_ID_UNDEFINED || task_id > 0);
}

/* 测试用例10: 根据名称获取任务ID */
TEST_CASE(test_task_get_id_by_name_success)
{
    osal_id_t task_id1, task_id2;

    OSAL_TaskCreate(&task_id1, "NAMED_TASK", test_task_func,
                    NULL, 4096, 100, 0);

    int32_t ret = OSAL_TaskGetIdByName(&task_id2, "NAMED_TASK");

    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    TEST_ASSERT_EQUAL(task_id1, task_id2);

    OSAL_TaskDelete(task_id1);
}

/* 测试用例11: 根据名称获取任务ID - 未找到 */
TEST_CASE(test_task_get_id_by_name_not_found)
{
    osal_id_t task_id;

    int32_t ret = OSAL_TaskGetIdByName(&task_id, "NONEXISTENT");
    TEST_ASSERT_EQUAL(OS_ERR_NAME_NOT_FOUND, ret);
}

/* 测试用例12: 设置任务优先级 */
TEST_CASE(test_task_set_priority_success)
{
    osal_id_t task_id;

    OSAL_TaskCreate(&task_id, "TEST", test_task_func,
                    NULL, 4096, 100, 0);

    int32_t ret = OSAL_TaskSetPriority(task_id, 150);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    OSAL_TaskDelete(task_id);
}

/* 测试用例13: 获取任务信息 */
TEST_CASE(test_task_get_info_success)
{
    osal_id_t task_id;
    OS_TaskProp_t task_prop;

    OSAL_TaskCreate(&task_id, "TEST", test_task_func,
                    NULL, 65536, 120, 0);

    int32_t ret = OSAL_TaskGetInfo(task_id, &task_prop);

    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    TEST_ASSERT_EQUAL(120, task_prop.priority);
    TEST_ASSERT_EQUAL(65536, task_prop.stack_size);

    OSAL_TaskDelete(task_id);
}

/* 注册测试套件 - 自动注册 */
TEST_SUITE_BEGIN(osal_task, "osal", "OSAL")
    TEST_CASE_REF(test_task_create_success)
    TEST_CASE_REF(test_task_create_null_pointer)
    TEST_CASE_REF(test_task_create_name_too_long)
    TEST_CASE_REF(test_task_create_invalid_priority)
    TEST_CASE_REF(test_task_create_name_taken)
    TEST_CASE_REF(test_task_delete_success)
    TEST_CASE_REF(test_task_delete_invalid_id)
    TEST_CASE_REF(test_task_delay_success)
    TEST_CASE_REF(test_task_get_id_success)
    TEST_CASE_REF(test_task_get_id_by_name_success)
    TEST_CASE_REF(test_task_get_id_by_name_not_found)
    TEST_CASE_REF(test_task_set_priority_success)
    TEST_CASE_REF(test_task_get_info_success)
TEST_SUITE_END(osal_task, "osal", "OSAL")
