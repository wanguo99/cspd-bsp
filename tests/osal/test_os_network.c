/************************************************************************
 * OSAL网络通信单元测试
 ************************************************************************/

#include "../test_framework.h"
#ifndef STANDALONE_TEST
#include "../test_runner.h"
#endif
#include "osal.h"
#include <string.h>
#include <unistd.h>

#define TEST_PORT 12345
#define TEST_ADDR "127.0.0.1"
#define TEST_DATA "Hello OSAL Network"

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

/* 测试用例1: Socket创建和关闭 */
void test_OS_SocketOpen_Close_Success(void)
{
    setUp();
    osal_id_t sock_id;
    int32 ret;

    /* 创建TCP Socket */
    ret = OS_SocketOpen(&sock_id, OS_SOCK_DOMAIN_INET, OS_SOCK_STREAM);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    TEST_ASSERT_NOT_EQUAL(0, sock_id);

    /* 关闭Socket */
    ret = OS_SocketClose(sock_id);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    tearDown();
}

/* 测试用例2: UDP Socket创建 */
void test_OS_SocketOpen_UDP_Success(void)
{
    setUp();
    osal_id_t sock_id;
    int32 ret;

    /* 创建UDP Socket */
    ret = OS_SocketOpen(&sock_id, OS_SOCK_DOMAIN_INET, OS_SOCK_DGRAM);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    TEST_ASSERT_NOT_EQUAL(0, sock_id);

    ret = OS_SocketClose(sock_id);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    tearDown();
}

/* 测试用例3: Socket绑定 */
void test_OS_SocketBind_Success(void)
{
    setUp();
    osal_id_t sock_id;
    int32 ret;

    ret = OS_SocketOpen(&sock_id, OS_SOCK_DOMAIN_INET, OS_SOCK_STREAM);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 绑定到本地地址 */
    ret = OS_SocketBind(sock_id, TEST_ADDR, TEST_PORT);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    OS_SocketClose(sock_id);
    tearDown();
}

/* 测试用例4: TCP服务器监听 */
void test_OS_SocketListen_Success(void)
{
    setUp();
    osal_id_t sock_id;
    int32 ret;

    ret = OS_SocketOpen(&sock_id, OS_SOCK_DOMAIN_INET, OS_SOCK_STREAM);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = OS_SocketBind(sock_id, TEST_ADDR, TEST_PORT);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 开始监听 */
    ret = OS_SocketListen(sock_id, 5);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    OS_SocketClose(sock_id);
    tearDown();
}

/* 测试用例5: Socket选项设置 */
void test_OS_SocketSetOpt_Success(void)
{
    setUp();
    osal_id_t sock_id;
    int32 ret;
    int32 reuse = 1;
    int32 value;
    uint32 len = sizeof(value);

    ret = OS_SocketOpen(&sock_id, OS_SOCK_DOMAIN_INET, OS_SOCK_STREAM);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 设置地址重用 */
    ret = OS_SocketSetOpt(sock_id, OS_SOL_SOCKET, OS_SO_REUSEADDR,
                          &reuse, sizeof(reuse));
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 获取选项验证 */
    ret = OS_SocketGetOpt(sock_id, OS_SOL_SOCKET, OS_SO_REUSEADDR,
                          &value, &len);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    TEST_ASSERT_EQUAL(1, value);

    OS_SocketClose(sock_id);
    tearDown();
}

