/************************************************************************
 * XConfig内部总头文件
 *
 * 功能：
 * - XConfig内部使用的总头文件
 * - 包含所有内部公共头文件和外设头文件
 * - 对外只提供 pcl_api.h
 *
 * 使用方式：
 *   XConfig内部源文件：#include "internal/xconfig.h"
 *   PDL层（对外）：    #include "pcl_api.h"
 *
 * 目录结构：
 *   api/        - 对外API接口（仅pcl_api.h）
 *   internal/   - 内部公共头文件（含xconfig.h）
 *   peripheral/ - 外设私有头文件
 ************************************************************************/

#ifndef PCL_H
#define PCL_H

/* 内部公共头文件 */
#include "pcl_common.h"
#include "pcl_board.h"

/* 外设头文件 */
#include "../peripheral/pcl_hardware_interface.h"
#include "../peripheral/pcl_mcu.h"
#include "../peripheral/pcl_bmc.h"
#include "../peripheral/pcl_satellite.h"
#include "../peripheral/pcl_sensor.h"
#include "../peripheral/pcl_storage.h"
#include "../peripheral/pcl_app.h"

#endif /* PCL_H */
