/************************************************************************
 * 卫星-载荷桥接管理板 - 主程序
 *
 * 系统入口，初始化所有模块并启动系统
 ************************************************************************/

#include "osal.h"
#include "system_config.h"
#include "can_gateway.h"
#include "protocol_converter.h"
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

/*
 * 全局运行标志
 */
static volatile bool g_running = true;

/**
 * @brief 信号处理函数
 */
static void signal_handler(int sig)
{
    if (sig == SIGINT || sig == SIGTERM)
    {
        OS_printf("\n收到退出信号，正在关闭系统...\n");
        g_running = false;
    }
}

/**
 * @brief 打印系统信息
 */
static void print_system_info(void)
{
    printf("========================================\n");
    printf("  %s v%d.%d.%d\n",
           SYSTEM_NAME,
           SYSTEM_VERSION_MAJOR,
           SYSTEM_VERSION_MINOR,
           SYSTEM_VERSION_PATCH);
    printf("========================================\n");
    printf("  CAN接口:    %s @ %d bps\n", CAN_INTERFACE, CAN_BAUDRATE);
    printf("  载荷IP:   %s\n", SERVER_IP_ADDRESS);
    printf("  IPMI端口:   %d\n", IPMI_PORT);
    printf("  备份串口:   %s @ %d bps\n", UART_DEVICE, UART_BAUDRATE);
    printf("========================================\n\n");
}

/**
 * @brief 统计信息任务
 *
 * 定期打印系统统计信息
 */
static void stats_task(void *arg __attribute__((unused)))
{
    uint32 can_rx, can_tx, can_err;
    uint32 cmd_count, success, fail, timeout;

    OS_printf("[Stats] 任务启动\n");

    while (g_running)
    {
        OS_TaskDelay(30000);  /* 30秒 */

        /* 获取统计信息 */
        CAN_Gateway_GetStats(&can_rx, &can_tx, &can_err);
        Protocol_Converter_GetStats(&cmd_count, &success, &fail, &timeout);

        /* 打印统计 */
        OS_printf("\n========== 系统统计 ==========\n");
        OS_printf("CAN:  RX=%u, TX=%u, ERR=%u\n", can_rx, can_tx, can_err);
        OS_printf("命令: 总数=%u, 成功=%u, 失败=%u, 超时=%u\n",
                 cmd_count, success, fail, timeout);
        OS_printf("==============================\n\n");
    }
}

/**
 * @brief 主函数
 */
int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused)))
{
    int32 ret;
    osal_id_t task_id;

    /* 打印系统信息 */
    print_system_info();

    /* 注册信号处理 */
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

    OS_printf("\n系统启动完成，按 Ctrl+C 退出\n\n");

    /* 主循环 */
    while (g_running)
    {
        OS_TaskDelay(1000);
    }

    /* 清理 */
    OS_printf("\n正在清理资源...\n");
    OS_TaskDelay(2000);  /* 等待任务退出 */

    OS_API_Teardown();

    printf("\n系统已退出\n");
    return EXIT_SUCCESS;
}
