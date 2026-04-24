/************************************************************************
 * OSAL Linux实现 - 互斥锁
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
    char          name[OS_MAX_API_NAME];
    pthread_mutex_t mutex;
    bool          valid;
} OS_mutex_record_t;

static OS_mutex_record_t OS_mutex_table[OS_MAX_MUTEXES];
static pthread_mutex_t   mutex_table_mutex = PTHREAD_MUTEX_INITIALIZER;
static uint32            next_mutex_id = 1;

static uint32 deadlock_threshold_msec = 5000;
static deadlock_callback_t deadlock_callback = NULL;

void OS_MutexTableInit(void)
{
    pthread_mutex_lock(&mutex_table_mutex);
    memset(OS_mutex_table, 0, sizeof(OS_mutex_table));
    next_mutex_id = 1;
    pthread_mutex_unlock(&mutex_table_mutex);
}

int32 OS_MutexCreate(osal_id_t *mutex_id, const char *mutex_name,
                     uint32 flags __attribute__((unused)))
{
    uint32 slot = 0;
    bool found_slot = false;

    if (mutex_id == NULL)
        return OS_INVALID_POINTER;

    if (mutex_name == NULL || strlen(mutex_name) >= OS_MAX_API_NAME)
        return OS_ERR_NAME_TOO_LONG;

    pthread_mutex_lock(&mutex_table_mutex);

    for (uint32 i = 0; i < OS_MAX_MUTEXES; i++)
    {
        if (!OS_mutex_table[i].is_used)
        {
            slot = i;
            found_slot = true;
            break;
        }
    }

    if (!found_slot)
    {
        pthread_mutex_unlock(&mutex_table_mutex);
        return OS_ERR_NO_FREE_IDS;
    }

    for (uint32 i = 0; i < OS_MAX_MUTEXES; i++)
    {
        if (OS_mutex_table[i].is_used &&
            strcmp(OS_mutex_table[i].name, mutex_name) == 0)
        {
            pthread_mutex_unlock(&mutex_table_mutex);
            return OS_ERR_NAME_TAKEN;
        }
    }

    if (pthread_mutex_init(&OS_mutex_table[slot].mutex, NULL) != 0)
    {
        pthread_mutex_unlock(&mutex_table_mutex);
        return OS_ERROR;
    }

    OS_mutex_table[slot].is_used = true;
    OS_mutex_table[slot].id = next_mutex_id++;
    OS_mutex_table[slot].valid = true;
    strncpy(OS_mutex_table[slot].name, mutex_name, OS_MAX_API_NAME - 1);
    OS_mutex_table[slot].name[OS_MAX_API_NAME - 1] = '\0';

    *mutex_id = OS_mutex_table[slot].id;

    pthread_mutex_unlock(&mutex_table_mutex);
    return OS_SUCCESS;
}

int32 OS_MutexDelete(osal_id_t mutex_id)
{
    pthread_mutex_t *mutex_to_destroy = NULL;
    uint32 slot_to_clear = OS_MAX_MUTEXES;

    pthread_mutex_lock(&mutex_table_mutex);

    for (uint32 i = 0; i < OS_MAX_MUTEXES; i++)
    {
        if (OS_mutex_table[i].is_used && OS_mutex_table[i].id == mutex_id)
        {
            OS_mutex_table[i].valid = false;
            mutex_to_destroy = &OS_mutex_table[i].mutex;
            slot_to_clear = i;
            break;
        }
    }

    pthread_mutex_unlock(&mutex_table_mutex);

    if (mutex_to_destroy == NULL)
        return OS_ERR_INVALID_ID;

    /* 在锁外销毁互斥锁 */
    pthread_mutex_destroy(mutex_to_destroy);

    /* 清理表项 */
    pthread_mutex_lock(&mutex_table_mutex);
    if (slot_to_clear < OS_MAX_MUTEXES)
    {
        OS_mutex_table[slot_to_clear].is_used = false;
    }
    pthread_mutex_unlock(&mutex_table_mutex);

    return OS_SUCCESS;
}

