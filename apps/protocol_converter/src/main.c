/************************************************************************
 * 协议转换应用 - 主程序
 *
 * 负责与载荷通信（IPMI/Redfish协议）
 ************************************************************************/

#include "osal.h"

#include "protocol_converter.h"
#include "config/app_config.h"
#include "config/app_config.h"
#include "config/ethernet_config.h"
#include "config/uart_config.h"
#include "config/task_config.h"
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
        OS_printf("\n收到退出信号，正在关闭协议转换器...\n");
        g_running = false;
    }
}

/**
 * @brief 统计信息任务
 */
static void stats_task(void *arg __attribute__((unused)))
{
    uint32 cmd_count, success, fail, timeout;

    OS_printf("[Stats] 任务启动\n");

    while (g_running)
    {
        OS_TaskDelay(30000);  /* 30秒 */

        Protocol_Converter_GetStats(&cmd_count, &success, &fail, &timeout);

        OS_printf("\n========== 协议转换统计 ==========\n");
        OS_printf("命令总数: %u\n", cmd_count);
        OS_printf("成功: %u, 失败: %u, 超时: %u\n", success, fail, timeout);
        OS_printf("==================================\n\n");
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
    printf("  协议转换应用 v%d.%d.%d\n",
           SYSTEM_VERSION_MAJOR,
           SYSTEM_VERSION_MINOR,
           SYSTEM_VERSION_PATCH);
    printf("========================================\n");
    printf("  载荷IP:   %s\n", SERVER_IP_ADDRESS);
    printf("  IPMI端口: %d\n", IPMI_PORT);
    printf("  备份串口: %s @ %d bps\n", UART_DEVICE, UART_BAUDRATE);
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

    /* 初始化协议转换 */
    ret = Protocol_Converter_Init();
    if (ret != OS_SUCCESS)
    {
        printf("协议转换初始化失败: %s\n", OS_GetErrorName(ret));
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

    OS_printf("\n协议转换器启动完成，按 Ctrl+C 退出\n\n");

    /* 主循环 */
    while (g_running)
    {
        OS_TaskDelay(1000);
    }

    /* 清理 */
    OS_printf("\n正在清理资源...\n");
    OS_TaskDelay(2000);

    OS_API_Teardown();

    printf("\n协议转换器已退出\n");
    return EXIT_SUCCESS;
}
