/************************************************************************
 * OSAL Linux/POSIX实现 - 消息队列（优化版）
 *
 * 修复内容：
 * 1. 修复线程安全问题：使用引用计数保护对象
 * 2. 添加对象生命周期管理
 * 3. 改进错误处理
 ************************************************************************/

#include "osal.h"
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <stdatomic.h>

/*
 * 环形缓冲区消息队列实现
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
    atomic_int      ref_count;  /* 引用计数 */
    bool            valid;      /* 有效标志 */
} queue_impl_t;

typedef struct
{
    bool        is_used;
    osal_id_t   id;
    str_t       name[OS_MAX_API_NAME];
    queue_impl_t *impl;
} osal_queue_record_t;

static osal_queue_record_t g_osal_queue_table[OS_MAX_QUEUES];
static pthread_mutex_t   g_queue_table_mutex = PTHREAD_MUTEX_INITIALIZER;
static uint32            g_next_queue_id = 1;

/*
 * 初始化队列表
 */
void osal_queue_table_init(void)
{
    pthread_mutex_lock(&g_queue_table_mutex);
    memset(g_osal_queue_table, 0, sizeof(g_osal_queue_table));
    g_next_queue_id = 1;
    pthread_mutex_unlock(&g_queue_table_mutex);
}

/*
 * 增加引用计数（必须在持有table锁时调用）
 */
static queue_impl_t* osal_queue_acquire(osal_id_t queue_id)
{
    for (uint32 i = 0; i < OS_MAX_QUEUES; i++)
    {
        if (g_osal_queue_table[i].is_used &&
            g_osal_queue_table[i].id == queue_id &&
            g_osal_queue_table[i].impl != NULL)
        {
            queue_impl_t *impl = g_osal_queue_table[i].impl;
            if (impl->valid)
            {
                atomic_fetch_add(&impl->ref_count, 1);
                return impl;
            }
        }
    }
    return NULL;
}

/*
 * 减少引用计数并在必要时释放资源
 */
static void osal_queue_release(queue_impl_t *impl)
{
    if (impl == NULL) return;

    int old_count = atomic_fetch_sub(&impl->ref_count, 1);

    /* 如果引用计数降为0且对象已标记为无效，则释放资源 */
    if (old_count == 1 && !impl->valid)
    {
        pthread_mutex_destroy(&impl->mutex);
        pthread_cond_destroy(&impl->not_empty);
        pthread_cond_destroy(&impl->not_full);
        free(impl->buffer);
        free(impl);
    }
}

static int32 osal_queue_find_free_slot(uint32 *slot)
{
    pthread_mutex_lock(&g_queue_table_mutex);

    for (uint32 i = 0; i < OS_MAX_QUEUES; i++)
    {
        if (!g_osal_queue_table[i].is_used)
        {
            *slot = i;
            pthread_mutex_unlock(&g_queue_table_mutex);
            return OS_SUCCESS;
        }
    }

    pthread_mutex_unlock(&g_queue_table_mutex);
    return OS_ERR_NO_FREE_IDS;
}

