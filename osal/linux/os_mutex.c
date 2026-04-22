/************************************************************************
 * OSAL Linux实现 - 互斥锁
 ************************************************************************/

#include "osal.h"
#include <pthread.h>
#include <string.h>
#include <stdbool.h>

/*
 * 互斥锁表
 */
typedef struct
{
    bool          is_used;
    osal_id_t     id;
    char          name[OS_MAX_API_NAME];
    pthread_mutex_t mutex;
} OS_mutex_record_t;

static OS_mutex_record_t OS_mutex_table[OS_MAX_MUTEXES];
static pthread_mutex_t   mutex_table_mutex = PTHREAD_MUTEX_INITIALIZER;
static uint32            next_mutex_id = 1;

/* 互斥锁API */
int32 OS_MutexCreate(osal_id_t *mutex_id, const char *mutex_name,
                     uint32 flags __attribute__((unused)))
{
    uint32 slot = 0;

    if (mutex_id == NULL)
        return OS_INVALID_POINTER;

    if (mutex_name == NULL || strlen(mutex_name) >= OS_MAX_API_NAME)
        return OS_ERR_NAME_TOO_LONG;

    pthread_mutex_lock(&mutex_table_mutex);

    /* 查找空闲槽 */
    for (uint32 i = 0; i < OS_MAX_MUTEXES; i++)
    {
        if (!OS_mutex_table[i].is_used)
        {
            slot = i;
            break;
        }
        if (i == OS_MAX_MUTEXES - 1)
        {
            pthread_mutex_unlock(&mutex_table_mutex);
            return OS_ERR_NO_FREE_IDS;
        }
    }

    /* 检查名称冲突 */
    for (uint32 i = 0; i < OS_MAX_MUTEXES; i++)
    {
        if (OS_mutex_table[i].is_used &&
            strcmp(OS_mutex_table[i].name, mutex_name) == 0)
        {
            pthread_mutex_unlock(&mutex_table_mutex);
            return OS_ERR_NAME_TAKEN;
        }
    }

    /* 初始化互斥锁 */
    if (pthread_mutex_init(&OS_mutex_table[slot].mutex, NULL) != 0)
    {
        pthread_mutex_unlock(&mutex_table_mutex);
        return OS_ERROR;
    }

    OS_mutex_table[slot].is_used = true;
    OS_mutex_table[slot].id = next_mutex_id++;
    strncpy(OS_mutex_table[slot].name, mutex_name, OS_MAX_API_NAME - 1);

    *mutex_id = OS_mutex_table[slot].id;

    pthread_mutex_unlock(&mutex_table_mutex);
    return OS_SUCCESS;
}

int32 OS_MutexDelete(osal_id_t mutex_id)
{
    pthread_mutex_lock(&mutex_table_mutex);

    for (uint32 i = 0; i < OS_MAX_MUTEXES; i++)
    {
        if (OS_mutex_table[i].is_used && OS_mutex_table[i].id == mutex_id)
        {
            pthread_mutex_destroy(&OS_mutex_table[i].mutex);
            OS_mutex_table[i].is_used = false;
            pthread_mutex_unlock(&mutex_table_mutex);
            return OS_SUCCESS;
        }
    }

    pthread_mutex_unlock(&mutex_table_mutex);
    return OS_ERR_INVALID_ID;
}

int32 OS_MutexLock(osal_id_t mutex_id)
{
    pthread_mutex_lock(&mutex_table_mutex);

    for (uint32 i = 0; i < OS_MAX_MUTEXES; i++)
    {
        if (OS_mutex_table[i].is_used && OS_mutex_table[i].id == mutex_id)
        {
            pthread_mutex_unlock(&mutex_table_mutex);
            pthread_mutex_lock(&OS_mutex_table[i].mutex);
            return OS_SUCCESS;
        }
    }

    pthread_mutex_unlock(&mutex_table_mutex);
    return OS_ERR_INVALID_ID;
}

int32 OS_MutexUnlock(osal_id_t mutex_id)
{
    pthread_mutex_lock(&mutex_table_mutex);

    for (uint32 i = 0; i < OS_MAX_MUTEXES; i++)
    {
        if (OS_mutex_table[i].is_used && OS_mutex_table[i].id == mutex_id)
        {
            pthread_mutex_unlock(&mutex_table_mutex);
            pthread_mutex_unlock(&OS_mutex_table[i].mutex);
            return OS_SUCCESS;
        }
    }

    pthread_mutex_unlock(&mutex_table_mutex);
    return OS_ERR_INVALID_ID;
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

/* 互斥锁表初始化 */
void OS_MutexTableInit(void)
{
    for (uint32 i = 0; i < OS_MAX_MUTEXES; i++)
    {
        OS_mutex_table[i].is_used = false;
    }
}
