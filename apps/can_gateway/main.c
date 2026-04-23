/************************************************************************
 * CAN网关应用 - 主程序
 *
 * 负责与卫星平台的CAN通信
 ************************************************************************/

#include "osal.h"
#include "system_config.h"
#include "can_gateway.h"
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

static volatile bool g_running = true;

/**
 * @brief 信号处理函数
 */
static void signal_handler(int sig)
{
    if (sig == SIGINT || sig == SIGTERM)
    {
        OS_printf("\n收到退出信号，正在关闭CAN网关...\n");
        g_running = false;
    }
}

/**
 * @brief 统计信息任务
 */
static void stats_task(void *arg __attribute__((unused)))
{
    uint32 rx_count, tx_count, err_count;

    OS_printf("[Stats] 任务启动\n");

    while (g_running)
    {
        OS_TaskDelay(30000);  /* 30秒 */

        CAN_Gateway_GetStats(&rx_count, &tx_count, &err_count);

        OS_printf("\n========== CAN网关统计 ==========\n");
        OS_printf("接收: %u, 发送: %u, 错误: %u\n", rx_count, tx_count, err_count);
        OS_printf("================================\n\n");
    }
}

/**
 * @brief 主函数
 */
int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused)))
{
    int32 ret;
    osal_id_t task_id;

    printf("========================================\n");
    printf("  CAN网关应用 v%d.%d.%d\n",
           SYSTEM_VERSION_MAJOR,
           SYSTEM_VERSION_MINOR,
           SYSTEM_VERSION_PATCH);
    printf("========================================\n");
    printf("  CAN接口: %s @ %d bps\n", CAN_INTERFACE, CAN_BAUDRATE);
    printf("========================================\n\n");

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    /* 初始化OSAL */
    ret = OS_API_Init();
    if (ret != OS_SUCCESS)
    {
        printf("OSAL初始化失败: %s\n", OS_GetErrorName(ret));
        return EXIT_FAILURE;
    }

    /* 初始化CAN网关 */
    ret = CAN_Gateway_Init();
    if (ret != OS_SUCCESS)
    {
        printf("CAN网关初始化失败: %s\n", OS_GetErrorName(ret));
        return EXIT_FAILURE;
    }

    /* 创建统计任务 */
    ret = OS_TaskCreate(&task_id, "STATS",
                        stats_task, NULL,
                        TASK_STACK_SIZE_SMALL,
                        PRIORITY_LOW, 0);
    if (ret != OS_SUCCESS)
    {
        printf("创建统计任务失败: %s\n", OS_GetErrorName(ret));
        return EXIT_FAILURE;
    }

    OS_printf("\nCAN网关启动完成，按 Ctrl+C 退出\n\n");

    /* 主循环 */
    while (g_running)
    {
        OS_TaskDelay(1000);
    }

    /* 清理 */
    OS_printf("\n正在清理资源...\n");
    OS_TaskDelay(2000);

    OS_API_Teardown();

    printf("\nCAN网关已退出\n");
    return EXIT_SUCCESS;
}
