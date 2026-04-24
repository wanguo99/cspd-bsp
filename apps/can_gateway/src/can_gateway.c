/************************************************************************
 * CAN网关应用
 *
 * 负责与卫星平台的CAN通信
 * - 接收卫星平台命令
 * - 发送响应和状态上报
 * - 心跳维护
 ************************************************************************/

#include "osal.h"
#include "config/can_config.h"
#include "config/can_protocol.h"
#include "hal_can.h"
#include "config/task_config.h"
#include <string.h>

/*
 * 全局变量
 */
static hal_can_handle_t g_can_handle;
static osal_id_t g_can_rx_queue;    /* CAN接收队列 */
static osal_id_t g_can_tx_queue;    /* CAN发送队列 */
static uint16 g_seq_num = 0;        /* 序列号 */

/*
 * 统计信息（使用原子操作保证线程安全）
 */
#include <stdatomic.h>

typedef struct
{
    atomic_uint rx_count;
    atomic_uint tx_count;
    atomic_uint err_count;
} can_gateway_stats_t;

static can_gateway_stats_t g_stats = {0};
static osal_id_t g_stats_mutex;  /* 保护序列号的互斥锁 */

/**
 * @brief CAN接收任务
 *
 * 从CAN总线接收消息并转发到内部队列
 */
static void can_rx_task(void *arg)
{
    (void)arg;
    can_frame_t frame;
    int32 ret;

    OS_printf("[CAN Gateway] RX任务启动\n");

    while (1)
    {
        /* 从CAN总线接收 */
        ret = HAL_CAN_Recv(g_can_handle, &frame, CAN_TIMEOUT_MS);

        if (ret == OS_SUCCESS)
        {
            /* 过滤：只接收卫星平台发来的消息 */
            if (frame.can_id == CAN_ID_SAT_TO_BRIDGE)
            {
                can_msg_t *msg = (can_msg_t *)frame.data;
                OS_printf("[CAN Gateway] 收到CAN消息: type=%s, cmd=%s, seq=%u\n",
                         can_get_msg_type_name(msg->msg_type),
                         can_get_cmd_type_name(msg->cmd_type),
                         msg->seq_num);

                /* 转发到内部队列 */
                ret = OS_QueuePut(g_can_rx_queue, &frame, sizeof(frame), 0);
                if (ret == OS_SUCCESS)
                {
                    atomic_fetch_add(&g_stats.rx_count, 1);
                }
                else
                {
                    OS_printf("[CAN Gateway] 队列满，丢弃消息\n");
                    atomic_fetch_add(&g_stats.err_count, 1);
                }
            }
        }
        else if (ret != OS_ERROR_TIMEOUT)
        {
            OS_printf("[CAN Gateway] 接收错误: %s\n", OS_GetErrorName(ret));
            atomic_fetch_add(&g_stats.err_count, 1);
            OS_TaskDelay(100);  /* 错误后延时 */
        }
    }
}

/**
 * @brief CAN发送任务
 *
 * 从内部队列取消息并发送到CAN总线
 */
static void can_tx_task(void *arg)
{
    (void)arg;
    can_frame_t frame;
    int32 ret;
    uint32 size;

    OS_printf("[CAN Gateway] TX任务启动\n");

    while (1)
    {
        /* 从发送队列获取消息 */
        ret = OS_QueueGet(g_can_tx_queue, &frame, sizeof(frame), &size, OS_PEND);

        if (ret == OS_SUCCESS)
        {
            /* 发送到CAN总线 */
            ret = HAL_CAN_Send(g_can_handle, &frame);

            if (ret == OS_SUCCESS)
            {
                can_msg_t *msg = (can_msg_t *)frame.data;
                OS_printf("[CAN Gateway] 发送CAN消息: type=%s, seq=%u\n",
                         can_get_msg_type_name(msg->msg_type),
                         msg->seq_num);
                atomic_fetch_add(&g_stats.tx_count, 1);
            }
            else
            {
                OS_printf("[CAN Gateway] 发送失败: %s\n", OS_GetErrorName(ret));
                atomic_fetch_add(&g_stats.err_count, 1);

                /* 发送失败，重试一次 */
                OS_TaskDelay(10);
                ret = HAL_CAN_Send(g_can_handle, &frame);
                if (ret == OS_SUCCESS)
                {
                    atomic_fetch_add(&g_stats.tx_count, 1);
                }
            }
        }
    }
}

/**
 * @brief 心跳任务
 *
 * 定期向卫星平台发送心跳消息
 */
static void can_heartbeat_task(void *arg __attribute__((unused)))
{
    can_frame_t frame;
    uint32 uptime = 0;

    OS_printf("[CAN Gateway] 心跳任务启动\n");

    while (1)
    {
        /* 延时5秒 */
        OS_TaskDelay(5000);

        /* 构造心跳消息 */
        can_build_heartbeat(&frame, uptime);

        /* 发送心跳 */
        OS_QueuePut(g_can_tx_queue, &frame, sizeof(frame), 0);

        uptime += 5;
    }
}

/**
 * @brief 初始化CAN网关
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 失败
 */
