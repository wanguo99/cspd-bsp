/************************************************************************
 * 硬件配置库API接口
 *
 * 功能：
 * - 硬件配置注册和查询（以外设为单位）
 * - APP配置查询
 * - 配置验证和加载
 * - 运行时配置切换
 *
 * 命名规范：
 * - XCONFIG_HW_*  - 硬件配置相关接口
 * - XCONFIG_APP_* - APP配置相关接口
 ************************************************************************/

#ifndef XCONFIG_API_H
#define XCONFIG_API_H

#include "xconfig_board.h"

/*===========================================================================
 * 配置库初始化
 *===========================================================================*/

/**
 * @brief 初始化硬件配置库
 *
 * @return OS_SUCCESS 成功
 */
int32_t XCONFIG_Init(void);

/**
 * @brief 清理硬件配置库
 */
void XCONFIG_Cleanup(void);

/*===========================================================================
 * 板级配置注册和查询
 *===========================================================================*/

/**
 * @brief 注册板级配置
 *
 * @param[in] config 板级配置指针
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 失败
 */
int32_t XCONFIG_Register(const xconfig_board_config_t *config);

/**
 * @brief 获取当前板级配置
 *
 * @return 板级配置指针，失败返回NULL
 */
const xconfig_board_config_t* XCONFIG_GetBoard(void);

/**
 * @brief 根据平台和产品名称查找配置
 *
 * @param[in] platform 平台名称（如"ti/am625"）
 * @param[in] product 产品名称（如"h200_payload"）
 * @param[in] version 版本号（如"v1.0"，可选）
 *
 * @return 板级配置指针，失败返回NULL
 */
const xconfig_board_config_t* XCONFIG_Find(const char *platform,
                                            const char *product,
                                            const char *version);

/**
 * @brief 列出所有已注册的配置
 *
 * @param[out] configs 配置列表
 * @param[in,out] count 输入：缓冲区大小，输出：实际数量
 *
 * @return OS_SUCCESS 成功
 */
int32_t XCONFIG_List(const xconfig_board_config_t **configs, uint32_t *count);

/*===========================================================================
 * 硬件外设配置查询接口（XCONFIG_HW_*）
 *===========================================================================*/

/**
 * @brief 根据名称查找MCU外设配置
 *
 * @param[in] board 板级配置
 * @param[in] name MCU名称
 *
 * @return MCU配置指针，失败返回NULL
 */
const xconfig_mcu_cfg_t* XCONFIG_HW_FindMCU(const xconfig_board_config_t *board,
                                             const char *name);

/**
 * @brief 根据编号获取MCU外设配置
 *
 * @param[in] board 板级配置
 * @param[in] id MCU编号（第几个）
 *
 * @return MCU配置指针，失败返回NULL
 */
const xconfig_mcu_cfg_t* XCONFIG_HW_GetMCU(const xconfig_board_config_t *board,
                                            uint32_t id);

/**
 * @brief 根据名称查找BMC外设配置
 *
 * @param[in] board 板级配置
 * @param[in] name BMC名称
 *
 * @return BMC配置指针，失败返回NULL
 */
const xconfig_bmc_cfg_t* XCONFIG_HW_FindBMC(const xconfig_board_config_t *board,
                                             const char *name);

/**
 * @brief 根据编号获取BMC外设配置
 *
 * @param[in] board 板级配置
 * @param[in] id BMC编号（第几个）
 *
 * @return BMC配置指针，失败返回NULL
 */
const xconfig_bmc_cfg_t* XCONFIG_HW_GetBMC(const xconfig_board_config_t *board,
                                            uint32_t id);

/**
 * @brief 根据名称查找卫星平台接口配置
 *
 * @param[in] board 板级配置
 * @param[in] name 卫星平台名称
 *
 * @return 卫星平台配置指针，失败返回NULL
 */
const xconfig_satellite_cfg_t* XCONFIG_HW_FindSatellite(const xconfig_board_config_t *board,
                                                         const char *name);

