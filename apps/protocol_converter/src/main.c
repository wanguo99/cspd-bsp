/************************************************************************
 * 协议转换应用 - 主程序
 *
 * 负责与载荷通信（IPMI/Redfish协议）
 ************************************************************************/

#include "osal.h"

#include "protocol_converter.h"
#include "config/protocol_converter_config.h"
#include "config/ethernet_config.h"

static volatile bool g_running = true;

/**
 * @brief 信号处理函数
 */
static void signal_handler(int32 sig)
{
    if (sig == OS_SIGNAL_INT || sig == OS_SIGNAL_TERM)
    {
        OSAL_Printf("\n收到退出信号，正在关闭协议转换器...\n");
        g_running = false;
    }
}

/**
 * @brief 统计信息任务
 */
static void stats_task(void *arg __attribute__((unused)))
{
    uint32 cmd_count, success, fail, timeout;

    OSAL_Printf("[Stats] 任务启动\n");

    while (g_running)
    {
        OSAL_TaskDelay(30000);  /* 30秒 */

        Protocol_Converter_GetStats(&cmd_count, &success, &fail, &timeout);

        OSAL_Printf("\n========== 协议转换统计 ==========\n");
        OSAL_Printf("命令总数: %u\n", cmd_count);
        OSAL_Printf("成功: %u, 失败: %u, 超时: %u\n", success, fail, timeout);
        OSAL_Printf("==================================\n\n");
    }
}

/**
 * @brief 主函数
 */
int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused)))
{
    int32 ret;
    osal_id_t task_id;

    OSAL_Printf("========================================\n");
    OSAL_Printf("  协议转换应用 v%d.%d.%d\n",
           SYSTEM_VERSION_MAJOR,
           SYSTEM_VERSION_MINOR,
           SYSTEM_VERSION_PATCH);
    OSAL_Printf("========================================\n");
    OSAL_Printf("  载荷IP:   %s\n", SERVER_IP_ADDRESS);
    OSAL_Printf("  IPMI端口: %d\n", IPMI_PORT);
    OSAL_Printf("  备份串口: %s @ %d bps\n", UART_DEVICE, UART_BAUDRATE);
    OSAL_Printf("========================================\n\n");

    OSAL_SignalRegister(OS_SIGNAL_INT, signal_handler);
    OSAL_SignalRegister(OS_SIGNAL_TERM, signal_handler);

    /* 初始化OSAL */
    ret = OS_API_Init();
    if (ret != OS_SUCCESS)
    {
        OSAL_Printf("OSAL初始化失败: %s\n", OS_GetErrorName(ret));
        return 1;
    }

    /* 初始化协议转换 */
    ret = Protocol_Converter_Init();
    if (ret != OS_SUCCESS)
    {
        OSAL_Printf("协议转换初始化失败: %s\n", OS_GetErrorName(ret));
        return 1;
    }

    /* 创建统计任务 */
    ret = OSAL_TaskCreate(&task_id, "STATS",
                        stats_task, NULL,
                        OSAL_TASK_STACK_SIZE_SMALL,
                        OSAL_TASK_PRIORITY_LOW, 0);
    if (ret != OS_SUCCESS)
    {
        OSAL_Printf("创建统计任务失败: %s\n", OS_GetErrorName(ret));
        return 1;
    }

    OSAL_Printf("\n协议转换器启动完成，按 Ctrl+C 退出\n\n");

    /* 主循环 */
    while (g_running)
    {
        OSAL_TaskDelay(1000);
    }

    /* 清理 */
    OSAL_Printf("\n正在清理资源...\n");
    OSAL_TaskDelay(2000);

    OS_API_Teardown();

    OSAL_Printf("\n协议转换器已退出\n");
    return 0;
}
