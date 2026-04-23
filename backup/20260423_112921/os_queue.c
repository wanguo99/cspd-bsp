/************************************************************************
 * OSAL Linux/POSIX实现 - 消息队列
 ************************************************************************/

#include "osal.h"
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

/*
 * 简化的消息队列实现(使用环形缓冲区)
 */
typedef struct
{
    uint8  *buffer;
    uint32  head;
    uint32  tail;
    uint32  count;
    uint32  depth;
    uint32  msg_size;
    pthread_mutex_t mutex;
    pthread_cond_t  not_empty;
    pthread_cond_t  not_full;
} queue_impl_t;

typedef struct
{
    bool        is_used;
    osal_id_t   id;
    char        name[OS_MAX_API_NAME];
    queue_impl_t *impl;
} OS_queue_record_t;

static OS_queue_record_t OS_queue_table[OS_MAX_QUEUES];
static pthread_mutex_t   queue_table_mutex = PTHREAD_MUTEX_INITIALIZER;
static uint32            next_queue_id = 1;

/*
 * 初始化队列表
 */
void OS_QueueTableInit(void)
{
    pthread_mutex_lock(&queue_table_mutex);
    memset(OS_queue_table, 0, sizeof(OS_queue_table));
    next_queue_id = 1;
    pthread_mutex_unlock(&queue_table_mutex);
}

static int32 find_free_queue_slot(uint32 *slot)
{
    pthread_mutex_lock(&queue_table_mutex);

    for (uint32 i = 0; i < OS_MAX_QUEUES; i++)
    {
        if (!OS_queue_table[i].is_used)
        {
            *slot = i;
            pthread_mutex_unlock(&queue_table_mutex);
            return OS_SUCCESS;
        }
    }

    pthread_mutex_unlock(&queue_table_mutex);
    return OS_ERR_NO_FREE_IDS;
}

int32 OS_QueueCreate(osal_id_t *queue_id,
                     const char *queue_name,
                     uint32 queue_depth,
                     uint32 data_size,
                     uint32 flags __attribute__((unused)))
{
    uint32 slot;
    int32 ret;
    queue_impl_t *impl;

    if (queue_id == NULL)
        return OS_INVALID_POINTER;

    if (queue_name == NULL || strlen(queue_name) >= OS_MAX_API_NAME)
        return OS_ERR_NAME_TOO_LONG;

    if (queue_depth == 0 || data_size == 0)
        return OS_QUEUE_INVALID_SIZE;

    ret = find_free_queue_slot(&slot);
    if (ret != OS_SUCCESS)
        return ret;

    /* 检查名称冲突 */
    pthread_mutex_lock(&queue_table_mutex);
    for (uint32 i = 0; i < OS_MAX_QUEUES; i++)
    {
        if (OS_queue_table[i].is_used &&
            strcmp(OS_queue_table[i].name, queue_name) == 0)
        {
            pthread_mutex_unlock(&queue_table_mutex);
            return OS_ERR_NAME_TAKEN;
        }
    }

    /* 分配队列实现 */
    impl = malloc(sizeof(queue_impl_t));
    if (impl == NULL)
    {
        pthread_mutex_unlock(&queue_table_mutex);
        return OS_ERROR;
    }

    impl->buffer = malloc(queue_depth * data_size);
    if (impl->buffer == NULL)
    {
        free(impl);
        pthread_mutex_unlock(&queue_table_mutex);
        return OS_ERROR;
    }

    impl->head = 0;
    impl->tail = 0;
    impl->count = 0;
    impl->depth = queue_depth;
    impl->msg_size = data_size;
    pthread_mutex_init(&impl->mutex, NULL);
    pthread_cond_init(&impl->not_empty, NULL);
    pthread_cond_init(&impl->not_full, NULL);

    /* 填充队列表 */
    OS_queue_table[slot].is_used = true;
    OS_queue_table[slot].id = next_queue_id++;
    strncpy(OS_queue_table[slot].name, queue_name, OS_MAX_API_NAME - 1);
    OS_queue_table[slot].impl = impl;

    *queue_id = OS_queue_table[slot].id;

    pthread_mutex_unlock(&queue_table_mutex);

    return OS_SUCCESS;
}

int32 OS_QueueDelete(osal_id_t queue_id)
{
    pthread_mutex_lock(&queue_table_mutex);

    for (uint32 i = 0; i < OS_MAX_QUEUES; i++)
    {
        if (OS_queue_table[i].is_used && OS_queue_table[i].id == queue_id)
        {
            queue_impl_t *impl = OS_queue_table[i].impl;

            pthread_mutex_destroy(&impl->mutex);
            pthread_cond_destroy(&impl->not_empty);
            pthread_cond_destroy(&impl->not_full);
            free(impl->buffer);
            free(impl);

            OS_queue_table[i].is_used = false;
            pthread_mutex_unlock(&queue_table_mutex);
            return OS_SUCCESS;
        }
    }

    pthread_mutex_unlock(&queue_table_mutex);
    return OS_ERR_INVALID_ID;
}

