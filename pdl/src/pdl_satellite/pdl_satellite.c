/************************************************************************
 * 卫星平台接口服务 - Linux实现
 ************************************************************************/

#include "hal_can.h"
#include "pdl_satellite.h"
#include "osal.h"
#include "config/task_config.h"
#include <stdlib.h>
#include <string.h>

/*
 * 卫星平台服务上下文
 */
typedef struct
{
    satellite_service_config_t config;
    hal_can_handle_t can_handle;
    satellite_cmd_callback_t callback;
    void *user_data;

    /* 统计信息 */
    uint32 rx_count;
    uint32 tx_count;
    uint32 error_count;

    /* 心跳任务 */
    osal_id_t heartbeat_task_id;
    bool running;
} satellite_service_context_t;

/*
 * 心跳任务
 */
static void heartbeat_task(void *arg)
{
    satellite_service_context_t *ctx = (satellite_service_context_t *)arg;

    LOG_INFO("SVC_SAT", "Satellite heartbeat task started");

    while (ctx->running) {
        /* 发送心跳 */
        SatellitePDL_SendHeartbeat((satellite_service_handle_t)ctx, STATUS_OK);

        /* 延迟 */
        OS_TaskDelay(ctx->config.heartbeat_interval_ms);
    }

    LOG_INFO("SVC_SAT", "Satellite heartbeat task stopped");
}

/*
 * CAN接收任务
 */
static void can_rx_task(void *arg)
{
    satellite_service_context_t *ctx = (satellite_service_context_t *)arg;
    can_frame_t frame;
    int32 ret;

    LOG_INFO("SVC_SAT", "Satellite CAN RX task started");

    while (ctx->running) {
        /* 接收CAN帧 */
        ret = HAL_CAN_Recv(ctx->can_handle, &frame, ctx->config.cmd_timeout_ms);

        if (ret == OS_SUCCESS) {
            ctx->rx_count++;

            /* 解析命令 */
            can_msg_t *msg = (can_msg_t *)frame.data;
            if (msg->msg_type == CAN_MSG_TYPE_CMD_REQ) {
                /* 调用回调函数 */
                if (ctx->callback != NULL) {
                    ctx->callback(msg->cmd_type, msg->data, ctx->user_data);
                }
            }
        } else if (ret != OS_ERROR_TIMEOUT) {
            ctx->error_count++;
            LOG_ERROR("SVC_SAT", "CAN receive error: %d", ret);
        }
    }

    LOG_INFO("SVC_SAT", "Satellite CAN RX task stopped");
}

/*
 * 初始化卫星平台服务
 */
int32 SatellitePDL_Init(const satellite_service_config_t *config,
                            satellite_service_handle_t *handle)
{
    if (config == NULL || handle == NULL) {
        return OS_INVALID_POINTER;
    }

    /* 分配上下文 */
    satellite_service_context_t *ctx = (satellite_service_context_t *)malloc(sizeof(satellite_service_context_t));
    if (ctx == NULL) {
        LOG_ERROR("SVC_SAT", "Failed to allocate satellite service context");
        return OS_ERROR;
    }

    memset(ctx, 0, sizeof(satellite_service_context_t));
    memcpy(&ctx->config, config, sizeof(satellite_service_config_t));
    ctx->running = true;

    /* 初始化CAN */
    hal_can_config_t can_cfg = {
        .interface = config->can_device,
        .baudrate = config->can_bitrate,
        .rx_timeout = 1000,
        .tx_timeout = 1000
    };

    int32 ret = HAL_CAN_Init(&can_cfg, &ctx->can_handle);
    if (ret != OS_SUCCESS) {
        LOG_ERROR("SVC_SAT", "Failed to initialize CAN device");
        free(ctx);
        return ret;
    }

    /* 创建CAN接收任务 */
    ret = OS_TaskCreate(&ctx->heartbeat_task_id, "SAT_RX",
                        can_rx_task, (uint32*)ctx,
                        TASK_STACK_SIZE_MEDIUM,
                        PRIORITY_HIGH, 0);
    if (ret != OS_SUCCESS) {
        LOG_ERROR("SVC_SAT", "Failed to create CAN RX task");
        HAL_CAN_Deinit(ctx->can_handle);
        free(ctx);
        return ret;
    }

    /* 创建心跳任务 */
    osal_id_t hb_task_id;
    ret = OS_TaskCreate(&hb_task_id, "SAT_HB",
                        heartbeat_task, (uint32*)ctx,
                        TASK_STACK_SIZE_SMALL,
                        PRIORITY_LOW, 0);
    if (ret != OS_SUCCESS) {
        LOG_ERROR("SVC_SAT", "Failed to create heartbeat task");
        ctx->running = false;
        HAL_CAN_Deinit(ctx->can_handle);
        free(ctx);
        return ret;
    }

    *handle = (satellite_service_handle_t)ctx;
    LOG_INFO("SVC_SAT", "Satellite service initialized");

    return OS_SUCCESS;
}

