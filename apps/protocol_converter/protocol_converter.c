/************************************************************************
 * 协议转换应用
 *
 * 负责卫星命令与载荷协议之间的转换
 * - CAN命令 -> IPMI/Redfish命令
 * - 载荷响应 -> CAN响应
 ************************************************************************/

#include "osal.h"
#include "system_config.h"
#include "can_protocol.h"
#include "can_gateway.h"
#include "payload_service.h"
#include <stdatomic.h>
#include <string.h>
#include <stdio.h>

static payload_service_handle_t g_payload_handle;

typedef struct
{
    atomic_uint cmd_count;
    atomic_uint success_count;
    atomic_uint fail_count;
    atomic_uint timeout_count;
} converter_stats_t;

static converter_stats_t g_stats = {0};

static can_status_t execute_ipmi_command(can_cmd_type_t cmd_type, uint32 param __attribute__((unused)), uint32 *result)
{
    char cmd_buf[256];
    char resp_buf[512];
    int32 ret;

    switch (cmd_type)
    {
        case CMD_TYPE_POWER_ON:
            snprintf(cmd_buf, sizeof(cmd_buf), "ipmitool chassis power on");
            break;

        case CMD_TYPE_POWER_OFF:
            snprintf(cmd_buf, sizeof(cmd_buf), "ipmitool chassis power off");
            break;

        case CMD_TYPE_RESET:
            snprintf(cmd_buf, sizeof(cmd_buf), "ipmitool chassis power reset");
            break;

        case CMD_TYPE_QUERY_STATUS:
            snprintf(cmd_buf, sizeof(cmd_buf), "ipmitool chassis status");
            break;

        case CMD_TYPE_QUERY_TEMP:
            snprintf(cmd_buf, sizeof(cmd_buf), "ipmitool sdr type temperature");
            break;

        case CMD_TYPE_QUERY_VOLTAGE:
            snprintf(cmd_buf, sizeof(cmd_buf), "ipmitool sdr type voltage");
            break;

        default:
            OS_printf("[Protocol Converter] 不支持的命令类型: %d\n", cmd_type);
            return STATUS_INVALID_CMD;
    }

    OS_printf("[Protocol Converter] 执行IPMI命令: %s\n", cmd_buf);

    ret = PayloadService_Send(g_payload_handle, cmd_buf, strlen(cmd_buf));
    if (ret < 0)
    {
        OS_printf("[Protocol Converter] 发送命令失败\n");
        return STATUS_COMM_ERROR;
    }

    ret = PayloadService_Recv(g_payload_handle, resp_buf, sizeof(resp_buf) - 1, CMD_TIMEOUT_MS);
    if (ret == OS_ERROR_TIMEOUT)
    {
        OS_printf("[Protocol Converter] 命令超时\n");
        return STATUS_TIMEOUT;
    }
    else if (ret < 0)
    {
        OS_printf("[Protocol Converter] 接收响应失败\n");
        return STATUS_COMM_ERROR;
    }

    /* 安全地添加字符串终止符 */
    if (ret >= (int32)sizeof(resp_buf))
        ret = sizeof(resp_buf) - 1;
    resp_buf[ret] = '\0';
    OS_printf("[Protocol Converter] 收到响应: %s\n", resp_buf);

    /* 解析响应 */
    /* TODO: 实际项目中需要解析IPMI响应格式 */
    *result = 0;  /* 简化处理 */

    return STATUS_OK;
}

/**
 * @brief 协议转换任务
 *
 * 从CAN网关接收命令，转换为载荷协议并执行
 */