int32 OS_QueuePut(osal_id_t queue_id, const void *data, uint32 size, uint32 flags __attribute__((unused)))
{
    queue_impl_t *impl = NULL;

    if (data == NULL)
        return OS_INVALID_POINTER;

    /* 查找队列 */
    pthread_mutex_lock(&queue_table_mutex);
    for (uint32 i = 0; i < OS_MAX_QUEUES; i++)
    {
        if (OS_queue_table[i].is_used && OS_queue_table[i].id == queue_id)
        {
            impl = OS_queue_table[i].impl;
            break;
        }
    }
    pthread_mutex_unlock(&queue_table_mutex);

    if (impl == NULL)
        return OS_ERR_INVALID_ID;

    if (size > impl->msg_size)
        return OS_QUEUE_INVALID_SIZE;

    pthread_mutex_lock(&impl->mutex);

    /* 等待队列非满 */
    while (impl->count >= impl->depth)
    {
        pthread_cond_wait(&impl->not_full, &impl->mutex);
    }

    /* 写入消息 */
    memcpy(impl->buffer + (impl->tail * impl->msg_size), data, size);
    impl->tail = (impl->tail + 1) % impl->depth;
    impl->count++;

    pthread_cond_signal(&impl->not_empty);
    pthread_mutex_unlock(&impl->mutex);

    return OS_SUCCESS;
}

int32 OS_QueueGet(osal_id_t queue_id, void *data, uint32 size,
                  uint32 *size_copied, int32 timeout)
{
    queue_impl_t *impl = NULL;
    struct timespec ts;
    int ret;

    if (data == NULL)
        return OS_INVALID_POINTER;

    /* 查找队列 */
    pthread_mutex_lock(&queue_table_mutex);
    for (uint32 i = 0; i < OS_MAX_QUEUES; i++)
    {
        if (OS_queue_table[i].is_used && OS_queue_table[i].id == queue_id)
        {
            impl = OS_queue_table[i].impl;
            break;
        }
    }
    pthread_mutex_unlock(&queue_table_mutex);

    if (impl == NULL)
        return OS_ERR_INVALID_ID;

    pthread_mutex_lock(&impl->mutex);

    /* 处理超时 */
    if (timeout == OS_CHECK)
    {
        /* 非阻塞 */
        if (impl->count == 0)
        {
            pthread_mutex_unlock(&impl->mutex);
            return OS_QUEUE_EMPTY;
        }
    }
    else if (timeout > 0)
    {
        /* 超时等待 */
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += timeout / 1000;
        ts.tv_nsec += (timeout % 1000) * 1000000;
        if (ts.tv_nsec >= 1000000000)
        {
            ts.tv_sec++;
            ts.tv_nsec -= 1000000000;
        }

        while (impl->count == 0)
        {
            ret = pthread_cond_timedwait(&impl->not_empty, &impl->mutex, &ts);
            if (ret == ETIMEDOUT)
            {
                pthread_mutex_unlock(&impl->mutex);
                return OS_QUEUE_TIMEOUT;
            }
        }
    }
    else
    {
        /* 永久等待 */
        while (impl->count == 0)
        {
            pthread_cond_wait(&impl->not_empty, &impl->mutex);
        }
    }

    /* 读取消息 */
    uint32 copy_size = (size < impl->msg_size) ? size : impl->msg_size;
    memcpy(data, impl->buffer + (impl->head * impl->msg_size), copy_size);
    impl->head = (impl->head + 1) % impl->depth;
    impl->count--;

    if (size_copied != NULL)
        *size_copied = copy_size;

    pthread_cond_signal(&impl->not_full);
    pthread_mutex_unlock(&impl->mutex);

    return OS_SUCCESS;
}

int32 OS_QueueGetIdByName(osal_id_t *queue_id, const char *queue_name)
{
    if (queue_id == NULL || queue_name == NULL)
        return OS_INVALID_POINTER;

    pthread_mutex_lock(&queue_table_mutex);

    for (uint32 i = 0; i < OS_MAX_QUEUES; i++)
    {
        if (OS_queue_table[i].is_used &&
            strcmp(OS_queue_table[i].name, queue_name) == 0)
        {
            *queue_id = OS_queue_table[i].id;
            pthread_mutex_unlock(&queue_table_mutex);
            return OS_SUCCESS;
        }
    }

    pthread_mutex_unlock(&queue_table_mutex);
    return OS_ERR_NAME_NOT_FOUND;
}
