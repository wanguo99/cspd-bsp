/************************************************************************
 * CAN网关应用 - 主程序
 *
 * 负责与卫星平台的CAN通信
 ************************************************************************/

#include "osal.h"

#include "can_gateway.h"
#include "config/can_gateway_config.h"

static volatile bool g_running = true;

/**
 * @brief 信号处理函数
 */
static void signal_handler(int32_t sig)
{
    if (sig == OS_SIGNAL_INT || sig == OS_SIGNAL_TERM)
    {
        OSAL_Printf("\n收到退出信号，正在关闭CAN网关...\n");
        g_running = false;
    }
}

/**
 * @brief 统计信息任务
 */
static void stats_task(void *arg __attribute__((unused)))
{
    uint32_t rx_count, tx_count, err_count;

    OSAL_Printf("[Stats] 任务启动\n");

    while (g_running)
    {
        OSAL_TaskDelay(30000);  /* 30秒 */

        CAN_Gateway_GetStats(&rx_count, &tx_count, &err_count);

        OSAL_Printf("\n========== CAN网关统计 ==========\n");
        OSAL_Printf("接收: %u, 发送: %u, 错误: %u\n", rx_count, tx_count, err_count);
        OSAL_Printf("================================\n\n");
    }
}

/**
 * @brief 主函数
 */
int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused)))
{
    int32_t ret;
    osal_id_t task_id;

    OSAL_Printf("========================================\n");
    OSAL_Printf("  CAN网关应用 v%d.%d.%d\n",
           SYSTEM_VERSION_MAJOR,
           SYSTEM_VERSION_MINOR,
           SYSTEM_VERSION_PATCH);
    OSAL_Printf("========================================\n");
    OSAL_Printf("  CAN接口: %s @ %d bps\n", CAN_INTERFACE, CAN_BAUDRATE);
    OSAL_Printf("========================================\n\n");

    OSAL_SignalRegister(OS_SIGNAL_INT, signal_handler);
    OSAL_SignalRegister(OS_SIGNAL_TERM, signal_handler);

    /* 初始化OSAL */
    ret = OS_API_Init();
    if (ret != OS_SUCCESS)
    {
        OSAL_Printf("OSAL初始化失败: %s\n", OSAL_GetStatusName(ret));
        return 1;
    }

    /* 初始化CAN网关 */
    ret = CAN_Gateway_Init();
    if (ret != OS_SUCCESS)
    {
        OSAL_Printf("CAN网关初始化失败: %s\n", OSAL_GetStatusName(ret));
        return 1;
    }

    /* 创建统计任务 */
    ret = OSAL_TaskCreate(&task_id, "STATS",
                        stats_task, NULL,
                        OSAL_TASK_STACK_SIZE_SMALL,
                        OSAL_TASK_PRIORITY_LOW, 0);
    if (ret != OS_SUCCESS)
    {
        OSAL_Printf("创建统计任务失败: %s\n", OSAL_GetStatusName(ret));
        return 1;
    }

    OSAL_Printf("\nCAN网关启动完成，按 Ctrl+C 退出\n\n");

    /* 主循环 */
    while (g_running)
    {
        OSAL_TaskDelay(1000);
    }

    /* 清理 */
    OSAL_Printf("\n正在清理资源...\n");
    OSAL_TaskDelay(2000);

    OS_API_Teardown();

    OSAL_Printf("\nCAN网关已退出\n");
    return 0;
}
