/************************************************************************
 * 卫星平台服务实现
 *
 * 职责：
 * - 实现对外业务接口
 * - 管理心跳和命令处理任务
 * - 调度内部CAN通信模块
 ************************************************************************/

#include "pdl_satellite.h"
#include "pdl_satellite_internal.h"
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
    void *can_handle;                 /* CAN通信句柄 */
    satellite_cmd_callback_t callback;
    void *user_data;

    /* 统计信息 */
    uint32 rx_count;
    uint32 tx_count;
    uint32 error_count;

    /* 任务控制 */
    osal_id_t rx_task_id;
    osal_id_t heartbeat_task_id;
    bool running;
} satellite_service_context_t;

/*
 * 心跳任务
 */
static void heartbeat_task(void *arg)
{
    satellite_service_context_t *ctx = (satellite_service_context_t *)arg;

    LOG_INFO("SAT", "Heartbeat task started");

    while (!OSAL_TaskShouldShutdown())
    {
        /* 发送心跳 */
        if (satellite_can_send_heartbeat(ctx->can_handle, STATUS_OK) == OS_SUCCESS)
        {
            ctx->tx_count++;
        }
        else
        {
            ctx->error_count++;
        }

        /* 延迟 */
        OSAL_TaskDelay(ctx->config.heartbeat_interval_ms);
    }

    LOG_INFO("SAT", "Heartbeat task stopped");
}

/*
 * CAN接收任务
 */
static void can_rx_task(void *arg)
{
    satellite_service_context_t *ctx = (satellite_service_context_t *)arg;
    satellite_can_msg_t msg;
    int32 ret;

    LOG_INFO("SAT", "CAN RX task started");

    while (!OSAL_TaskShouldShutdown())
    {
        /* 接收CAN消息 */
        ret = satellite_can_recv(ctx->can_handle, &msg, ctx->config.cmd_timeout_ms);

        if (ret == OS_SUCCESS)
        {
            ctx->rx_count++;

            /* 处理命令请求 */
            if (msg.msg_type == CAN_MSG_TYPE_CMD_REQ)
            {
                if (ctx->callback != NULL)
                {
                    ctx->callback(msg.cmd_type, msg.data, ctx->user_data);
                }
            }
        }
        else if (ret != OS_ERROR_TIMEOUT)
        {
            ctx->error_count++;
            LOG_ERROR("SAT", "CAN receive error: %d", ret);
        }
    }

    LOG_INFO("SAT", "CAN RX task stopped");
}

/**
 * @brief 初始化卫星平台服务
 */
int32 PDL_SatelliteInit(const satellite_service_config_t *config,
                        satellite_service_handle_t *handle)
{
    if (config == NULL || handle == NULL)
    {
        return OS_ERROR;
    }

    /* 分配上下文 */
    satellite_service_context_t *ctx = (satellite_service_context_t *)malloc(sizeof(satellite_service_context_t));
    if (ctx == NULL)
    {
        LOG_ERROR("SAT", "Failed to allocate context");
        return OS_ERROR;
    }

    memset(ctx, 0, sizeof(satellite_service_context_t));
    memcpy(&ctx->config, config, sizeof(satellite_service_config_t));
    ctx->running = true;

    /* 初始化CAN通信 */
    int32 ret = satellite_can_init(config->can_device, config->can_bitrate, &ctx->can_handle);
    if (ret != OS_SUCCESS)
    {
        LOG_ERROR("SAT", "Failed to initialize CAN");
        free(ctx);
        return ret;
    }

    /* 创建CAN接收任务 */
    ret = OSAL_TaskCreate(&ctx->rx_task_id, "SAT_RX",
                        can_rx_task, (uint32 *)ctx,
                        TASK_STACK_SIZE_MEDIUM,
                        PRIORITY_HIGH, 0);
    if (ret != OS_SUCCESS)
    {
        LOG_ERROR("SAT", "Failed to create RX task");
        satellite_can_deinit(ctx->can_handle);
        free(ctx);
        return ret;
    }

    /* 创建心跳任务 */
    ret = OSAL_TaskCreate(&ctx->heartbeat_task_id, "SAT_HB",
                        heartbeat_task, (uint32 *)ctx,
                        TASK_STACK_SIZE_SMALL,
                        PRIORITY_LOW, 0);
    if (ret != OS_SUCCESS)
    {
        LOG_ERROR("SAT", "Failed to create heartbeat task");
        OSAL_TaskDelete(ctx->rx_task_id);
        satellite_can_deinit(ctx->can_handle);
        free(ctx);
        return ret;
    }

    *handle = (satellite_service_handle_t)ctx;
    LOG_INFO("SAT", "Satellite service initialized");

    return OS_SUCCESS;
}