int32 OS_MutexLock(osal_id_t mutex_id)
{
    pthread_mutex_t *target_mutex = NULL;
    bool is_valid = false;

    /* 查找互斥锁 */
    pthread_mutex_lock(&mutex_table_mutex);

    for (uint32 i = 0; i < OS_MAX_MUTEXES; i++)
    {
        if (OS_mutex_table[i].is_used &&
            OS_mutex_table[i].id == mutex_id &&
            OS_mutex_table[i].valid)
        {
            target_mutex = &OS_mutex_table[i].mutex;
            is_valid = true;
            break;
        }
    }

    pthread_mutex_unlock(&mutex_table_mutex);

    if (!is_valid || target_mutex == NULL)
        return OS_ERR_INVALID_ID;

    /* 在锁外获取用户互斥锁 */
    if (pthread_mutex_lock(target_mutex) != 0)
        return OS_ERROR;

    return OS_SUCCESS;
}

int32 OS_MutexUnlock(osal_id_t mutex_id)
{
    pthread_mutex_t *target_mutex = NULL;
    bool is_valid = false;

    /* 查找互斥锁 */
    pthread_mutex_lock(&mutex_table_mutex);

    for (uint32 i = 0; i < OS_MAX_MUTEXES; i++)
    {
        if (OS_mutex_table[i].is_used &&
            OS_mutex_table[i].id == mutex_id &&
            OS_mutex_table[i].valid)
        {
            target_mutex = &OS_mutex_table[i].mutex;
            is_valid = true;
            break;
        }
    }

    pthread_mutex_unlock(&mutex_table_mutex);

    if (!is_valid || target_mutex == NULL)
        return OS_ERR_INVALID_ID;

    /* 在锁外释放用户互斥锁 */
    if (pthread_mutex_unlock(target_mutex) != 0)
        return OS_ERROR;

    return OS_SUCCESS;
}

int32 OS_MutexGetIdByName(osal_id_t *mutex_id, const char *mutex_name)
{
    if (mutex_id == NULL || mutex_name == NULL)
        return OS_INVALID_POINTER;

    pthread_mutex_lock(&mutex_table_mutex);

    for (uint32 i = 0; i < OS_MAX_MUTEXES; i++)
    {
        if (OS_mutex_table[i].is_used &&
            strcmp(OS_mutex_table[i].name, mutex_name) == 0)
        {
            *mutex_id = OS_mutex_table[i].id;
            pthread_mutex_unlock(&mutex_table_mutex);
            return OS_SUCCESS;
        }
    }

    pthread_mutex_unlock(&mutex_table_mutex);
    return OS_ERR_NAME_NOT_FOUND;
}

int32 OS_MutexLockTimeout(osal_id_t mutex_id, uint32 timeout_msec)
{
    pthread_mutex_t *target_mutex = NULL;
    bool is_valid = false;
    char mutex_name[OS_MAX_API_NAME] = {0};
    struct timespec start_time, current_time, abs_timeout;
    int ret;

    pthread_mutex_lock(&mutex_table_mutex);

    for (uint32 i = 0; i < OS_MAX_MUTEXES; i++)
    {
        if (OS_mutex_table[i].is_used &&
            OS_mutex_table[i].id == mutex_id &&
            OS_mutex_table[i].valid)
        {
            target_mutex = &OS_mutex_table[i].mutex;
            strncpy(mutex_name, OS_mutex_table[i].name, OS_MAX_API_NAME - 1);
            is_valid = true;
            break;
        }
    }

    pthread_mutex_unlock(&mutex_table_mutex);

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
        uint32 wait_time = (current_time.tv_sec - start_time.tv_sec) * 1000 +
                          (current_time.tv_nsec - start_time.tv_nsec) / 1000000;

        if (wait_time >= deadlock_threshold_msec && deadlock_callback != NULL)
        {
            deadlock_callback(mutex_name, wait_time);
        }

        return OS_ERROR_TIMEOUT;
    }
    else if (ret != 0)
    {
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

int32 OS_MutexSetDeadlockDetection(uint32 threshold_msec, deadlock_callback_t callback)
{
    deadlock_threshold_msec = threshold_msec;
    deadlock_callback = callback;
    return OS_SUCCESS;
}
