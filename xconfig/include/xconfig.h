/************************************************************************
 * XConfig总头文件
 *
 * 功能：
 * - 提供统一的包含入口
 * - 用户只需包含此文件即可使用所有xconfig功能
 *
 * 使用方式：
 *   #include "xconfig.h"
 ************************************************************************/

#ifndef XCONFIG_H
#define XCONFIG_H

/* 包含所有xconfig头文件 */
#include "xconfig_common.h"
#include "xconfig_hardware_interface.h"
#include "xconfig_mcu.h"
#include "xconfig_bmc.h"
#include "xconfig_satellite.h"
#include "xconfig_app.h"
#include "xconfig_board.h"
#include "xconfig_api.h"

#endif /* XCONFIG_H */
