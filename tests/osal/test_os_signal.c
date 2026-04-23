/************************************************************************
 * OSAL信号处理单元测试
 ************************************************************************/

#include "../test_framework.h"
#include "osal.h"
#include <unistd.h>
#include <signal.h>

static volatile int32 g_signal_received = 0;
static volatile int32 g_signal_number = 0;

/* 信号处理函数 */
static void test_signal_handler(int32 signum)
{
    g_signal_received = 1;
    g_signal_number = signum;
}

/* 测试前初始化 */
void setUp(void)
{
    OS_API_Init();
    g_signal_received = 0;
    g_signal_number = 0;
}

/* 测试后清理 */
void tearDown(void)
{
    /* 恢复默认信号处理 */
    OS_SignalIgnore(OS_SIGNAL_INT);
    OS_SignalIgnore(OS_SIGNAL_TERM);
    OS_API_Teardown();
}

/* 测试用例1: 注册信号处理函数 */
void test_OS_SignalRegister_Success(void)
{
    setUp();
    int32 ret;

    /* 注册SIGINT处理函数 */
    ret = OS_SignalRegister(OS_SIGNAL_INT, test_signal_handler);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 发送信号给自己 */
    kill(getpid(), SIGINT);

    /* 等待信号处理 */
    usleep(100000);  /* 100ms */

    TEST_ASSERT_EQUAL(1, g_signal_received);
    TEST_ASSERT_EQUAL(SIGINT, g_signal_number);

    tearDown();
}

/* 测试用例2: 忽略信号 */
void test_OS_SignalIgnore_Success(void)
{
    setUp();
    int32 ret;

    /* 先注册处理函数 */
    ret = OS_SignalRegister(OS_SIGNAL_TERM, test_signal_handler);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 然后忽略信号 */
    ret = OS_SignalIgnore(OS_SIGNAL_TERM);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 发送信号 */
    kill(getpid(), SIGTERM);
    usleep(100000);

    /* 信号应该被忽略，处理函数不应该被调用 */
    TEST_ASSERT_EQUAL(0, g_signal_received);

    tearDown();
}

/* 测试用例3: 阻塞信号 */
void test_OS_SignalBlock_Success(void)
{
    setUp();
    int32 ret;

    /* 阻塞SIGINT */
    ret = OS_SignalBlock(OS_SIGNAL_INT);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 注册处理函数 */
    ret = OS_SignalRegister(OS_SIGNAL_INT, test_signal_handler);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 发送信号 */
    kill(getpid(), SIGINT);
    usleep(100000);

    /* 信号被阻塞，处理函数不应该被调用 */
    TEST_ASSERT_EQUAL(0, g_signal_received);

    /* 解除阻塞 */
    ret = OS_SignalUnblock(OS_SIGNAL_INT);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 等待信号处理 */
    usleep(100000);

    /* 现在信号应该被处理 */
    TEST_ASSERT_EQUAL(1, g_signal_received);

    tearDown();
}

/* 测试用例4: 恢复默认信号处理 */
void test_OS_SignalDefault_Success(void)
{
    setUp();
    int32 ret;

    /* 先注册处理函数 */
    ret = OS_SignalRegister(OS_SIGNAL_INT, test_signal_handler);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 恢复默认处理 */
    ret = OS_SignalDefault(OS_SIGNAL_INT);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 注意：发送SIGINT会导致进程终止（默认行为），所以这里不测试实际效果 */
    /* 只验证API调用成功 */

    tearDown();
}

/* 测试用例5: 多个信号处理 */
void test_OS_SignalRegister_Multiple(void)
{
    setUp();
    int32 ret;

    /* 注册多个信号 */
    ret = OS_SignalRegister(OS_SIGNAL_INT, test_signal_handler);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = OS_SignalRegister(OS_SIGNAL_TERM, test_signal_handler);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = OS_SignalRegister(OS_SIGNAL_USR1, test_signal_handler);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 发送SIGUSR1 */
    kill(getpid(), SIGUSR1);
    usleep(100000);

    TEST_ASSERT_EQUAL(1, g_signal_received);
    TEST_ASSERT_EQUAL(SIGUSR1, g_signal_number);

    tearDown();
}

/* 测试用例7: 无效参数测试 */
void test_OS_SignalRegister_InvalidParams(void)
{
    setUp();
    int32 ret;

    /* NULL处理函数 */
    ret = OS_SignalRegister(OS_SIGNAL_INT, NULL);
    TEST_ASSERT_EQUAL(OS_INVALID_POINTER, ret);

    tearDown();
}

/* 主测试运行器 */
int main(void)
{
    TEST_BEGIN();

    RUN_TEST(test_OS_SignalRegister_Success);
    RUN_TEST(test_OS_SignalIgnore_Success);
    RUN_TEST(test_OS_SignalBlock_Success);
    RUN_TEST(test_OS_SignalDefault_Success);
    RUN_TEST(test_OS_SignalRegister_Multiple);
    RUN_TEST(test_OS_SignalRegister_InvalidParams);

    return TEST_END();
}
