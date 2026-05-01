/************************************************************************
 * OSAL - 资源泄漏检测
 *
 * 功能：
 * - 跟踪OSAL资源的创建和销毁（任务、队列、互斥锁等）
 * - 检测资源泄漏
 * - 提供资源使用统计
 *
 * 设计原则：
 * - 仅在调试模式下启用（通过OSAL_ENABLE_RESOURCE_TRACKING宏控制）
 * - 线程安全
 * - 最小性能开销
 ************************************************************************/

#ifndef OSAL_RESOURCE_TRACKER_H
#define OSAL_RESOURCE_TRACKER_H

#include "osal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 资源类型 */
typedef enum
{
    OSAL_RESOURCE_TYPE_TASK = 0,
    OSAL_RESOURCE_TYPE_QUEUE,
    OSAL_RESOURCE_TYPE_MUTEX,
    OSAL_RESOURCE_TYPE_MAX
} osal_resource_type_t;

/* 资源统计信息 */
typedef struct
{
    uint32_t total_created;    /* 总创建数 */
    uint32_t total_deleted;    /* 总删除数 */
    uint32_t current_count;    /* 当前活跃数 */
    uint32_t peak_count;       /* 峰值数量 */
} osal_resource_stats_t;

#ifdef OSAL_ENABLE_RESOURCE_TRACKING

/**
 * @brief 注册资源
 * @param id 资源ID
 * @param type 资源类型
 * @param name 资源名称
 * @param file 创建文件
 * @param line 创建行号
 */
void OSAL_ResourceRegister(osal_id_t id, osal_resource_type_t type,
                           const char *name, const char *file, int32_t line);

/**
 * @brief 注销资源
 * @param id 资源ID
 * @param type 资源类型
 */
void OSAL_ResourceUnregister(osal_id_t id, osal_resource_type_t type);

/**
 * @brief 检查资源泄漏
 * @return 泄漏的资源数量
 */
uint32_t OSAL_ResourceCheckLeaks(void);

/**
 * @brief 获取资源统计信息
 * @param type 资源类型
 * @param stats 输出统计信息
 * @return OSAL_SUCCESS 成功，其他值失败
 */
int32_t OSAL_ResourceGetStats(osal_resource_type_t type, osal_resource_stats_t *stats);

/**
 * @brief 打印资源使用报告
 */
void OSAL_ResourcePrintReport(void);

/**
 * @brief 重置资源统计
 */
void OSAL_ResourceResetStats(void);

#else

/* 禁用资源跟踪时，定义为空宏 */
#define OSAL_ResourceRegister(id, type, name, file, line) ((void)0)
#define OSAL_ResourceUnregister(id, type) ((void)0)
#define OSAL_ResourceCheckLeaks() (0)
#define OSAL_ResourceGetStats(type, stats) (OSAL_SUCCESS)
#define OSAL_ResourcePrintReport() ((void)0)
#define OSAL_ResourceResetStats() ((void)0)

#endif /* OSAL_ENABLE_RESOURCE_TRACKING */

#ifdef __cplusplus
}
#endif

#endif /* OSAL_RESOURCE_TRACKER_H */
