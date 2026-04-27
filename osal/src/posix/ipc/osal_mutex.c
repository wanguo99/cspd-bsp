/************************************************************************
 * OSAL POSIX实现 - 互斥锁
 ************************************************************************/

#include "osal.h"
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

typedef struct
{
    bool          is_used;
    osal_id_t     id;
    str_t         name[OS_MAX_API_NAME];
    pthread_mutex_t mutex;
    bool          valid;
} osal_mutex_record_t;

static osal_mutex_record_t g_osal_mutex_table[OS_MAX_MUTEXES] = {0};  /* 静态变量自动初始化为0 */
static pthread_mutex_t g_mutex_table_mutex = PTHREAD_MUTEX_INITIALIZER;
static uint32_t g_next_mutex_id = 1;

static uint32_t g_deadlock_threshold_msec = 5000;
static deadlock_callback_t g_deadlock_callback = NULL;

/* 移除 osal_mutex_table_init() - 静态变量已自动初始化 */

int32_t OSAL_MutexCreate(osal_id_t *mutex_id, const char *mutex_name,
                     uint32_t flags __attribute__((unused)))
{
    uint32_t slot = 0;
    bool found_slot = false;

    if (NULL == mutex_id)
        return OS_INVALID_POINTER;

    if (mutex_name == NULL || strlen(mutex_name) >= OS_MAX_API_NAME)
        return OS_ERR_NAME_TOO_LONG;

    pthread_mutex_lock(&g_mutex_table_mutex);

    for (uint32_t i = 0; i < OS_MAX_MUTEXES; i++)
    {
        if (!g_osal_mutex_table[i].is_used)
        {
            slot = i;
            found_slot = true;
            break;
        }
    }

    if (!found_slot)
    {
        pthread_mutex_unlock(&g_mutex_table_mutex);
        return OS_ERR_NO_FREE_IDS;
    }

    for (uint32_t i = 0; i < OS_MAX_MUTEXES; i++)
    {
        if (g_osal_mutex_table[i].is_used &&
            strcmp(g_osal_mutex_table[i].name, mutex_name) == 0)
        {
            pthread_mutex_unlock(&g_mutex_table_mutex);
            return OS_ERR_NAME_TAKEN;
        }
    }

    if (pthread_mutex_init(&g_osal_mutex_table[slot].mutex, NULL) != 0)
    {
        pthread_mutex_unlock(&g_mutex_table_mutex);
        return OS_ERROR;
    }

    g_osal_mutex_table[slot].is_used = true;
    g_osal_mutex_table[slot].id = g_next_mutex_id++;
    g_osal_mutex_table[slot].valid = true;
    strncpy(g_osal_mutex_table[slot].name, mutex_name, OS_MAX_API_NAME - 1);
    g_osal_mutex_table[slot].name[OS_MAX_API_NAME - 1] = '\0';

    *mutex_id = g_osal_mutex_table[slot].id;

    pthread_mutex_unlock(&g_mutex_table_mutex);
    return OS_SUCCESS;
}

int32_t OSAL_MutexDelete(osal_id_t mutex_id)
{
    pthread_mutex_t *mutex_to_destroy = NULL;
    uint32_t slot_to_clear = OS_MAX_MUTEXES;

    pthread_mutex_lock(&g_mutex_table_mutex);

    for (uint32_t i = 0; i < OS_MAX_MUTEXES; i++)
    {
        if (g_osal_mutex_table[i].is_used && g_osal_mutex_table[i].id == mutex_id)
        {
            g_osal_mutex_table[i].valid = false;
            mutex_to_destroy = &g_osal_mutex_table[i].mutex;
            slot_to_clear = i;
            break;
        }
    }

    pthread_mutex_unlock(&g_mutex_table_mutex);

    if (NULL == mutex_to_destroy)
        return OS_ERR_INVALID_ID;

    /* 在锁外销毁互斥锁 */
    pthread_mutex_destroy(mutex_to_destroy);

    /* 清理表项 */
    pthread_mutex_lock(&g_mutex_table_mutex);
    if (slot_to_clear < OS_MAX_MUTEXES)
    {
        g_osal_mutex_table[slot_to_clear].is_used = false;
    }
    pthread_mutex_unlock(&g_mutex_table_mutex);

    return OS_SUCCESS;
}

int32_t OSAL_MutexLock(osal_id_t mutex_id)
{
    pthread_mutex_t *target_mutex = NULL;
    bool is_valid = false;

    /* 查找互斥锁 */
    pthread_mutex_lock(&g_mutex_table_mutex);

    for (uint32_t i = 0; i < OS_MAX_MUTEXES; i++)
    {
        if (g_osal_mutex_table[i].is_used &&
            g_osal_mutex_table[i].id == mutex_id &&
            g_osal_mutex_table[i].valid)
        {
            target_mutex = &g_osal_mutex_table[i].mutex;
            is_valid = true;
            break;
        }
    }

    pthread_mutex_unlock(&g_mutex_table_mutex);

    if (!is_valid || target_mutex == NULL)
        return OS_ERR_INVALID_ID;

    /* 在锁外获取用户互斥锁 */
    if (pthread_mutex_lock(target_mutex) != 0)
        return OS_ERROR;

    return OS_SUCCESS;
}

