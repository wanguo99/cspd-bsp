/**
 * @file test_signal.c
 * @brief OSAL信号处理单元测试
 *
 * 使用新的libtest框架，测试自动注册
 */

#include "libutest.h"
#include "test_assert.h"
#include "test_registry.h"
#include "osal.h"
#include "sys/osal_process.h"
#include "sys/osal_time.h"

static volatile int32_t g_signal_received = 0;
static volatile int32_t g_signal_number = 0;

/* 信号处理函数 */
static void test_signal_handler(int32_t signum)
{
    g_signal_received = 1;
    g_signal_number = signum;
}

/* 测试用例1: 注册信号处理函数 */
TEST_CASE(test_signal_register_success)
{
    g_signal_received = 0;
    g_signal_number = 0;

    /* 注册SIGINT处理函数 */
    int32_t ret = OSAL_SignalRegister(OS_SIGNAL_INT, test_signal_handler);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 发送信号给自己 */
    OSAL_Kill(OSAL_Getpid(), SIGINT);

    /* 等待信号处理 */
    OSAL_TaskDelay(100);

    TEST_ASSERT_EQUAL(1, g_signal_received);
    TEST_ASSERT_EQUAL(SIGINT, g_signal_number);

    /* 恢复默认 */
    OSAL_SignalIgnore(OS_SIGNAL_INT);
}

/* 测试用例2: 忽略信号 */
TEST_CASE(test_signal_ignore_success)
{
    g_signal_received = 0;
    g_signal_number = 0;

    /* 先注册处理函数 */
    int32_t ret = OSAL_SignalRegister(OS_SIGNAL_TERM, test_signal_handler);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 然后忽略信号 */
    ret = OSAL_SignalIgnore(OS_SIGNAL_TERM);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 发送信号 */
    OSAL_Kill(OSAL_Getpid(), SIGTERM);
    OSAL_usleep(100000);

    /* 信号应该被忽略，处理函数不应该被调用 */
    TEST_ASSERT_EQUAL(0, g_signal_received);
}

/* 测试用例3: 阻塞信号 */
TEST_CASE(test_signal_block_success)
{
    g_signal_received = 0;
    g_signal_number = 0;

    /* 阻塞SIGINT */
    int32_t ret = OSAL_SignalBlock(OS_SIGNAL_INT);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 注册处理函数 */
    ret = OSAL_SignalRegister(OS_SIGNAL_INT, test_signal_handler);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 发送信号 */
    OSAL_Kill(OSAL_Getpid(), SIGINT);
    OSAL_usleep(100000);

    /* 信号被阻塞，处理函数不应该被调用 */
    TEST_ASSERT_EQUAL(0, g_signal_received);

    /* 解除阻塞 */
    ret = OSAL_SignalUnblock(OS_SIGNAL_INT);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 等待信号处理 */
    OSAL_usleep(100000);

    /* 现在信号应该被处理 */
    TEST_ASSERT_EQUAL(1, g_signal_received);

    /* 恢复默认 */
    OSAL_SignalIgnore(OS_SIGNAL_INT);
}

/* 测试用例4: 恢复默认信号处理 */
TEST_CASE(test_signal_default_success)
{
    g_signal_received = 0;
    g_signal_number = 0;

    /* 先注册处理函数 */
    int32_t ret = OSAL_SignalRegister(OS_SIGNAL_INT, test_signal_handler);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 恢复默认处理 */
    ret = OSAL_SignalDefault(OS_SIGNAL_INT);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 注意：发送SIGINT会导致进程终止（默认行为），所以这里不测试实际效果 */
    /* 只验证API调用成功 */
}

/* 测试用例5: 多个信号处理 */
TEST_CASE(test_signal_register_multiple)
{
    g_signal_received = 0;
    g_signal_number = 0;

    /* 注册多个信号 */
    int32_t ret = OSAL_SignalRegister(OS_SIGNAL_INT, test_signal_handler);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = OSAL_SignalRegister(OS_SIGNAL_TERM, test_signal_handler);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = OSAL_SignalRegister(OS_SIGNAL_USR1, test_signal_handler);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 发送SIGUSR1 */
    OSAL_Kill(OSAL_Getpid(), SIGUSR1);
    OSAL_usleep(100000);

    TEST_ASSERT_EQUAL(1, g_signal_received);
    TEST_ASSERT_EQUAL(SIGUSR1, g_signal_number);

    /* 恢复默认 */
    OSAL_SignalIgnore(OS_SIGNAL_INT);
    OSAL_SignalIgnore(OS_SIGNAL_TERM);
    OSAL_SignalIgnore(OS_SIGNAL_USR1);
}

/* 测试用例6: 无效参数测试 */
TEST_CASE(test_signal_register_invalidparams)
{
    /* NULL处理函数 */
    int32_t ret = OSAL_SignalRegister(OS_SIGNAL_INT, NULL);
    TEST_ASSERT_EQUAL(OS_INVALID_POINTER, ret);
}

/* 注册测试套件 - 自动注册 */
TEST_SUITE_BEGIN(osal_signal, "osal", "OSAL")
    TEST_CASE_REF(test_signal_register_success)
    TEST_CASE_REF(test_signal_ignore_success)
    TEST_CASE_REF(test_signal_block_success)
    TEST_CASE_REF(test_signal_default_success)
    TEST_CASE_REF(test_signal_register_multiple)
    TEST_CASE_REF(test_signal_register_invalidparams)
TEST_SUITE_END(osal_signal, "osal", "OSAL")
