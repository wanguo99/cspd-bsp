/************************************************************************
 * 看门狗任务重启机制测试
 ************************************************************************/

#include "unittest_runner.h"
#include "unittest_framework.h"
#include "watchdog.h"
#include "osal.h"
#include <unistd.h>
#include <string.h>

static watchdog_config_t g_watchdog_config;
static volatile int g_test_task_counter = 0;
static volatile bool g_test_task_should_fail = false;

/* 测试任务 */
static void test_task_entry(void *arg)
{
    (void)arg;
    osal_id_t task_id = OS_TaskGetId();

    while (!OS_TaskShouldShutdown())
    {
        g_test_task_counter++;

        if (g_test_task_should_fail)
        {
            /* 模拟任务失败 - 不发送心跳 */
            OS_TaskDelay(100);
        }
        else
        {
            /* 正常运行 - 发送心跳 */
            Watchdog_Heartbeat(task_id);
            OS_TaskDelay(100);
        }
    }
}

/* 测试：任务重启 */
static void test_watchdog_task_restart(void)
{
    int32 ret;
    osal_id_t task_id;

    /* 初始化看门狗 */
    g_watchdog_config.check_interval_ms = 100;
    g_watchdog_config.task_timeout_ms = 500;
    g_watchdog_config.enable_hw_watchdog = false;
    g_watchdog_config.hw_watchdog_dev = "/dev/watchdog";
    g_watchdog_config.hw_watchdog_timeout_s = 10;

    ret = Watchdog_Init(&g_watchdog_config);
    TEST_ASSERT_EQUAL(ret, OS_SUCCESS);

    /* 创建测试任务 */
    ret = OS_TaskCreate(&task_id, "TEST_TASK", test_task_entry, NULL, 8192, 50, 0);
    TEST_ASSERT_EQUAL(ret, OS_SUCCESS);

    /* 注册任务到看门狗 */
    g_test_task_counter = 0;
    g_test_task_should_fail = false;
    ret = Watchdog_RegisterTask(task_id, "TEST_TASK", 500,
                                test_task_entry, NULL, 8192, 50, 3);
    TEST_ASSERT_EQUAL(ret, OS_SUCCESS);

    /* 让任务正常运行一段时间 */
    OS_TaskDelay(1000);
    int counter_before = g_test_task_counter;
    TEST_ASSERT(counter_before > 0);

    /* 让任务停止发送心跳，模拟失败 */
    g_test_task_should_fail = true;
    OS_TaskDelay(800);  /* 等待看门狗检测到失败并重启一次 */

    /* 恢复任务正常运行 */
    g_test_task_should_fail = false;
    OS_TaskDelay(1000);  /* 等待任务恢复正常 */

    /* 检查系统健康状态应该恢复 */
    task_health_t system_health = Watchdog_GetSystemHealth();
    TEST_ASSERT_EQUAL(TASK_HEALTH_OK, system_health);

    /* 清理 */
    Watchdog_Deinit();
}

/* 测试：安全模式 */
static void test_watchdog_safe_mode(void)
{
    int32 ret;
    bool safe_mode;

    /* 初始化看门狗 */
    g_watchdog_config.check_interval_ms = 100;
    g_watchdog_config.task_timeout_ms = 500;
    g_watchdog_config.enable_hw_watchdog = false;
    g_watchdog_config.hw_watchdog_dev = "/dev/watchdog";
    g_watchdog_config.hw_watchdog_timeout_s = 10;

    ret = Watchdog_Init(&g_watchdog_config);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 初始状态不应该在安全模式 */
    safe_mode = Watchdog_IsSafeMode();
    TEST_ASSERT_EQUAL(false, safe_mode);

    /* 清理 */
    Watchdog_Deinit();
}

/* 测试：系统健康状态 */
static void test_watchdog_system_health(void)
{
    int32 ret;
    task_health_t health;

    /* 初始化看门狗 */
    g_watchdog_config.check_interval_ms = 100;
    g_watchdog_config.task_timeout_ms = 500;
    g_watchdog_config.enable_hw_watchdog = false;
    g_watchdog_config.hw_watchdog_dev = "/dev/watchdog";
    g_watchdog_config.hw_watchdog_timeout_s = 10;

    ret = Watchdog_Init(&g_watchdog_config);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 初始状态应该是OK */
    health = Watchdog_GetSystemHealth();
    TEST_ASSERT_EQUAL(TASK_HEALTH_OK, health);

    /* 清理 */
    Watchdog_Deinit();
}

/* 测试：多任务监控 */
static void test_watchdog_multiple_tasks(void)
{
    int32 ret;
    osal_id_t task_id1, task_id2;

    /* 初始化看门狗 */
    g_watchdog_config.check_interval_ms = 100;
    g_watchdog_config.task_timeout_ms = 500;
    g_watchdog_config.enable_hw_watchdog = false;
    g_watchdog_config.hw_watchdog_dev = "/dev/watchdog";
    g_watchdog_config.hw_watchdog_timeout_s = 10;

    ret = Watchdog_Init(&g_watchdog_config);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 创建两个测试任务 */
    ret = OS_TaskCreate(&task_id1, "TEST_TASK1", test_task_entry, NULL, 8192, 50, 0);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = OS_TaskCreate(&task_id2, "TEST_TASK2", test_task_entry, NULL, 8192, 50, 0);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 注册两个任务 */
    ret = Watchdog_RegisterTask(task_id1, "TEST_TASK1", 500,
                                test_task_entry, NULL, 8192, 50, 3);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = Watchdog_RegisterTask(task_id2, "TEST_TASK2", 500,
                                test_task_entry, NULL, 8192, 50, 3);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 让任务正常运行 */
    g_test_task_should_fail = false;
    OS_TaskDelay(1000);

    /* 检查系统健康状态 */
    task_health_t system_health = Watchdog_GetSystemHealth();
    TEST_ASSERT_EQUAL(TASK_HEALTH_OK, system_health);

    /* 清理 */
    Watchdog_Deinit();
}

/* 注册测试模块 */
TEST_MODULE_BEGIN(test_watchdog_restart)
    TEST_CASE(test_watchdog_task_restart)
    TEST_CASE(test_watchdog_safe_mode)
    TEST_CASE(test_watchdog_system_health)
    TEST_CASE(test_watchdog_multiple_tasks)
TEST_MODULE_END(test_watchdog_restart)