int32_t OSAL_MutexUnlock(osal_id_t mutex_id)
{
    pthread_mutex_t *target_mutex = NULL;
    bool is_valid = false;

    /* 查找互斥锁 */
    pthread_mutex_lock(&g_mutex_table_mutex);

    for (uint32_t i = 0; i < OS_MAX_MUTEXES; i++)
    {
        if (g_osal_mutex_table[i].is_used &&
            g_osal_mutex_table[i].id == mutex_id &&
            g_osal_mutex_table[i].valid)
        {
            target_mutex = &g_osal_mutex_table[i].mutex;
            is_valid = true;
            break;
        }
    }

    pthread_mutex_unlock(&g_mutex_table_mutex);

    if (!is_valid || target_mutex == NULL)
        return OS_ERR_INVALID_ID;

    /* 在锁外释放用户互斥锁 */
    if (pthread_mutex_unlock(target_mutex) != 0)
        return OS_ERROR;

    return OS_SUCCESS;
}

int32_t OSAL_MutexGetIdByName(osal_id_t *mutex_id, const char *mutex_name)
{
    if (mutex_id == NULL || mutex_name == NULL)
        return OS_INVALID_POINTER;

    pthread_mutex_lock(&g_mutex_table_mutex);

    for (uint32_t i = 0; i < OS_MAX_MUTEXES; i++)
    {
        if (g_osal_mutex_table[i].is_used &&
            strcmp(g_osal_mutex_table[i].name, mutex_name) == 0)
        {
            *mutex_id = g_osal_mutex_table[i].id;
            pthread_mutex_unlock(&g_mutex_table_mutex);
            return OS_SUCCESS;
        }
    }

    pthread_mutex_unlock(&g_mutex_table_mutex);
    return OS_ERR_NAME_NOT_FOUND;
}

int32_t OSAL_MutexLockTimeout(osal_id_t mutex_id, uint32_t timeout_msec)
{
    pthread_mutex_t *target_mutex = NULL;
    bool is_valid = false;
    str_t mutex_name[OS_MAX_API_NAME] = {0};
    struct timespec start_time, current_time, abs_timeout;
    int ret;

    pthread_mutex_lock(&g_mutex_table_mutex);

    for (uint32_t i = 0; i < OS_MAX_MUTEXES; i++)
    {
        if (g_osal_mutex_table[i].is_used &&
            g_osal_mutex_table[i].id == mutex_id &&
            g_osal_mutex_table[i].valid)
        {
            target_mutex = &g_osal_mutex_table[i].mutex;
            strncpy(mutex_name, g_osal_mutex_table[i].name, OS_MAX_API_NAME - 1);
            is_valid = true;
            break;
        }
    }

    pthread_mutex_unlock(&g_mutex_table_mutex);

    if (!is_valid || target_mutex == NULL)
        return OS_ERR_INVALID_ID;

    clock_gettime(CLOCK_REALTIME, &start_time);
    abs_timeout.tv_sec = start_time.tv_sec + timeout_msec / 1000;
    abs_timeout.tv_nsec = start_time.tv_nsec + (timeout_msec % 1000) * 1000000;
    if (abs_timeout.tv_nsec >= 1000000000)
    {
        abs_timeout.tv_sec++;
        abs_timeout.tv_nsec -= 1000000000;
    }

    ret = pthread_mutex_timedlock(target_mutex, &abs_timeout);

    if (ret == ETIMEDOUT)
    {
        clock_gettime(CLOCK_REALTIME, &current_time);
        uint32_t wait_time = (current_time.tv_sec - start_time.tv_sec) * 1000 +
                          (current_time.tv_nsec - start_time.tv_nsec) / 1000000;

        /* 读取死锁检测配置时加锁保护 */
        pthread_mutex_lock(&g_mutex_table_mutex);
        uint32_t threshold = g_deadlock_threshold_msec;
        deadlock_callback_t callback = g_deadlock_callback;
        pthread_mutex_unlock(&g_mutex_table_mutex);

        if (wait_time >= threshold && NULL != callback)
        {
            callback(mutex_name, wait_time);
        }

        return OS_ERROR_TIMEOUT;
    }
    else if (0 != ret)
    {
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

int32_t OSAL_MutexSetDeadlockDetection(uint32_t threshold_msec, deadlock_callback_t callback)
{
    pthread_mutex_lock(&g_mutex_table_mutex);
    g_deadlock_threshold_msec = threshold_msec;
    g_deadlock_callback = callback;
    pthread_mutex_unlock(&g_mutex_table_mutex);
    return OS_SUCCESS;
}
