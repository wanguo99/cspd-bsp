/************************************************************************
 * BMC服务内部接口
 *
 * 仅供pdl_bmc模块内部使用，不对外暴露
 ************************************************************************/

#ifndef PDL_BMC_INTERNAL_H
#define PDL_BMC_INTERNAL_H

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
 * 网络通信接口（pdl_bmc_net.c实现）
 */
int32_t bmc_redfish_init(const char *ip_addr, uint16_t port, uint32_t timeout_ms, void **handle);
int32_t bmc_redfish_deinit(void *handle);
int32_t bmc_redfish_send_recv(void *handle,
                       const uint8_t *request,
                       uint32_t req_size,
                       uint8_t *response,
                       uint32_t resp_size,
                       uint32_t *actual_size);

/*
 * 串口通信接口（pdl_bmc_net.c实现，复用网络模块）
 */
int32_t bmc_serial_init(const char *device, uint32_t baudrate, uint32_t timeout_ms, void **handle);
int32_t bmc_serial_deinit(void *handle);
int32_t bmc_serial_send_recv(void *handle,
                          const uint8_t *request,
                          uint32_t req_size,
                          uint8_t *response,
                          uint32_t resp_size,
                          uint32_t *actual_size);

/*
 * IPMI协议接口（pdl_bmc_ipmi.c实现）
 */
int32_t bmc_ipmi_pack_command(uint8_t cmd_code,
                           uint8_t subcmd,
                           const uint8_t *data,
                           uint32_t data_len,
                           uint8_t *frame,
                           uint32_t frame_size,
                           uint32_t *actual_size);

int32_t bmc_ipmi_unpack_response(const uint8_t *frame,
                              uint32_t frame_len,
                              uint8_t *status,
                              uint8_t *data,
                              uint32_t data_size,
                              uint32_t *actual_size);

int32_t bmc_ipmi_power_on(void *comm_handle,
                       int32_t (*send_recv)(void*, const uint8_t*, uint32_t, uint8_t*, uint32_t, uint32_t*));

int32_t bmc_ipmi_power_off(void *comm_handle,
                        int32_t (*send_recv)(void*, const uint8_t*, uint32_t, uint8_t*, uint32_t, uint32_t*));

int32_t bmc_ipmi_power_reset(void *comm_handle,
                          int32_t (*send_recv)(void*, const uint8_t*, uint32_t, uint8_t*, uint32_t, uint32_t*));

int32_t bmc_ipmi_get_power_state(void *comm_handle,
                              int32_t (*send_recv)(void*, const uint8_t*, uint32_t, uint8_t*, uint32_t, uint32_t*),
                              bmc_power_state_t *state);

int32_t bmc_ipmi_read_sensors(void *comm_handle,
                           int32_t (*send_recv)(void*, const uint8_t*, uint32_t, uint8_t*, uint32_t, uint32_t*),
                           bmc_sensor_type_t type,
                           bmc_sensor_reading_t *readings,
                           uint32_t max_count,
                           uint32_t *actual_count);

#endif /* PDL_BMC_INTERNAL_H */