/* 测试用例6: UDP发送和接收 */
void test_OS_SocketSendTo_RecvFrom_Success(void)
{
    setUp();
    osal_id_t send_sock, recv_sock;
    int32 ret;
    uint8 send_buffer[64];
    uint8 recv_buffer[64];
    char recv_addr[32];
    uint16 recv_port;
    uint32 data_len = strlen(TEST_DATA);

    /* 创建接收Socket */
    ret = OS_SocketOpen(&recv_sock, OS_SOCK_DOMAIN_INET, OS_SOCK_DGRAM);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = OS_SocketBind(recv_sock, TEST_ADDR, TEST_PORT);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 创建发送Socket */
    ret = OS_SocketOpen(&send_sock, OS_SOCK_DOMAIN_INET, OS_SOCK_DGRAM);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 发送数据 */
    strcpy((char *)send_buffer, TEST_DATA);
    ret = OS_SocketSendTo(send_sock, send_buffer, data_len, TEST_ADDR, TEST_PORT);
    TEST_ASSERT_EQUAL((int32)data_len, ret);

    /* 接收数据 */
    memset(recv_buffer, 0, sizeof(recv_buffer));
    ret = OS_SocketRecvFrom(recv_sock, recv_buffer, sizeof(recv_buffer),
                            recv_addr, &recv_port, 1000);
    TEST_ASSERT_EQUAL((int32)data_len, ret);
    TEST_ASSERT_EQUAL_STRING(TEST_DATA, (char *)recv_buffer);
    TEST_ASSERT_EQUAL_STRING(TEST_ADDR, recv_addr);

    OS_SocketClose(send_sock);
    OS_SocketClose(recv_sock);
    tearDown();
}

/* 测试用例7: 无效参数测试 */
void test_OS_SocketOpen_InvalidParams(void)
{
    setUp();
    osal_id_t sock_id;
    int32 ret;

    /* NULL指针 */
    ret = OS_SocketOpen(NULL, OS_SOCK_DOMAIN_INET, OS_SOCK_STREAM);
    TEST_ASSERT_EQUAL(OS_INVALID_POINTER, ret);

    /* 无效的Socket类型 */
    ret = OS_SocketOpen(&sock_id, OS_SOCK_DOMAIN_INET, 999);
    TEST_ASSERT_EQUAL(OS_ERROR, ret);

    tearDown();
}

/* 测试用例8: 关闭无效Socket */
void test_OS_SocketClose_InvalidID(void)
{
    setUp();
    int32 ret;

    /* 无效的Socket ID */
    ret = OS_SocketClose(0);
    TEST_ASSERT_EQUAL(OS_ERR_INVALID_ID, ret);

    ret = OS_SocketClose(999);
    TEST_ASSERT_EQUAL(OS_ERR_INVALID_ID, ret);

    tearDown();
}

/* 测试用例9: TCP连接超时 */
void test_OS_SocketConnect_Timeout(void)
{
    setUp();
    osal_id_t sock_id;
    int32 ret;

    ret = OS_SocketOpen(&sock_id, OS_SOCK_DOMAIN_INET, OS_SOCK_STREAM);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 连接到不存在的服务器，应该超时 */
    ret = OS_SocketConnect(sock_id, "192.0.2.1", 9999, 100);
    TEST_ASSERT_TRUE(ret == OS_ERROR_TIMEOUT || ret == OS_ERROR);

    OS_SocketClose(sock_id);
    tearDown();
}

/* 模块注册 */
#include "../test_runner.h"

TEST_MODULE_BEGIN(test_os_network)
    TEST_CASE(test_OS_SocketOpen_Close_Success)
    TEST_CASE(test_OS_SocketOpen_UDP_Success)
    TEST_CASE(test_OS_SocketBind_Success)
    TEST_CASE(test_OS_SocketListen_Success)
    TEST_CASE(test_OS_SocketSetOpt_Success)
    TEST_CASE(test_OS_SocketSendTo_RecvFrom_Success)
    TEST_CASE(test_OS_SocketOpen_InvalidParams)
    TEST_CASE(test_OS_SocketClose_InvalidID)
    TEST_CASE(test_OS_SocketConnect_Timeout)
TEST_MODULE_END(test_os_network)

/* 独立运行时的主函数 */
#ifdef STANDALONE_TEST
int main(void)
{
    TEST_BEGIN();

    RUN_TEST(test_OS_SocketOpen_Close_Success);
    RUN_TEST(test_OS_SocketOpen_UDP_Success);
    RUN_TEST(test_OS_SocketBind_Success);
    RUN_TEST(test_OS_SocketListen_Success);
    RUN_TEST(test_OS_SocketSetOpt_Success);
    RUN_TEST(test_OS_SocketSendTo_RecvFrom_Success);
    RUN_TEST(test_OS_SocketOpen_InvalidParams);
    RUN_TEST(test_OS_SocketClose_InvalidID);
    RUN_TEST(test_OS_SocketConnect_Timeout);

    return TEST_END();
}
#endif