/*
 * 反初始化卫星平台服务
 */
int32 SatellitePDL_Deinit(satellite_service_handle_t handle)
{
    if (handle == NULL) {
        return OS_INVALID_POINTER;
    }

    satellite_service_context_t *ctx = (satellite_service_context_t *)handle;

    /* 停止任务 */
    ctx->running = false;
    OS_TaskDelay(100);  /* 等待任务退出 */

    /* 关闭CAN */
    if (ctx->can_handle != NULL) {
        HAL_CAN_Deinit(ctx->can_handle);
    }

    free(ctx);
    LOG_INFO("SVC_SAT", "Satellite service deinitialized");

    return OS_SUCCESS;
}

/*
 * 注册命令回调函数
 */
int32 SatellitePDL_RegisterCallback(satellite_service_handle_t handle,
                                        satellite_cmd_callback_t callback,
                                        void *user_data)
{
    if (handle == NULL) {
        return OS_INVALID_POINTER;
    }

    satellite_service_context_t *ctx = (satellite_service_context_t *)handle;
    ctx->callback = callback;
    ctx->user_data = user_data;

    return OS_SUCCESS;
}

/*
 * 发送响应到卫星平台
 */
int32 SatellitePDL_SendResponse(satellite_service_handle_t handle,
                                    uint32 seq_num,
                                    can_status_t status,
                                    uint32 result)
{
    if (handle == NULL) {
        return OS_INVALID_POINTER;
    }

    satellite_service_context_t *ctx = (satellite_service_context_t *)handle;

    /* 构造响应帧 */
    can_frame_t frame;
    can_msg_t *msg = (can_msg_t *)frame.data;
    msg->msg_type = CAN_MSG_TYPE_CMD_RESP;
    msg->seq_num = seq_num;
    msg->cmd_type = status;
    msg->data = result;

    /* 发送 */
    int32 ret = HAL_CAN_Send(ctx->can_handle, &frame);
    if (ret == OS_SUCCESS) {
        ctx->tx_count++;
    } else {
        ctx->error_count++;
        LOG_ERROR("SVC_SAT", "Failed to send response");
    }

    return ret;
}

/*
 * 发送心跳到卫星平台
 */
int32 SatellitePDL_SendHeartbeat(satellite_service_handle_t handle,
                                     can_status_t status)
{
    if (handle == NULL) {
        return OS_INVALID_POINTER;
    }

    satellite_service_context_t *ctx = (satellite_service_context_t *)handle;

    /* 构造心跳帧 */
    can_frame_t frame;
    can_msg_t *msg = (can_msg_t *)frame.data;
    msg->msg_type = CAN_MSG_TYPE_HEARTBEAT;
    msg->seq_num = 0;
    msg->cmd_type = status;
    msg->data = 0;

    /* 发送 */
    int32 ret = HAL_CAN_Send(ctx->can_handle, &frame);
    if (ret == OS_SUCCESS) {
        ctx->tx_count++;
    } else {
        ctx->error_count++;
    }

    return ret;
}

/*
 * 获取服务统计信息
 */
int32 SatellitePDL_GetStats(satellite_service_handle_t handle,
                                uint32 *rx_count,
                                uint32 *tx_count,
                                uint32 *error_count)
{
    if (handle == NULL) {
        return OS_INVALID_POINTER;
    }

    satellite_service_context_t *ctx = (satellite_service_context_t *)handle;

    if (rx_count != NULL) *rx_count = ctx->rx_count;
    if (tx_count != NULL) *tx_count = ctx->tx_count;
    if (error_count != NULL) *error_count = ctx->error_count;

    return OS_SUCCESS;
}