int32 CAN_Gateway_Init(void)
{
    int32 ret;
    osal_id_t task_id;
    hal_can_config_t can_config;

    OS_printf("[CAN Gateway] 初始化...\n");

    /* 初始化原子变量 */
    atomic_init(&g_stats.rx_count, 0);
    atomic_init(&g_stats.tx_count, 0);
    atomic_init(&g_stats.err_count, 0);

    /* 创建互斥锁保护序列号 */
    ret = OS_MutexCreate(&g_stats_mutex, "CAN_SEQ_MUTEX", 0);
    if (ret != OS_SUCCESS)
    {
        OS_printf("[CAN Gateway] 创建互斥锁失败\n");
        return ret;
    }

    /* 创建消息队列 */
    ret = OS_QueueCreate(&g_can_rx_queue, "CAN_RX_QUEUE",
                         CAN_RX_QUEUE_DEPTH, sizeof(can_frame_t), 0);
    if (ret != OS_SUCCESS)
    {
        OS_printf("[CAN Gateway] 创建RX队列失败\n");
        return ret;
    }

    ret = OS_QueueCreate(&g_can_tx_queue, "CAN_TX_QUEUE",
                         CAN_TX_QUEUE_DEPTH, sizeof(can_frame_t), 0);
    if (ret != OS_SUCCESS)
    {
        OS_printf("[CAN Gateway] 创建TX队列失败\n");
        return ret;
    }

    /* 初始化CAN驱动 */
    can_config.interface = CAN_INTERFACE;
    can_config.baudrate = CAN_BAUDRATE;
    can_config.rx_timeout = CAN_TIMEOUT_MS;
    can_config.tx_timeout = CAN_TIMEOUT_MS;

    ret = HAL_CAN_Init(&can_config, &g_can_handle);
    if (ret != OS_SUCCESS)
    {
        OS_printf("[CAN Gateway] CAN驱动初始化失败\n");
        return ret;
    }

    /* 设置CAN过滤器：只接收卫星平台消息 */
    HAL_CAN_SetFilter(g_can_handle, CAN_ID_SAT_TO_BRIDGE, 0x7FF);

    /* 创建任务 */
    ret = OS_TaskCreate(&task_id, "CAN_RX",
                        can_rx_task, NULL,
                        TASK_STACK_SIZE_MEDIUM,
                        PRIORITY_CRITICAL, 0);
    if (ret != OS_SUCCESS)
    {
        OS_printf("[CAN Gateway] 创建RX任务失败\n");
        return ret;
    }

    ret = OS_TaskCreate(&task_id, "CAN_TX",
                        can_tx_task, NULL,
                        TASK_STACK_SIZE_MEDIUM,
                        PRIORITY_CRITICAL, 0);
    if (ret != OS_SUCCESS)
    {
        OS_printf("[CAN Gateway] 创建TX任务失败\n");
        return ret;
    }

    ret = OS_TaskCreate(&task_id, "CAN_HB",
                        can_heartbeat_task, NULL,
                        TASK_STACK_SIZE_SMALL,
                        PRIORITY_LOW, 0);
    if (ret != OS_SUCCESS)
    {
        OS_printf("[CAN Gateway] 创建心跳任务失败\n");
        return ret;
    }

    OS_printf("[CAN Gateway] 初始化完成\n");
    return OS_SUCCESS;
}

/**
 * @brief 获取CAN接收队列ID
 */
osal_id_t CAN_Gateway_GetRxQueue(void)
{
    return g_can_rx_queue;
}

/**
 * @brief 获取CAN发送队列ID
 */
osal_id_t CAN_Gateway_GetTxQueue(void)
{
    return g_can_tx_queue;
}

/**
 * @brief 发送CAN响应
 *
 * @param[in] seq_num 序列号
 * @param[in] status 状态码
 * @param[in] result 结果数据
 *
 * @return OS_SUCCESS 成功
 */
int32 CAN_Gateway_SendResponse(uint16 seq_num, can_status_t status, uint32 result)
{
    can_frame_t frame;

    can_build_cmd_response(&frame, seq_num, status, result);

    return OS_QueuePut(g_can_tx_queue, &frame, sizeof(frame), 0);
}

/**
 * @brief 发送状态上报
 *
 * @param[in] status_data 状态数据
 *
 * @return OS_SUCCESS 成功
 */
int32 CAN_Gateway_SendStatus(uint32 status_data)
{
    can_frame_t frame;
    uint16 seq;

    /* 使用互斥锁保护序列号递增 */
    OS_MutexLock(g_stats_mutex);
    seq = g_seq_num++;
    OS_MutexUnlock(g_stats_mutex);

    can_build_status_report(&frame, seq, status_data);

    return OS_QueuePut(g_can_tx_queue, &frame, sizeof(frame), 0);
}

/**
 * @brief 获取统计信息
 */
void CAN_Gateway_GetStats(uint32 *rx_count, uint32 *tx_count, uint32 *err_count)
{
    /* 原子读取统计信息 */
    if (rx_count)  *rx_count = atomic_load(&g_stats.rx_count);
    if (tx_count)  *tx_count = atomic_load(&g_stats.tx_count);
    if (err_count) *err_count = atomic_load(&g_stats.err_count);
}
