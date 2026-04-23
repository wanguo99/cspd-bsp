/************************************************************************
 * 载荷服务测试模块包装
 * 用于统一测试入口
 ************************************************************************/

#include "test_runner.h"
#include "test_framework.h"
#include "payload_service.h"
#include "osal.h"
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

static payload_service_handle_t test_handle;

/* 模拟服务器线程数据 */
typedef struct {
    uint16 port;
    volatile bool server_ready;
    volatile bool server_running;
    int server_fd;
} mock_server_data_t;

static mock_server_data_t g_mock_server;

/* 模拟TCP服务器线程 */
static void* mock_tcp_server_thread(void *arg)
{
    (void)arg;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd;
    int reuse = 1;

    /* 创建socket */
    g_mock_server.server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (g_mock_server.server_fd < 0) {
        g_mock_server.server_ready = false;
        return NULL;
    }

    /* 设置地址重用 */
    setsockopt(g_mock_server.server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    /* 绑定地址 */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(g_mock_server.port);

    if (bind(g_mock_server.server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(g_mock_server.server_fd);
        g_mock_server.server_ready = false;
        return NULL;
    }

    /* 开始监听 */
    if (listen(g_mock_server.server_fd, 5) < 0) {
        close(g_mock_server.server_fd);
        g_mock_server.server_ready = false;
        return NULL;
    }

    g_mock_server.server_ready = true;

    /* 接受连接 - 只接受一次连接后就退出 */
    fd_set readfds;
    struct timeval tv;

    FD_ZERO(&readfds);
    FD_SET(g_mock_server.server_fd, &readfds);
    tv.tv_sec = 10;  /* 10秒超时 */
    tv.tv_usec = 0;

    int ret = select(g_mock_server.server_fd + 1, &readfds, NULL, NULL, &tv);
    if (ret > 0) {
        client_fd = accept(g_mock_server.server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd >= 0) {
            /* 保持连接打开，不做任何操作 */
            /* 等待客户端关闭或超时 */
            fd_set client_readfds;
            struct timeval client_tv;

            FD_ZERO(&client_readfds);
            FD_SET(client_fd, &client_readfds);
            client_tv.tv_sec = 5;
            client_tv.tv_usec = 0;

            /* 等待数据或超时 */
            select(client_fd + 1, &client_readfds, NULL, NULL, &client_tv);

            close(client_fd);
        }
    }

    close(g_mock_server.server_fd);
    g_mock_server.server_running = false;
    return NULL;
}

void test_PayloadService_Init_Success(void)
{
    OS_API_Init();

    pthread_t server_thread;

    /* 初始化模拟服务器 */
    memset(&g_mock_server, 0, sizeof(g_mock_server));
    g_mock_server.port = 18080;
    g_mock_server.server_ready = false;
    g_mock_server.server_running = true;

    /* 启动模拟服务器 */
    int thread_ret = pthread_create(&server_thread, NULL, mock_tcp_server_thread, NULL);
    if (thread_ret != 0) {
        TEST_MESSAGE("无法创建模拟服务器，测试将失败");
        OS_API_Teardown();
        TEST_ASSERT_EQUAL(0, thread_ret);
        return;
    }

    /* 等待服务器就绪 */
    int timeout = 0;
    while (!g_mock_server.server_ready && timeout < 50) {
        usleep(100000);  /* 100ms */
        timeout++;
    }

    if (!g_mock_server.server_ready) {
        g_mock_server.server_running = false;
        pthread_join(server_thread, NULL);
        TEST_MESSAGE("模拟服务器启动失败，测试将失败");
        OS_API_Teardown();
        TEST_ASSERT_TRUE(g_mock_server.server_ready);
        return;
    }

    /* 配置使用模拟服务器 */
    payload_service_config_t config = {
        .ethernet = {
            .ip_addr = "127.0.0.1",
            .port = 18080,
            .timeout_ms = 5000
        },
        .uart = {
            .device = "/dev/null",  /* 使用/dev/null避免真实串口 */
            .baudrate = 115200,
            .timeout_ms = 2000
        },
        .auto_switch = false,  /* 禁用自动切换 */
        .retry_count = 1
    };

    int32 ret = PayloadService_Init(&config, &test_handle);

    if (ret != OS_SUCCESS) {
        /* 停止模拟服务器 */
        g_mock_server.server_running = false;
        pthread_join(server_thread, NULL);
        TEST_MESSAGE("载荷服务初始化失败");
    } else {
        TEST_ASSERT_NOT_NULL(test_handle);
        TEST_ASSERT_TRUE(PayloadService_IsConnected(test_handle));

        /* 先清理服务，这会关闭连接 */
        PayloadService_Deinit(test_handle);

        /* 等待服务器线程结束 */
        pthread_join(server_thread, NULL);
    }

    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    OS_API_Teardown();
}

void test_PayloadService_Init_NullConfig(void)
{
    OS_API_Init();

    int32 ret = PayloadService_Init(NULL, &test_handle);
    TEST_ASSERT_NOT_EQUAL(OS_SUCCESS, ret);

    OS_API_Teardown();
}

/* 注册测试模块 */
TEST_MODULE_BEGIN(test_payload_service)
    TEST_CASE(test_PayloadService_Init_Success)
    TEST_CASE(test_PayloadService_Init_NullConfig)
TEST_MODULE_END(test_payload_service)
