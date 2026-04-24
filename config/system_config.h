/************************************************************************
 * 系统配置文件（主入口）
 *
 * 包含所有子配置文件
 ************************************************************************/

#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

/*
 * ============================================================================
 * 系统基础配置
 * ============================================================================
 */

/* 系统版本 */
#define SYSTEM_VERSION_MAJOR    1
#define SYSTEM_VERSION_MINOR    0
#define SYSTEM_VERSION_PATCH    0

/* 系统名称 */
#define SYSTEM_NAME             "CSPD-BSP"

/* 调试模式 */
#define DEBUG_MODE              1

/*
 * ============================================================================
 * 包含子配置文件
 * ============================================================================
 */

/* 硬件配置 */
#include "hardware/can_config.h"
#include "hardware/ethernet_config.h"
#include "hardware/uart_config.h"

/* 系统配置 */
#include "system/task_config.h"
#include "system/queue_config.h"
#include "system/log_config.h"
#include "system/watchdog_config.h"

/* 协议定义 */
#include "protocol/can_protocol.h"

/*
 * ============================================================================
 * 协议转换配置
 * ============================================================================
 */

/* 命令超时时间(ms) */
#define CMD_TIMEOUT_MS          10000

/* 最大重试次数 */
#define CMD_RETRY_COUNT         3

/*
 * ============================================================================
 * 状态监控配置
 * ============================================================================
 */

/* 状态查询周期(ms) */
#define STATUS_QUERY_PERIOD_MS  5000

/* 状态缓存时间(ms) */
#define STATUS_CACHE_TIME_MS    10000

/*
 * ============================================================================
 * 数据缓存配置
 * ============================================================================
 */

/* 缓存文件路径 */
#define CACHE_FILE_PATH         "/var/cache/cspd-bsp.cache"

/* 缓存最大条目数 */
#define CACHE_MAX_ENTRIES       1000

/* 缓存持久化周期(ms) */
#define CACHE_PERSIST_PERIOD_MS 60000

#endif /* SYSTEM_CONFIG_H */