/**
 * @brief 根据编号获取卫星平台接口配置
 *
 * @param[in] board 板级配置
 * @param[in] id 卫星平台编号（第几个）
 *
 * @return 卫星平台配置指针，失败返回NULL
 */
const xconfig_satellite_cfg_t* XCONFIG_HW_GetSatellite(const xconfig_board_config_t *board,
                                                        uint32_t id);

/**
 * @brief 根据名称查找传感器外设配置
 *
 * @param[in] board 板级配置
 * @param[in] name 传感器名称
 *
 * @return 传感器配置指针，失败返回NULL
 */
const xconfig_sensor_cfg_t* XCONFIG_HW_FindSensor(const xconfig_board_config_t *board,
                                                   const char *name);

/**
 * @brief 根据编号获取传感器外设配置
 *
 * @param[in] board 板级配置
 * @param[in] id 传感器编号（第几个）
 *
 * @return 传感器配置指针，失败返回NULL
 */
const xconfig_sensor_cfg_t* XCONFIG_HW_GetSensor(const xconfig_board_config_t *board,
                                                  uint32_t id);

/**
 * @brief 根据名称查找存储设备配置
 *
 * @param[in] board 板级配置
 * @param[in] name 存储设备名称
 *
 * @return 存储设备配置指针，失败返回NULL
 */
const xconfig_storage_cfg_t* XCONFIG_HW_FindStorage(const xconfig_board_config_t *board,
                                                     const char *name);

/**
 * @brief 根据编号获取存储设备配置
 *
 * @param[in] board 板级配置
 * @param[in] id 存储设备编号（第几个）
 *
 * @return 存储设备配置指针，失败返回NULL
 */
const xconfig_storage_cfg_t* XCONFIG_HW_GetStorage(const xconfig_board_config_t *board,
                                                    uint32_t id);

/**
 * @brief 根据名称查找电源域配置
 *
 * @param[in] board 板级配置
 * @param[in] name 电源域名称
 *
 * @return 电源域配置指针，失败返回NULL
 */
const xconfig_power_domain_t* XCONFIG_HW_FindPowerDomain(const xconfig_board_config_t *board,
                                                          const char *name);

/*===========================================================================
 * APP配置查询接口（XCONFIG_APP_*）
 *===========================================================================*/

/**
 * @brief 根据APP名称查找APP配置
 *
 * @param[in] board 板级配置
 * @param[in] app_name APP名称（如"can_gateway"）
 *
 * @return APP配置指针，失败返回NULL
 */
const xconfig_app_config_t* XCONFIG_APP_Find(const xconfig_board_config_t *board,
                                              const char *app_name);

/**
 * @brief 查找APP的外设映射
 *
 * @param[in] app APP配置
 * @param[in] function 功能名称（如"satellite_comm"）
 *
 * @return 外设映射指针，失败返回NULL
 */
const xconfig_app_device_mapping_t* XCONFIG_APP_FindDevice(const xconfig_app_config_t *app,
                                                            const char *function);

/**
 * @brief 根据APP外设映射获取实际的硬件外设配置
 *
 * @param[in] board 板级配置
 * @param[in] mapping 外设映射
 *
 * @return 硬件外设配置指针（需要根据device_type转换类型），失败返回NULL
 */
const void* XCONFIG_APP_GetDeviceByMapping(const xconfig_board_config_t *board,
                                             const xconfig_app_device_mapping_t *mapping);

/*===========================================================================
 * 配置验证
 *===========================================================================*/

/**
 * @brief 验证板级配置的完整性和正确性
 *
 * @param[in] config 板级配置
 *
 * @return OS_SUCCESS 验证通过
 * @return OS_ERROR 验证失败
 */
int32_t XCONFIG_Validate(const xconfig_board_config_t *config);

/**
 * @brief 打印板级配置信息（用于调试）
 *
 * @param[in] config 板级配置
 */
void XCONFIG_Print(const xconfig_board_config_t *config);

#endif /* XCONFIG_API_H */
