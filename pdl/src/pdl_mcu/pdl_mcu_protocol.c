/************************************************************************
 * MCU协议工具函数
 *
 * 职责：
 * - CRC16校验计算
 * - 通用帧封装/解析（如果需要）
 ************************************************************************/

#include "pdl_mcu_internal.h"

/**
 * @brief 计算CRC16校验（MODBUS标准）
 *
 * 多项式：0xA001
 * 初始值：0xFFFF
 */
uint16 mcu_protocol_calc_crc16(const uint8 *data, uint32 len)
{
    uint16 crc = 0xFFFF;

    for (uint32 i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (int j = 0; j < 8; j++)
        {
            if (crc & 0x0001)
            {
                crc = (crc >> 1) ^ 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return crc;
}

/**
 * @brief 封装通用帧（预留，目前由各通信模块自己实现）
 */
int32 mcu_protocol_pack_frame(uint8 cmd_code,
                             const uint8 *data,
                             uint32 data_len,
                             bool enable_crc,
                             uint8 *frame,
                             uint32 frame_size,
                             uint32 *actual_size)
{
    (void)cmd_code;
    (void)data;
    (void)data_len;
    (void)enable_crc;
    (void)frame;
    (void)frame_size;
    (void)actual_size;
    /* 预留接口 */
    return OS_ERROR;
}

/**
 * @brief 解析通用帧（预留，目前由各通信模块自己实现）
 */
int32 mcu_protocol_unpack_frame(const uint8 *frame,
                               uint32 frame_len,
                               bool enable_crc,
                               uint8 *cmd_code,
                               uint8 *data,
                               uint32 data_size,
                               uint32 *actual_size)
{
    (void)frame;
    (void)frame_len;
    (void)enable_crc;
    (void)cmd_code;
    (void)data;
    (void)data_size;
    (void)actual_size;
    /* 预留接口 */
    return OS_ERROR;
}
