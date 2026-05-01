/************************************************************************
 * OSAL POSIX实现 - 资源泄漏检测
 ************************************************************************/

#include "osal.h"
#include "util/osal_resource_tracker.h"
#include <pthread.h>
#include <string.h>

#ifdef OSAL_ENABLE_RESOURCE_TRACKING

/* 最大跟踪资源数 */
#define OSAL_MAX_TRACKED_RESOURCES  256

/* 资源条目 */
typedef struct
{
    bool in_use;
    osal_id_t id;
    osal_resource_type_t type;
    char name[OS_MAX_API_NAME];
    char file[128];
    int32_t line;
} osal_resource_entry_t;

/* 全局资源表 */
static osal_resource_entry_t g_resource_table[OSAL_MAX_TRACKED_RESOURCES];
static pthread_mutex_t g_resource_mutex = PTHREAD_MUTEX_INITIALIZER;

/* 资源统计 */
static osal_resource_stats_t g_resource_stats[OSAL_RESOURCE_TYPE_MAX];

/* 资源类型名称 */
static const char *g_resource_type_names[] = {
    "Task",
    "Queue",
    "Mutex"
};

/**
 * @brief 查找空闲槽位
 */
static int32_t find_free_slot(void)
{
    for (uint32_t i = 0; i < OSAL_MAX_TRACKED_RESOURCES; i++)
    {
        if (!g_resource_table[i].in_use)
        {
            return (int32_t)i;
        }
    }
    return -1;
}

/**
 * @brief 查找资源条目
 */
static int32_t find_resource(osal_id_t id, osal_resource_type_t type)
{
    for (uint32_t i = 0; i < OSAL_MAX_TRACKED_RESOURCES; i++)
    {
        if (g_resource_table[i].in_use &&
            g_resource_table[i].id == id &&
            g_resource_table[i].type == type)
        {
            return (int32_t)i;
        }
    }
    return -1;
}

void OSAL_ResourceRegister(osal_id_t id, osal_resource_type_t type,
                           const char *name, const char *file, int32_t line)
{
    if (type >= OSAL_RESOURCE_TYPE_MAX)
    {
        return;
    }

    pthread_mutex_lock(&g_resource_mutex);

    /* 查找空闲槽位 */
    int32_t slot = find_free_slot();
    if (slot < 0)
    {
        LOG_ERROR("OSAL_ResourceTracker", "Resource table full, cannot track resource");
        pthread_mutex_unlock(&g_resource_mutex);
        return;
    }

    /* 注册资源 */
    g_resource_table[slot].in_use = true;
    g_resource_table[slot].id = id;
    g_resource_table[slot].type = type;
    OSAL_Strncpy(g_resource_table[slot].name, name, OS_MAX_API_NAME - 1);
    OSAL_Strncpy(g_resource_table[slot].file, file, 127);
    g_resource_table[slot].line = line;

    /* 更新统计 */
    g_resource_stats[type].total_created++;
    g_resource_stats[type].current_count++;
    if (g_resource_stats[type].current_count > g_resource_stats[type].peak_count)
    {
        g_resource_stats[type].peak_count = g_resource_stats[type].current_count;
    }

    pthread_mutex_unlock(&g_resource_mutex);
}

void OSAL_ResourceUnregister(osal_id_t id, osal_resource_type_t type)
{
    if (type >= OSAL_RESOURCE_TYPE_MAX)
    {
        return;
    }

    pthread_mutex_lock(&g_resource_mutex);

    /* 查找资源 */
    int32_t slot = find_resource(id, type);
    if (slot < 0)
    {
        LOG_WARN("OSAL_ResourceTracker", "Resource ID %u type %d not found in tracker",
                 (uint32_t)id, type);
        pthread_mutex_unlock(&g_resource_mutex);
        return;
    }

    /* 注销资源 */
    g_resource_table[slot].in_use = false;

    /* 更新统计 */
    g_resource_stats[type].total_deleted++;
    if (g_resource_stats[type].current_count > 0)
    {
        g_resource_stats[type].current_count--;
    }

    pthread_mutex_unlock(&g_resource_mutex);
}

uint32_t OSAL_ResourceCheckLeaks(void)
{
    uint32_t leak_count = 0;

    pthread_mutex_lock(&g_resource_mutex);

    LOG_INFO("OSAL_ResourceTracker", "=== Resource Leak Check ===");

    for (uint32_t i = 0; i < OSAL_MAX_TRACKED_RESOURCES; i++)
    {
        if (g_resource_table[i].in_use)
        {
            LOG_ERROR("OSAL_ResourceTracker", "LEAK: %s '%s' (ID=%u) created at %s:%d",
                     g_resource_type_names[g_resource_table[i].type],
                     g_resource_table[i].name,
                     (uint32_t)g_resource_table[i].id,
                     g_resource_table[i].file,
                     g_resource_table[i].line);
            leak_count++;
        }
    }

    if (leak_count == 0)
    {
        LOG_INFO("OSAL_ResourceTracker", "No resource leaks detected");
    }
    else
    {
        LOG_ERROR("OSAL_ResourceTracker", "Total resource leaks: %u", leak_count);
    }

    pthread_mutex_unlock(&g_resource_mutex);

    return leak_count;
}

int32_t OSAL_ResourceGetStats(osal_resource_type_t type, osal_resource_stats_t *stats)
{
    if (type >= OSAL_RESOURCE_TYPE_MAX || stats == NULL)
    {
        return OSAL_ERR_INVALID_POINTER;
    }

    pthread_mutex_lock(&g_resource_mutex);
    OSAL_Memcpy(stats, &g_resource_stats[type], sizeof(osal_resource_stats_t));
    pthread_mutex_unlock(&g_resource_mutex);

    return OSAL_SUCCESS;
}

void OSAL_ResourcePrintReport(void)
{
    pthread_mutex_lock(&g_resource_mutex);

    LOG_INFO("OSAL_ResourceTracker", "=== Resource Usage Report ===");

    for (uint32_t type = 0; type < OSAL_RESOURCE_TYPE_MAX; type++)
    {
        LOG_INFO("OSAL_ResourceTracker", "%s: Created=%u, Deleted=%u, Current=%u, Peak=%u",
                 g_resource_type_names[type],
                 g_resource_stats[type].total_created,
                 g_resource_stats[type].total_deleted,
                 g_resource_stats[type].current_count,
                 g_resource_stats[type].peak_count);
    }

    pthread_mutex_unlock(&g_resource_mutex);
}

void OSAL_ResourceResetStats(void)
{
    pthread_mutex_lock(&g_resource_mutex);

    OSAL_Memset(g_resource_stats, 0, sizeof(g_resource_stats));

    pthread_mutex_unlock(&g_resource_mutex);
}

#endif /* OSAL_ENABLE_RESOURCE_TRACKING */
