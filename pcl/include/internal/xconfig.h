/************************************************************************
 * XConfig内部总头文件
 *
 * 功能：
 * - XConfig内部使用的总头文件
 * - 包含所有内部公共头文件和外设头文件
 * - 对外只提供 xconfig_api.h
 *
 * 使用方式：
 *   XConfig内部源文件：#include "internal/xconfig.h"
 *   PDL层（对外）：    #include "xconfig_api.h"
 *
 * 目录结构：
 *   api/        - 对外API接口（仅xconfig_api.h）
 *   internal/   - 内部公共头文件（含xconfig.h）
 *   peripheral/ - 外设私有头文件
 ************************************************************************/

#ifndef XCONFIG_H
#define XCONFIG_H

/* 内部公共头文件 */
#include "xconfig_common.h"
#include "xconfig_board.h"

/* 外设头文件 */
#include "../peripheral/xconfig_hardware_interface.h"
#include "../peripheral/xconfig_mcu.h"
#include "../peripheral/xconfig_bmc.h"
#include "../peripheral/xconfig_satellite.h"
#include "../peripheral/xconfig_sensor.h"
#include "../peripheral/xconfig_storage.h"
#include "../peripheral/xconfig_app.h"

#endif /* XCONFIG_H */
