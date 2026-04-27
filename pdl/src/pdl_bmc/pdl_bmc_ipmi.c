/************************************************************************
 * BMCIPMI协议实现
 *
 * 职责：
 * - IPMI命令封装和解析
 * - 实现常用IPMI命令（电源控制、传感器读取等）
 ************************************************************************/

#include "pdl_bmc_internal.h"
#include "osal.h"

/*
 * IPMI帧格式（简化版）：
 * [NetFn/LUN][Cmd][Data...][Checksum]
 */

/**
 * @brief 封装IPMI命令
 */
int32_t bmc_ipmi_pack_command(uint8_t cmd_code,
                           uint8_t subcmd,
                           const uint8_t *data,
                           uint32_t data_len,
                           uint8_t *frame,
                           uint32_t frame_size,
                           uint32_t *actual_size)
{
    if (frame == NULL || actual_size == NULL)
    {
        return OS_ERROR;
    }

    uint32_t required_size = 2 + data_len;  /* cmd + subcmd + data */
    if (frame_size < required_size)
    {
        return OS_ERROR;
    }

    uint32_t pos = 0;
    frame[pos++] = cmd_code;
    frame[pos++] = subcmd;

    if (data != NULL && data_len > 0)
    {
        OSAL_Memcpy(&frame[pos], data, data_len);
        pos += data_len;
    }

    *actual_size = pos;
    return OS_SUCCESS;
}

/**
 * @brief 解析IPMI响应
 */
int32_t bmc_ipmi_unpack_response(const uint8_t *frame,
                              uint32_t frame_len,
                              uint8_t *status,
                              uint8_t *data,
                              uint32_t data_size,
                              uint32_t *actual_size)
{
    if (frame == NULL || frame_len < 1)
    {
        return OS_ERROR;
    }

    /* 第一个字节是状态码 */
    if (NULL != status)
    {
        *status = frame[0];
    }

    /* 剩余是数据 */
    if (data != NULL && frame_len > 1)
    {
        uint32_t copy_len = (frame_len - 1 < data_size) ? (frame_len - 1) : data_size;
        OSAL_Memcpy(data, &frame[1], copy_len);

        if (NULL != actual_size)
        {
            *actual_size = copy_len;
        }
    }

    return OS_SUCCESS;
}

/**
 * @brief IPMI电源开机
 */
int32_t bmc_ipmi_power_on(void *comm_handle,
                       int32_t (*send_recv)(void*, const uint8_t*, uint32_t, uint8_t*, uint32_t, uint32_t*))
{
    uint8_t cmd[2];
    uint32_t cmd_len;

    bmc_ipmi_pack_command(IPMI_CMD_CHASSIS_CONTROL, IPMI_CHASSIS_POWER_UP,
                         NULL, 0, cmd, sizeof(cmd), &cmd_len);

    uint8_t resp[16];
    uint32_t resp_len;

    return send_recv(comm_handle, cmd, cmd_len, resp, sizeof(resp), &resp_len);
}

/**
 * @brief IPMI电源关机
 */
int32_t bmc_ipmi_power_off(void *comm_handle,
                        int32_t (*send_recv)(void*, const uint8_t*, uint32_t, uint8_t*, uint32_t, uint32_t*))
{
    uint8_t cmd[2];
    uint32_t cmd_len;

    bmc_ipmi_pack_command(IPMI_CMD_CHASSIS_CONTROL, IPMI_CHASSIS_POWER_DOWN,
                         NULL, 0, cmd, sizeof(cmd), &cmd_len);

    uint8_t resp[16];
    uint32_t resp_len;

    return send_recv(comm_handle, cmd, cmd_len, resp, sizeof(resp), &resp_len);
}

/**
 * @brief IPMI电源复位
 */
int32_t bmc_ipmi_power_reset(void *comm_handle,
                          int32_t (*send_recv)(void*, const uint8_t*, uint32_t, uint8_t*, uint32_t, uint32_t*))
{
    uint8_t cmd[2];
    uint32_t cmd_len;

    bmc_ipmi_pack_command(IPMI_CMD_CHASSIS_CONTROL, IPMI_CHASSIS_HARD_RESET,
                         NULL, 0, cmd, sizeof(cmd), &cmd_len);

    uint8_t resp[16];
    uint32_t resp_len;

    return send_recv(comm_handle, cmd, cmd_len, resp, sizeof(resp), &resp_len);
}

/**
 * @brief 获取电源状态
 */
int32_t bmc_ipmi_get_power_state(void *comm_handle,
                              int32_t (*send_recv)(void*, const uint8_t*, uint32_t, uint8_t*, uint32_t, uint32_t*),
                              bmc_power_state_t *state)
{
    uint8_t cmd[2];
    uint32_t cmd_len;

    bmc_ipmi_pack_command(IPMI_CMD_GET_CHASSIS_STATUS, 0,
                         NULL, 0, cmd, sizeof(cmd), &cmd_len);

    uint8_t resp[16];
    uint32_t resp_len;

    int32_t ret = send_recv(comm_handle, cmd, cmd_len, resp, sizeof(resp), &resp_len);
    if (ret == OS_SUCCESS && resp_len >= 2)
    {
        uint8_t status;
        uint8_t data[16];
        uint32_t data_len;

        bmc_ipmi_unpack_response(resp, resp_len, &status, data, sizeof(data), &data_len);

        if (status == 0 && data_len >= 1)
        {
            /* Bit 0: power is on */
            *state = (data[0] & 0x01) ? BMC_POWER_ON : BMC_POWER_OFF;
        }
        else
        {
            *state = BMC_POWER_UNKNOWN;
        }
    }
    else
    {
        *state = BMC_POWER_UNKNOWN;
    }

    return ret;
}

/**
 * @brief 读取传感器
 */
int32_t bmc_ipmi_read_sensors(void *comm_handle,
                           int32_t (*send_recv)(void*, const uint8_t*, uint32_t, uint8_t*, uint32_t, uint32_t*),
                           bmc_sensor_type_t type,
                           bmc_sensor_reading_t *readings,
                           uint32_t max_count,
                           uint32_t *actual_count)
{
    (void)comm_handle;
    (void)send_recv;
    (void)type;
    (void)readings;
    (void)max_count;

    /* TODO: 实现传感器读取逻辑 */
    if (NULL != actual_count)
    {
        *actual_count = 0;
    }

    return OS_ERROR;
}
