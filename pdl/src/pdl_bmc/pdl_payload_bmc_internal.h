/************************************************************************
 * BMC载荷服务内部接口
 *
 * 仅供pdl_payload_bmc模块内部使用，不对外暴露
 ************************************************************************/

#ifndef PDL_PAYLOAD_BMC_INTERNAL_H
#define PDL_PAYLOAD_BMC_INTERNAL_H

#include "osal_types.h"
#include "pdl_bmc.h"

/*
 * IPMI命令码定义
 */
#define IPMI_CMD_CHASSIS_CONTROL    0x00
#define IPMI_CMD_GET_CHASSIS_STATUS 0x01
#define IPMI_CMD_GET_SENSOR_READING 0x2D

/*
 * IPMI Chassis Control子命令
 */
#define IPMI_CHASSIS_POWER_DOWN     0x00
#define IPMI_CHASSIS_POWER_UP       0x01
#define IPMI_CHASSIS_POWER_CYCLE    0x02
#define IPMI_CHASSIS_HARD_RESET     0x03

/*
 * 网络通信接口（pdl_payload_bmc_net.c实现）
 */
int32 bmc_net_init(const char *ip_addr, uint16 port, uint32 timeout_ms, void **handle);
int32 bmc_net_deinit(void *handle);
int32 bmc_net_send_recv(void *handle,
                       const uint8 *request,
                       uint32 req_size,
                       uint8 *response,
                       uint32 resp_size,
                       uint32 *actual_size);

/*
 * 串口通信接口（pdl_payload_bmc_net.c实现，复用网络模块）
 */
int32 bmc_serial_init(const char *device, uint32 baudrate, uint32 timeout_ms, void **handle);
int32 bmc_serial_deinit(void *handle);
int32 bmc_serial_send_recv(void *handle,
                          const uint8 *request,
                          uint32 req_size,
                          uint8 *response,
                          uint32 resp_size,
                          uint32 *actual_size);

/*
 * IPMI协议接口（pdl_payload_bmc_ipmi.c实现）
 */
int32 bmc_ipmi_pack_command(uint8 cmd_code,
                           uint8 subcmd,
                           const uint8 *data,
                           uint32 data_len,
                           uint8 *frame,
                           uint32 frame_size,
                           uint32 *actual_size);

int32 bmc_ipmi_unpack_response(const uint8 *frame,
                              uint32 frame_len,
                              uint8 *status,
                              uint8 *data,
                              uint32 data_size,
                              uint32 *actual_size);

int32 bmc_ipmi_power_on(void *comm_handle,
                       int32 (*send_recv)(void*, const uint8*, uint32, uint8*, uint32, uint32*));

int32 bmc_ipmi_power_off(void *comm_handle,
                        int32 (*send_recv)(void*, const uint8*, uint32, uint8*, uint32, uint32*));

int32 bmc_ipmi_power_reset(void *comm_handle,
                          int32 (*send_recv)(void*, const uint8*, uint32, uint8*, uint32, uint32*));

int32 bmc_ipmi_get_power_state(void *comm_handle,
                              int32 (*send_recv)(void*, const uint8*, uint32, uint8*, uint32, uint32*),
                              bmc_power_state_t *state);

int32 bmc_ipmi_read_sensors(void *comm_handle,
                           int32 (*send_recv)(void*, const uint8*, uint32, uint8*, uint32, uint32*),
                           bmc_sensor_type_t type,
                           bmc_sensor_reading_t *readings,
                           uint32 max_count,
                           uint32 *actual_count);

#endif /* PDL_PAYLOAD_BMC_INTERNAL_H */
