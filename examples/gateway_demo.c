/************************************************************************
 * 示例应用 - 网关管理系统
 *
 * 演示如何使用CSPD-BSP框架构建载荷管理系统
 ************************************************************************/

#include "osal.h"
#include <stdio.h>
#include <string.h>
#include <signal.h>

/*
 * 应用配置
 */
#define QUEUE_DEPTH         10
#define MSG_SIZE            256

/*
 * 消息类型
 */
typedef enum
{
    MSG_TYPE_CMD_REQUEST = 1,
    MSG_TYPE_CMD_RESPONSE,
    MSG_TYPE_STATUS_UPDATE,
    MSG_TYPE_SHUTDOWN
} msg_type_t;

/*
 * 消息结构
 */
typedef struct
{
    msg_type_t type;
    uint32     seq;
    char       data[200];
} app_msg_t;

/*
 * 全局队列ID
 */
static osal_id_t g_cmd_queue;
static osal_id_t g_status_queue;
static volatile bool g_running = true;

/*
 * 信号处理
 */
void signal_handler(int sig)
{
    if (sig == SIGINT || sig == SIGTERM)
    {
        OS_printf("\n收到退出信号，正在关闭...\n");
        g_running = false;
    }
}

/*
 * CAN网关任务
 *
 * 模拟从卫星平台接收命令
 */
void can_gateway_task(void)
{
    app_msg_t msg;
    uint32 seq = 0;
    int32 ret;

    OS_printf("[CAN Gateway] 任务启动\n");

    while (g_running)
    {
        /* 模拟接收CAN命令 */
        OS_TaskDelay(3000);

        if (!g_running)
            break;

        /* 构造命令消息 */
        msg.type = MSG_TYPE_CMD_REQUEST;
        msg.seq = seq++;
        snprintf(msg.data, sizeof(msg.data),
                 "查询载荷状态 (seq=%u)", msg.seq);

        /* 发送到命令队列 */
        ret = OS_QueuePut(g_cmd_queue, &msg, sizeof(msg), 0);
        if (ret == OS_SUCCESS)
        {
            OS_printf("[CAN Gateway] 发送命令: %s\n", msg.data);
        }
        else
        {
            OS_printf("[CAN Gateway] 发送失败: %s\n", OS_GetErrorName(ret));
        }
    }

    OS_printf("[CAN Gateway] 任务退出\n");
}

/*
 * 设备管理任务
 *
 * 处理命令并查询设备状态
 */
void device_manager_task(void)
{
    app_msg_t cmd_msg, status_msg;
    int32 ret;
    uint32 size_copied;

    OS_printf("[Device Manager] 任务启动\n");

    while (g_running)
    {
        /* 接收命令(超时1秒) */
        ret = OS_QueueGet(g_cmd_queue, &cmd_msg, sizeof(cmd_msg),
                          &size_copied, 1000);

        if (ret == OS_QUEUE_TIMEOUT)
        {
            continue;
        }
        else if (ret != OS_SUCCESS)
        {
            OS_printf("[Device Manager] 接收失败: %s\n", OS_GetErrorName(ret));
            continue;
        }

        OS_printf("[Device Manager] 收到命令: %s\n", cmd_msg.data);

        /* 模拟处理命令 */
        OS_TaskDelay(500);

        /* 构造状态响应 */
        status_msg.type = MSG_TYPE_STATUS_UPDATE;
        status_msg.seq = cmd_msg.seq;
        snprintf(status_msg.data, sizeof(status_msg.data),
                 "载荷在线, CPU=45%%, 内存=60%% (seq=%u)", status_msg.seq);

        /* 发送状态更新 */
        ret = OS_QueuePut(g_status_queue, &status_msg, sizeof(status_msg), 0);
        if (ret == OS_SUCCESS)
        {
            OS_printf("[Device Manager] 发送状态: %s\n", status_msg.data);
        }
    }

    OS_printf("[Device Manager] 任务退出\n");
}

/*
 * 状态上报任务
 *
 * 将状态通过CAN发送回卫星平台
 */
void status_reporter_task(void)
{
    app_msg_t status_msg;
    int32 ret;
    uint32 size_copied;

    OS_printf("[Status Reporter] 任务启动\n");

    while (g_running)
    {
        /* 接收状态消息(超时1秒) */
        ret = OS_QueueGet(g_status_queue, &status_msg, sizeof(status_msg),
                          &size_copied, 1000);

        if (ret == OS_QUEUE_TIMEOUT)
        {
            continue;
        }
        else if (ret != OS_SUCCESS)
        {
            OS_printf("[Status Reporter] 接收失败: %s\n", OS_GetErrorName(ret));
            continue;
        }

        OS_printf("[Status Reporter] 上报状态: %s\n", status_msg.data);

        /* 模拟通过CAN发送 */
        OS_TaskDelay(100);
    }

    OS_printf("[Status Reporter] 任务退出\n");
}

/*
 * 主函数
 */
int main(int argc, char *argv[])
{
    int32 ret;
    osal_id_t task_id;

    printf("========================================\n");
    printf("  CSPD-BSP 载荷管理系统示例\n");
    printf("========================================\n\n");

    /* 注册信号处理 */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    /* 初始化OSAL */
    ret = OS_API_Init();
    if (ret != OS_SUCCESS)
    {
        printf("OSAL初始化失败: %s\n", OS_GetErrorName(ret));
        return -1;
    }

    /* 创建消息队列 */
    ret = OS_QueueCreate(&g_cmd_queue, "CMD_QUEUE", QUEUE_DEPTH, MSG_SIZE, 0);
    if (ret != OS_SUCCESS)
    {
        printf("创建命令队列失败: %s\n", OS_GetErrorName(ret));
        return -1;
    }

    ret = OS_QueueCreate(&g_status_queue, "STATUS_QUEUE", QUEUE_DEPTH, MSG_SIZE, 0);
    if (ret != OS_SUCCESS)
    {
        printf("创建状态队列失败: %s\n", OS_GetErrorName(ret));
        return -1;
    }

    OS_printf("消息队列创建成功\n\n");

    /* 创建任务 */
    ret = OS_TaskCreate(&task_id, "CAN_GATEWAY",
                        can_gateway_task, NULL,
                        16384, 100, 0);
    if (ret != OS_SUCCESS)
    {
        printf("创建CAN网关任务失败: %s\n", OS_GetErrorName(ret));
        return -1;
    }

    ret = OS_TaskCreate(&task_id, "DEVICE_MGR",
                        device_manager_task, NULL,
                        16384, 100, 0);
    if (ret != OS_SUCCESS)
    {
        printf("创建设备管理任务失败: %s\n", OS_GetErrorName(ret));
        return -1;
    }

    ret = OS_TaskCreate(&task_id, "STATUS_RPT",
                        status_reporter_task, NULL,
                        16384, 100, 0);
    if (ret != OS_SUCCESS)
    {
        printf("创建状态上报任务失败: %s\n", OS_GetErrorName(ret));
        return -1;
    }

    OS_printf("所有任务创建成功\n");
    OS_printf("按 Ctrl+C 退出\n\n");

    /* 主循环 */
    while (g_running)
    {
        OS_TaskDelay(1000);
    }

    /* 清理 */
    OS_printf("\n正在清理资源...\n");
    OS_TaskDelay(1000);  /* 等待任务退出 */

    OS_QueueDelete(g_cmd_queue);
    OS_QueueDelete(g_status_queue);

    OS_API_Teardown();

    printf("\n程序已退出\n");
    return 0;
}