/**
 * @brief 反初始化卫星平台服务
 */
int32 PDL_SatelliteDeinit(satellite_service_handle_t handle)
{
    if (handle == NULL)
    {
        return OS_ERROR;
    }

    satellite_service_context_t *ctx = (satellite_service_context_t *)handle;

    /* 停止任务 */
    ctx->running = false;
    OSAL_TaskDelete(ctx->rx_task_id);
    OSAL_TaskDelete(ctx->heartbeat_task_id);

    /* 关闭CAN */
    satellite_can_deinit(ctx->can_handle);

    free(ctx);
    LOG_INFO("SAT", "Satellite service deinitialized");

    return OS_SUCCESS;
}

/**
 * @brief 注册命令回调函数
 */
int32 PDL_SatelliteRegisterCallback(satellite_service_handle_t handle,
                                    satellite_cmd_callback_t callback,
                                    void *user_data)
{
    if (handle == NULL)
    {
        return OS_ERROR;
    }

    satellite_service_context_t *ctx = (satellite_service_context_t *)handle;
    ctx->callback = callback;
    ctx->user_data = user_data;

    return OS_SUCCESS;
}

/**
 * @brief 发送响应到卫星平台
 */
int32 PDL_SatelliteSendResponse(satellite_service_handle_t handle,
                                uint32 seq_num,
                                can_status_t status,
                                uint32 result)
{
    if (handle == NULL)
    {
        return OS_ERROR;
    }

    satellite_service_context_t *ctx = (satellite_service_context_t *)handle;

    int32 ret = satellite_can_send_response(ctx->can_handle, seq_num, status, result);
    if (ret == OS_SUCCESS)
    {
        ctx->tx_count++;
    }
    else
    {
        ctx->error_count++;
        LOG_ERROR("SAT", "Failed to send response");
    }

    return ret;
}

/**
 * @brief 发送心跳到卫星平台
 */
int32 PDL_SatelliteSendHeartbeat(satellite_service_handle_t handle,
                                 can_status_t status)
{
    if (handle == NULL)
    {
        return OS_ERROR;
    }

    satellite_service_context_t *ctx = (satellite_service_context_t *)handle;

    int32 ret = satellite_can_send_heartbeat(ctx->can_handle, status);
    if (ret == OS_SUCCESS)
    {
        ctx->tx_count++;
    }
    else
    {
        ctx->error_count++;
    }

    return ret;
}

/**
 * @brief 获取服务统计信息
 */
int32 PDL_SatelliteGetStats(satellite_service_handle_t handle,
                            uint32 *rx_count,
                            uint32 *tx_count,
                            uint32 *error_count)
{
    if (handle == NULL)
    {
        return OS_ERROR;
    }

    satellite_service_context_t *ctx = (satellite_service_context_t *)handle;

    if (rx_count != NULL) *rx_count = ctx->rx_count;
    if (tx_count != NULL) *tx_count = ctx->tx_count;
    if (error_count != NULL) *error_count = ctx->error_count;

    return OS_SUCCESS;
}