int32 OSAL_QueueCreate(osal_id_t *queue_id,
                     const char *queue_name,
                     uint32 queue_depth,
                     uint32 data_size,
                     uint32 flags __attribute__((unused)))
{
    uint32 slot;
    int32 ret;
    queue_impl_t *impl;

    /* 参数检查 */
    if (queue_id == NULL)
        return OS_INVALID_POINTER;

    if (queue_name == NULL || strlen(queue_name) >= OS_MAX_API_NAME)
        return OS_ERR_NAME_TOO_LONG;

    if (queue_depth == 0 || data_size == 0)
        return OS_QUEUE_INVALID_SIZE;

    /* 限制队列深度和消息大小，防止过度内存分配 */
    if (queue_depth > 10000 || data_size > 65536)
        return OS_QUEUE_INVALID_SIZE;

    /* 检查乘法溢出 */
    if (queue_depth > 0 && data_size > 0)
    {
        if (queue_depth > UINT32_MAX / data_size)
        {
            return OS_QUEUE_INVALID_SIZE;  /* 溢出风险 */
        }
    }

    ret = osal_queue_find_free_slot(&slot);
    if (ret != OS_SUCCESS)
        return ret;

    /* 检查名称冲突 */
    pthread_mutex_lock(&g_queue_table_mutex);
    for (uint32 i = 0; i < OS_MAX_QUEUES; i++)
    {
        if (g_osal_queue_table[i].is_used &&
            strcmp(g_osal_queue_table[i].name, queue_name) == 0)
        {
            pthread_mutex_unlock(&g_queue_table_mutex);
            return OS_ERR_NAME_TAKEN;
        }
    }

    /* 分配队列实现 */
    impl = malloc(sizeof(queue_impl_t));
    if (impl == NULL)
    {
        pthread_mutex_unlock(&g_queue_table_mutex);
        return OS_ERR_NO_MEMORY;
    }

    memset(impl, 0, sizeof(queue_impl_t));

    /* 安全的内存分配（已检查溢出） */
    size_t buffer_size = (size_t)queue_depth * (size_t)data_size;
    impl->buffer = malloc(buffer_size);
    if (impl->buffer == NULL)
    {
        free(impl);
        pthread_mutex_unlock(&g_queue_table_mutex);
        return OS_ERR_NO_MEMORY;
    }

    impl->head = 0;
    impl->tail = 0;
    impl->count = 0;
    impl->depth = queue_depth;
    impl->msg_size = data_size;
    atomic_init(&impl->ref_count, 1);  /* 初始引用计数为1 */
    impl->valid = true;

    pthread_mutex_init(&impl->mutex, NULL);
    pthread_cond_init(&impl->not_empty, NULL);
    pthread_cond_init(&impl->not_full, NULL);

    /* 填充队列表 */
    g_osal_queue_table[slot].is_used = true;
    g_osal_queue_table[slot].id = g_next_queue_id++;
    strncpy(g_osal_queue_table[slot].name, queue_name, OS_MAX_API_NAME - 1);
    g_osal_queue_table[slot].name[OS_MAX_API_NAME - 1] = '\0';
    g_osal_queue_table[slot].impl = impl;

    *queue_id = g_osal_queue_table[slot].id;

    pthread_mutex_unlock(&g_queue_table_mutex);

    return OS_SUCCESS;
}

int32 OSAL_QueueDelete(osal_id_t queue_id)
{
    queue_impl_t *impl = NULL;

    pthread_mutex_lock(&g_queue_table_mutex);

    for (uint32 i = 0; i < OS_MAX_QUEUES; i++)
    {
        if (g_osal_queue_table[i].is_used && g_osal_queue_table[i].id == queue_id)
        {
            impl = g_osal_queue_table[i].impl;
            g_osal_queue_table[i].is_used = false;
            g_osal_queue_table[i].impl = NULL;
            break;
        }
    }

    pthread_mutex_unlock(&g_queue_table_mutex);

    if (impl == NULL)
        return OS_ERR_INVALID_ID;

    /* 标记为无效并唤醒所有等待线程 */
    pthread_mutex_lock(&impl->mutex);
    impl->valid = false;
    pthread_cond_broadcast(&impl->not_empty);
    pthread_cond_broadcast(&impl->not_full);
    pthread_mutex_unlock(&impl->mutex);

    /* 释放初始引用 */
    osal_queue_release(impl);

    return OS_SUCCESS;
}

