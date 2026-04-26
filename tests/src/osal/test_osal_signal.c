/************************************************************************
 * OSAL信号处理单元测试
 ************************************************************************/

#include "test_framework.h"
#ifndef STANDALONE_TEST
#include "test_runner.h"
#endif
#include "osal.h"

/* 测试代码需要使用平台API来测试OSAL信号功能 */
#include <unistd.h>
#include <signal.h>

static volatile int32_t g_signal_received = 0;
static volatile int32_t g_signal_number = 0;

/* 信号处理函数 */
static void test_signal_handler(int32_t signum)
{
    g_signal_received = 1;
    g_signal_number = signum;
}

/* 测试前初始化 */
__attribute__((unused)) static void setUp(void)
{
    
    g_signal_received = 0;
    g_signal_number = 0;
}

/* 测试后清理 */
__attribute__((unused)) static void tearDown(void)
{
    /* 恢复默认信号处理 */
    OSAL_SignalIgnore(OS_SIGNAL_INT);
    OSAL_SignalIgnore(OS_SIGNAL_TERM);
    
}

/* 测试用例1: 注册信号处理函数 */
void test_osal_signal_register_success(void)
{
    setUp();
    int32_t ret;

    /* 注册SIGINT处理函数 */
    ret = OSAL_SignalRegister(OS_SIGNAL_INT, test_signal_handler);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 发送信号给自己 */
    kill(getpid(), SIGINT);

    /* 等待信号处理 */
    OSAL_TaskDelay(100);  /* 100ms */

    TEST_ASSERT_EQUAL(1, g_signal_received);
    TEST_ASSERT_EQUAL(SIGINT, g_signal_number);

    tearDown();
}

/* 测试用例2: 忽略信号 */
void test_osal_signal_ignore_success(void)
{
    setUp();
    int32_t ret;

    /* 先注册处理函数 */
    ret = OSAL_SignalRegister(OS_SIGNAL_TERM, test_signal_handler);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 然后忽略信号 */
    ret = OSAL_SignalIgnore(OS_SIGNAL_TERM);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 发送信号 */
    kill(getpid(), SIGTERM);
    usleep(100000);

    /* 信号应该被忽略，处理函数不应该被调用 */
    TEST_ASSERT_EQUAL(0, g_signal_received);

    tearDown();
}

/* 测试用例3: 阻塞信号 */
void test_osal_signal_block_success(void)
{
    setUp();
    int32_t ret;

    /* 阻塞SIGINT */
    ret = OSAL_SignalBlock(OS_SIGNAL_INT);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 注册处理函数 */
    ret = OSAL_SignalRegister(OS_SIGNAL_INT, test_signal_handler);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 发送信号 */
    kill(getpid(), SIGINT);
    usleep(100000);

    /* 信号被阻塞，处理函数不应该被调用 */
    TEST_ASSERT_EQUAL(0, g_signal_received);

    /* 解除阻塞 */
    ret = OSAL_SignalUnblock(OS_SIGNAL_INT);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 等待信号处理 */
    usleep(100000);

    /* 现在信号应该被处理 */
    TEST_ASSERT_EQUAL(1, g_signal_received);

    tearDown();
}

/* 测试用例4: 恢复默认信号处理 */
void test_osal_signal_default_success(void)
{
    setUp();
    int32_t ret;

    /* 先注册处理函数 */
    ret = OSAL_SignalRegister(OS_SIGNAL_INT, test_signal_handler);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 恢复默认处理 */
    ret = OSAL_SignalDefault(OS_SIGNAL_INT);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 注意：发送SIGINT会导致进程终止（默认行为），所以这里不测试实际效果 */
    /* 只验证API调用成功 */

    tearDown();
}

/* 测试用例5: 多个信号处理 */
void test_osal_signal_register_multiple(void)
{
    setUp();
    int32_t ret;

    /* 注册多个信号 */
    ret = OSAL_SignalRegister(OS_SIGNAL_INT, test_signal_handler);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = OSAL_SignalRegister(OS_SIGNAL_TERM, test_signal_handler);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = OSAL_SignalRegister(OS_SIGNAL_USR1, test_signal_handler);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 发送SIGUSR1 */
    kill(getpid(), SIGUSR1);
    usleep(100000);

    TEST_ASSERT_EQUAL(1, g_signal_received);
    TEST_ASSERT_EQUAL(SIGUSR1, g_signal_number);

    tearDown();
}

/* 测试用例7: 无效参数测试 */
void test_osal_signal_register_invalidparams(void)
{
    setUp();
    int32_t ret;

    /* NULL处理函数 */
    ret = OSAL_SignalRegister(OS_SIGNAL_INT, NULL);
    TEST_ASSERT_EQUAL(OS_INVALID_POINTER, ret);

    tearDown();
}

/* 模块注册 */
#include "test_runner.h"

TEST_MODULE_BEGIN(test_osal_signal)
    TEST_CASE(test_osal_signal_register_success)
    TEST_CASE(test_osal_signal_ignore_success)
    TEST_CASE(test_osal_signal_block_success)
    TEST_CASE(test_osal_signal_default_success)
    TEST_CASE(test_osal_signal_register_multiple)
    TEST_CASE(test_osal_signal_register_invalidparams)
TEST_MODULE_END(test_osal_signal)

/* 独立运行时的主函数 */
#ifdef STANDALONE_TEST
int main(void)
{
    TEST_BEGIN();

    RUN_TEST(test_osal_SignalRegister_Success);
    RUN_TEST(test_osal_SignalIgnore_Success);
    RUN_TEST(test_osal_SignalBlock_Success);
    RUN_TEST(test_osal_SignalDefault_Success);
    RUN_TEST(test_osal_SignalRegister_Multiple);
    RUN_TEST(test_osal_SignalRegister_InvalidParams);

    return TEST_END();
}
#endif
