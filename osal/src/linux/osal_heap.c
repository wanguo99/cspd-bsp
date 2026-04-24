/************************************************************************
 * OSAL Linux实现 - 内存管理
 ************************************************************************/

#include "osal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

typedef struct {
    uint32 threshold_percent;
    uint32 peak_usage;
    uint32 current_usage;
    bool alert_triggered;
    pthread_mutex_t lock;
} heap_monitor_t;

static heap_monitor_t g_heap_monitor = {
    .threshold_percent = 80,
    .peak_usage = 0,
    .current_usage = 0,
    .alert_triggered = false,
};

void OS_HeapInit(void)
{
    pthread_mutex_init(&g_heap_monitor.lock, NULL);
}

static uint32 read_memory_from_proc(const char *field)
{
    FILE *fp = fopen("/proc/self/status", "r");
    if (fp == NULL)
        return 0;

    char line[256];
    uint32 value = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strncmp(line, field, strlen(field)) == 0) {
            sscanf(line, "%*s %u", &value);
            break;
        }
    }
    fclose(fp);
    return value * 1024;
}

int32 OSAL_HeapGetInfo(uint32 *free_bytes, uint32 *total_bytes)
{
    uint32 vm_rss = read_memory_from_proc("VmRSS:");
    uint32 vm_peak = read_memory_from_proc("VmPeak:");

    pthread_mutex_lock(&g_heap_monitor.lock);
    g_heap_monitor.current_usage = vm_rss;
    if (vm_rss > g_heap_monitor.peak_usage)
        g_heap_monitor.peak_usage = vm_rss;
    pthread_mutex_unlock(&g_heap_monitor.lock);

    if (free_bytes != NULL)
        *free_bytes = (vm_peak > vm_rss) ? (vm_peak - vm_rss) : 0;
    if (total_bytes != NULL)
        *total_bytes = vm_peak;

    return OS_SUCCESS;
}

int32 OSAL_HeapSetThreshold(uint32 percent)
{
    if (percent > 100)
        return OS_ERR_INVALID_SIZE;

    pthread_mutex_lock(&g_heap_monitor.lock);
    g_heap_monitor.threshold_percent = percent;
    pthread_mutex_unlock(&g_heap_monitor.lock);

    return OS_SUCCESS;
}

int32 OSAL_HeapCheckThreshold(bool *exceeded)
{
    if (exceeded == NULL)
        return OS_INVALID_POINTER;

    uint32 free_bytes, total_bytes;
    OSAL_HeapGetInfo(&free_bytes, &total_bytes);

    if (total_bytes == 0) {
        *exceeded = false;
        return OS_SUCCESS;
    }

    uint32 usage_percent = ((total_bytes - free_bytes) * 100) / total_bytes;

    pthread_mutex_lock(&g_heap_monitor.lock);
    *exceeded = (usage_percent >= g_heap_monitor.threshold_percent);
    if (*exceeded && !g_heap_monitor.alert_triggered) {
        g_heap_monitor.alert_triggered = true;
        OSAL_Printf("[HEAP] Memory threshold exceeded: %u%% (threshold: %u%%)\n",
                  usage_percent, g_heap_monitor.threshold_percent);
    } else if (!*exceeded) {
        g_heap_monitor.alert_triggered = false;
    }
    pthread_mutex_unlock(&g_heap_monitor.lock);

    return OS_SUCCESS;
}

int32 OSAL_HeapGetStats(uint32 *current, uint32 *peak)
{
    if (current == NULL || peak == NULL)
        return OS_INVALID_POINTER;

    pthread_mutex_lock(&g_heap_monitor.lock);
    *current = g_heap_monitor.current_usage;
    *peak = g_heap_monitor.peak_usage;
    pthread_mutex_unlock(&g_heap_monitor.lock);

    return OS_SUCCESS;
}