int32 OSAL_QueuePut(osal_id_t queue_id, const void *data, uint32 size, uint32 flags __attribute__((unused)))
{
    queue_impl_t *impl = NULL;
    int32 result = OS_SUCCESS;

    if (data == NULL)
        return OS_INVALID_POINTER;

    /* 获取队列并增加引用计数 */
    pthread_mutex_lock(&g_queue_table_mutex);
    impl = osal_queue_acquire(queue_id);
    pthread_mutex_unlock(&g_queue_table_mutex);

    if (impl == NULL)
        return OS_ERR_INVALID_ID;

    if (size > impl->msg_size)
    {
        osal_queue_release(impl);
        return OS_QUEUE_INVALID_SIZE;
    }

    pthread_mutex_lock(&impl->mutex);

    /* 检查队列是否仍然有效 */
    if (!impl->valid)
    {
        pthread_mutex_unlock(&impl->mutex);
        osal_queue_release(impl);
        return OS_ERR_INVALID_ID;
    }

    /* 等待队列非满 */
    while (impl->count >= impl->depth && impl->valid)
    {
        pthread_cond_wait(&impl->not_full, &impl->mutex);
    }

    /* 再次检查有效性 */
    if (!impl->valid)
    {
        result = OS_ERR_INVALID_ID;
    }
    else
    {
        /* 写入消息 */
        memcpy(impl->buffer + (impl->tail * impl->msg_size), data, size);
        impl->tail = (impl->tail + 1) % impl->depth;
        impl->count++;
        pthread_cond_signal(&impl->not_empty);
    }

    pthread_mutex_unlock(&impl->mutex);
    osal_queue_release(impl);

    return result;
}

int32 OSAL_QueueGet(osal_id_t queue_id, void *data, uint32 size,
                  uint32 *size_copied, int32 timeout)
{
    queue_impl_t *impl = NULL;
    struct timespec ts;
    int ret;
    int32 result = OS_SUCCESS;

    if (data == NULL)
        return OS_INVALID_POINTER;

    /* 获取队列并增加引用计数 */
    pthread_mutex_lock(&g_queue_table_mutex);
    impl = osal_queue_acquire(queue_id);
    pthread_mutex_unlock(&g_queue_table_mutex);

    if (impl == NULL)
        return OS_ERR_INVALID_ID;

    pthread_mutex_lock(&impl->mutex);

    /* 检查队列是否仍然有效 */
    if (!impl->valid)
    {
        pthread_mutex_unlock(&impl->mutex);
        osal_queue_release(impl);
        return OS_ERR_INVALID_ID;
    }

    /* 处理超时 */
    if (timeout == OS_CHECK)
    {
        /* 非阻塞 */
        if (impl->count == 0)
        {
            result = OS_QUEUE_EMPTY;
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

        while (impl->count == 0 && impl->valid)
        {
            ret = pthread_cond_timedwait(&impl->not_empty, &impl->mutex, &ts);
            if (ret == ETIMEDOUT)
            {
                result = OS_QUEUE_TIMEOUT;
                break;
            }
        }

        if (!impl->valid)
            result = OS_ERR_INVALID_ID;
    }
    else
    {
        /* 永久等待 */
        while (impl->count == 0 && impl->valid)
        {
            pthread_cond_wait(&impl->not_empty, &impl->mutex);
        }

        if (!impl->valid)
            result = OS_ERR_INVALID_ID;
    }

    /* 读取消息 */
    if (result == OS_SUCCESS && impl->count > 0)
    {
        uint32 copy_size = (size < impl->msg_size) ? size : impl->msg_size;
        memcpy(data, impl->buffer + (impl->head * impl->msg_size), copy_size);
        impl->head = (impl->head + 1) % impl->depth;
        impl->count--;

        if (size_copied != NULL)
            *size_copied = copy_size;

        pthread_cond_signal(&impl->not_full);
    }

    pthread_mutex_unlock(&impl->mutex);
    osal_queue_release(impl);

    return result;
}

int32 OSAL_QueueGetIdByName(osal_id_t *queue_id, const char *queue_name)
{
    if (queue_id == NULL || queue_name == NULL)
        return OS_INVALID_POINTER;

    pthread_mutex_lock(&g_queue_table_mutex);

    for (uint32 i = 0; i < OS_MAX_QUEUES; i++)
    {
        if (g_osal_queue_table[i].is_used &&
            strcmp(g_osal_queue_table[i].name, queue_name) == 0)
        {
            *queue_id = g_osal_queue_table[i].id;
            pthread_mutex_unlock(&g_queue_table_mutex);
            return OS_SUCCESS;
        }
    }

    pthread_mutex_unlock(&g_queue_table_mutex);
    return OS_ERR_NAME_NOT_FOUND;
}