static void protocol_converter_task(void *arg __attribute__((unused)))
{
    can_frame_t frame;
    int32 ret;
    uint32 size;
    can_status_t status;
    uint32 result;

    OS_printf("[Protocol Converter] 任务启动\n");

    /* 获取CAN接收队列 */
    osal_id_t can_rx_queue = CAN_Gateway_GetRxQueue();

    while (1)
    {
        /* 从CAN网关接收命令 */
        ret = OS_QueueGet(can_rx_queue, &frame, sizeof(frame), &size, OS_PEND);

        if (ret != OS_SUCCESS)
        {
            continue;
        }

        /* 只处理命令请求 */
        if (frame.msg.msg_type != CAN_MSG_TYPE_CMD_REQ)
        {
            continue;
        }

        atomic_fetch_add(&g_stats.cmd_count, 1);

        OS_printf("[Protocol Converter] 处理命令: %s (seq=%u)\n",
                 can_get_cmd_type_name(frame.msg.cmd_type),
                 frame.msg.seq_num);

        /* 检查载荷连接 */
        if (!PayloadService_IsConnected(g_payload_handle))
        {
            OS_printf("[Protocol Converter] 载荷未连接\n");
            status = STATUS_PAYLOAD_OFFLINE;
            result = 0;
            atomic_fetch_add(&g_stats.fail_count, 1);
        }
        else
        {
            /* 执行命令 */
            status = execute_ipmi_command(frame.msg.cmd_type, frame.msg.data, &result);

            if (status == STATUS_OK)
            {
                atomic_fetch_add(&g_stats.success_count, 1);
            }
            else if (status == STATUS_TIMEOUT)
            {
                atomic_fetch_add(&g_stats.timeout_count, 1);
            }
            else
            {
                atomic_fetch_add(&g_stats.fail_count, 1);
            }
        }

        /* 发送响应 */
        CAN_Gateway_SendResponse(frame.msg.seq_num, status, result);
    }
}

/**
 * @brief 初始化协议转换模块
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 失败
 */
int32 Protocol_Converter_Init(void)
{
    int32 ret;
    osal_id_t task_id;
    payload_service_config_t service_config;

    OS_printf("[Protocol Converter] 初始化...\n");

    /* 配置载荷服务 */
    service_config.ethernet.ip_addr = SERVER_IP_ADDRESS;
    service_config.ethernet.port = IPMI_PORT;
    service_config.ethernet.timeout_ms = ETHERNET_TIMEOUT_MS;
    service_config.uart.device = UART_DEVICE;
    service_config.uart.baudrate = UART_BAUDRATE;
    service_config.uart.timeout_ms = UART_TIMEOUT_MS;
    service_config.auto_switch = true;
    service_config.retry_count = 3;

    /* 初始化载荷服务 */
    ret = PayloadService_Init(&service_config, &g_payload_handle);
    if (ret != OS_SUCCESS)
    {
        OS_printf("[Protocol Converter] 载荷服务初始化失败\n");
        return ret;
    }

    /* 创建协议转换任务 */
    ret = OS_TaskCreate(&task_id, "PROTO_CONV",
                        protocol_converter_task, NULL,
                        TASK_STACK_SIZE_LARGE,
                        PRIORITY_HIGH, 0);
    if (ret != OS_SUCCESS)
    {
        OS_printf("[Protocol Converter] 创建任务失败\n");
        PayloadService_Deinit(g_payload_handle);
        return ret;
    }

    OS_printf("[Protocol Converter] 初始化完成\n");
    return OS_SUCCESS;
}

/**
 * @brief 获取统计信息
 */
void Protocol_Converter_GetStats(uint32 *cmd_count, uint32 *success_count,
                                  uint32 *fail_count, uint32 *timeout_count)
{
    if (cmd_count)     *cmd_count = atomic_load(&g_stats.cmd_count);
    if (success_count) *success_count = atomic_load(&g_stats.success_count);
    if (fail_count)    *fail_count = atomic_load(&g_stats.fail_count);
    if (timeout_count) *timeout_count = atomic_load(&g_stats.timeout_count);
}

/**
 * @brief 切换载荷通信通道
 *
 * @param[in] channel 通道类型
 *
 * @return OS_SUCCESS 成功
 */
int32 Protocol_Converter_SwitchChannel(payload_channel_t channel)
{
    return PayloadService_SwitchChannel(g_payload_handle, channel);
}

/**
 * @brief 获取当前通道
 *
 * @return payload_channel_t 当前通道
 */
payload_channel_t Protocol_Converter_GetChannel(void)
{
    return PayloadService_GetChannel(g_payload_handle);
}
