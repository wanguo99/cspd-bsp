/************************************************************************
 * OSAL POSIX实现 - 消息队列（优化版）
 *
 * 修复内容：
 * 1. 修复线程安全问题：使用引用计数保护对象
 * 2. 添加对象生命周期管理
 * 3. 改进错误处理
 * 4. 修复整数溢出检查（使用 SIZE_MAX 而非 UINT32_MAX）
 ************************************************************************/

#include "osal.h"
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <stdatomic.h>
#include <stdint.h>  /* SIZE_MAX */

/*
 * 环形缓冲区消息队列实现
 */
typedef struct
{
    uint8_t  *buffer;
    uint32_t  head;
    uint32_t  tail;
    uint32_t  count;
    uint32_t  depth;
    uint32_t  msg_size;
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

static osal_queue_record_t g_osal_queue_table[OS_MAX_QUEUES] = {0};  /* 静态变量自动初始化为0 */
static pthread_mutex_t   g_queue_table_mutex = PTHREAD_MUTEX_INITIALIZER;
static uint32_t            g_next_queue_id = 1;

/* 移除 osal_queue_table_init() - 静态变量已自动初始化 */

/*
 * 增加引用计数（必须在持有table锁时调用）
 */
static queue_impl_t* osal_queue_acquire(osal_id_t queue_id)
{
    for (uint32_t i = 0; i < OS_MAX_QUEUES; i++)
    {
        if (g_osal_queue_table[i].is_used &&
            g_osal_queue_table[i].id == queue_id &&
            NULL != g_osal_queue_table[i].impl)
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
    if (NULL == impl) return;

    int32_t old_count = atomic_fetch_sub(&impl->ref_count, 1);

    /* 如果引用计数降为0且对象已标记为无效，则释放资源 */
    if (1 == old_count && !impl->valid)
    {
        pthread_mutex_destroy(&impl->mutex);
        pthread_cond_destroy(&impl->not_empty);
        pthread_cond_destroy(&impl->not_full);
        OSAL_Free(impl->buffer);
        OSAL_Free(impl);
    }
}

static int32_t osal_queue_find_free_slot(uint32_t *slot)
{
    pthread_mutex_lock(&g_queue_table_mutex);

    for (uint32_t i = 0; i < OS_MAX_QUEUES; i++)
    {
        if (!g_osal_queue_table[i].is_used)
        {
            *slot = i;
            pthread_mutex_unlock(&g_queue_table_mutex);
            return OSAL_SUCCESS;
        }
    }

    pthread_mutex_unlock(&g_queue_table_mutex);
    return OSAL_ERR_NO_FREE_IDS;
}

int32_t OSAL_QueueCreate(osal_id_t *queue_id,
                     const char *queue_name,
                     uint32_t queue_depth,
                     uint32_t data_size,
                     uint32_t flags __attribute__((unused)))
{
    uint32_t slot;
    int32_t ret;
    queue_impl_t *impl;

    /* 参数检查 */
    if (NULL == queue_id)
        return OSAL_ERR_INVALID_POINTER;

    if (NULL == queue_name || strlen(queue_name) >= OS_MAX_API_NAME)
        return OSAL_ERR_NAME_TOO_LONG;

    if (0 == queue_depth || 0 == data_size)
        return OSAL_ERR_QUEUE_INVALID_SIZE;

    /* 限制队列深度和消息大小，防止过度内存分配 */
    if (queue_depth > OSAL_QUEUE_DEPTH_LIMIT || data_size > OSAL_QUEUE_DATA_SIZE_LIMIT)
        return OSAL_ERR_QUEUE_INVALID_SIZE;

    /* 检查乘法溢出（使用 SIZE_MAX 确保在 64 位系统上也正确）*/
    if (queue_depth > SIZE_MAX / data_size)
    {
        return OSAL_ERR_QUEUE_INVALID_SIZE;  /* 溢出风险 */
    }

    ret = osal_queue_find_free_slot(&slot);
    if (OSAL_SUCCESS != ret)
        return ret;

    /* 检查名称冲突 */
    pthread_mutex_lock(&g_queue_table_mutex);
    for (uint32_t i = 0; i < OS_MAX_QUEUES; i++)
    {
        if (g_osal_queue_table[i].is_used &&
            0 == strcmp(g_osal_queue_table[i].name, queue_name))
        {
            pthread_mutex_unlock(&g_queue_table_mutex);
            return OSAL_ERR_NAME_TAKEN;
        }
    }

    /* 分配队列实现 */
    impl = OSAL_Malloc(sizeof(queue_impl_t));
    if (NULL == impl)
    {
        pthread_mutex_unlock(&g_queue_table_mutex);
        return OSAL_ERR_NO_MEMORY;
    }

    OSAL_Memset(impl, 0, sizeof(queue_impl_t));

    /* 尽早初始化引用计数，避免竞态条件 */
    atomic_init(&impl->ref_count, 1);
    impl->valid = true;

    /* 安全的内存分配（已检查溢出） */
    osal_size_t buffer_size = queue_depth;
    buffer_size *= data_size;
    impl->buffer = OSAL_Malloc(buffer_size);
    if (NULL == impl->buffer)
    {
        OSAL_Free(impl);
        pthread_mutex_unlock(&g_queue_table_mutex);
        return OSAL_ERR_NO_MEMORY;
    }

    impl->head = 0;
    impl->tail = 0;
    impl->count = 0;
    impl->depth = queue_depth;
    impl->msg_size = data_size;

    /* 初始化同步原语，检查返回值 */
    if (0 != pthread_mutex_init(&impl->mutex, NULL))
    {
        OSAL_Free(impl->buffer);
        OSAL_Free(impl);
        pthread_mutex_unlock(&g_queue_table_mutex);
        return OSAL_ERR_GENERIC;
    }

    if (0 != pthread_cond_init(&impl->not_empty, NULL))
    {
        pthread_mutex_destroy(&impl->mutex);
        OSAL_Free(impl->buffer);
        OSAL_Free(impl);
        pthread_mutex_unlock(&g_queue_table_mutex);
        return OSAL_ERR_GENERIC;
    }

    if (0 != pthread_cond_init(&impl->not_full, NULL))
    {
        pthread_cond_destroy(&impl->not_empty);
        pthread_mutex_destroy(&impl->mutex);
        OSAL_Free(impl->buffer);
        OSAL_Free(impl);
        pthread_mutex_unlock(&g_queue_table_mutex);
        return OSAL_ERR_GENERIC;
    }

    /* 填充队列表 */
    g_osal_queue_table[slot].is_used = true;
    g_osal_queue_table[slot].id = g_next_queue_id++;
    strncpy(g_osal_queue_table[slot].name, queue_name, OS_MAX_API_NAME - 1);
    g_osal_queue_table[slot].name[OS_MAX_API_NAME - 1] = '\0';
    g_osal_queue_table[slot].impl = impl;

    *queue_id = g_osal_queue_table[slot].id;

    /* 注册资源到泄漏检测系统 */
    OSAL_ResourceRegister(*queue_id, OSAL_RESOURCE_TYPE_QUEUE, queue_name, __FILE__, __LINE__);

    pthread_mutex_unlock(&g_queue_table_mutex);

    return OSAL_SUCCESS;
}

int32_t OSAL_QueueDelete(osal_id_t queue_id)
{
    queue_impl_t *impl = NULL;

    pthread_mutex_lock(&g_queue_table_mutex);

    for (uint32_t i = 0; i < OS_MAX_QUEUES; i++)
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

    if (NULL == impl)
        return OSAL_ERR_INVALID_ID;

    /* 标记为无效并唤醒所有等待线程 */
    pthread_mutex_lock(&impl->mutex);
    impl->valid = false;
    pthread_cond_broadcast(&impl->not_empty);
    pthread_cond_broadcast(&impl->not_full);
    pthread_mutex_unlock(&impl->mutex);

    /* 释放初始引用 */
    osal_queue_release(impl);

    /* 注销资源 */
    OSAL_ResourceUnregister(queue_id, OSAL_RESOURCE_TYPE_QUEUE);

    return OSAL_SUCCESS;
}

int32_t OSAL_QueuePut(osal_id_t queue_id, const void *data, uint32_t size, uint32_t flags __attribute__((unused)))
{
    queue_impl_t *impl = NULL;
    int32_t result = OSAL_SUCCESS;

    if (NULL == data)
        return OSAL_ERR_INVALID_POINTER;

    /* 获取队列并增加引用计数 */
    pthread_mutex_lock(&g_queue_table_mutex);
    impl = osal_queue_acquire(queue_id);
    pthread_mutex_unlock(&g_queue_table_mutex);

    if (NULL == impl)
        return OSAL_ERR_INVALID_ID;

    if (size > impl->msg_size)
    {
        osal_queue_release(impl);
        return OSAL_ERR_QUEUE_INVALID_SIZE;
    }

    pthread_mutex_lock(&impl->mutex);

    /* 检查队列是否仍然有效 */
    if (!impl->valid)
    {
        pthread_mutex_unlock(&impl->mutex);
        osal_queue_release(impl);
        return OSAL_ERR_INVALID_ID;
    }

    /* 等待队列非满 */
    while (impl->count >= impl->depth && impl->valid)
    {
        pthread_cond_wait(&impl->not_full, &impl->mutex);
    }

    /* 再次检查有效性 */
    if (!impl->valid)
    {
        result = OSAL_ERR_INVALID_ID;
    }
    else if (size > impl->msg_size)
    {
        /* 防御性检查：消息大小不能超过队列配置的最大消息大小 */
        result = OSAL_ERR_INVALID_SIZE;
    }
    else
    {
        /* 写入消息 */
        OSAL_Memcpy(impl->buffer + (impl->tail * impl->msg_size), data, size);
        impl->tail = (impl->tail + 1) % impl->depth;
        impl->count++;
        pthread_cond_signal(&impl->not_empty);
    }

    pthread_mutex_unlock(&impl->mutex);
    osal_queue_release(impl);

    return result;
}

int32_t OSAL_QueueGet(osal_id_t queue_id, void *data, uint32_t size,
                  uint32_t *size_copied, int32_t timeout)
{
    queue_impl_t *impl = NULL;
    struct timespec ts;
    int32_t ret;
    int32_t result = OSAL_SUCCESS;

    if (NULL == data)
        return OSAL_ERR_INVALID_POINTER;

    /* 获取队列并增加引用计数 */
    pthread_mutex_lock(&g_queue_table_mutex);
    impl = osal_queue_acquire(queue_id);
    pthread_mutex_unlock(&g_queue_table_mutex);

    if (NULL == impl)
        return OSAL_ERR_INVALID_ID;

    pthread_mutex_lock(&impl->mutex);

    /* 检查队列是否仍然有效 */
    if (!impl->valid)
    {
        pthread_mutex_unlock(&impl->mutex);
        osal_queue_release(impl);
        return OSAL_ERR_INVALID_ID;
    }

    /* 处理超时 */
    if (OS_CHECK == timeout)
    {
        /* 非阻塞 */
        if (0 == impl->count)
        {
            result = OSAL_ERR_QUEUE_EMPTY;
        }
    }
    else if (timeout > 0)
    {
        /* 超时等待 */
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += timeout / OSAL_MSEC_PER_SEC;
        ts.tv_nsec += (timeout % OSAL_MSEC_PER_SEC) * OSAL_NSEC_PER_MSEC;
        if (ts.tv_nsec >= OSAL_NSEC_PER_SEC)
        {
            ts.tv_sec++;
            ts.tv_nsec -= OSAL_NSEC_PER_SEC;
        }

        while (0 == impl->count && impl->valid)
        {
            ret = pthread_cond_timedwait(&impl->not_empty, &impl->mutex, &ts);
            if (ret == ETIMEDOUT)
            {
                result = OSAL_ERR_QUEUE_TIMEOUT;
                break;
            }
        }

        if (!impl->valid)
            result = OSAL_ERR_INVALID_ID;
    }
    else
    {
        /* 永久等待 */
        while (0 == impl->count && impl->valid)
        {
            pthread_cond_wait(&impl->not_empty, &impl->mutex);
        }

        if (!impl->valid)
            result = OSAL_ERR_INVALID_ID;
    }

    /* 读取消息 */
    if (OSAL_SUCCESS == result && impl->count > 0)
    {
        uint32_t copy_size = (size < impl->msg_size) ? size : impl->msg_size;
        OSAL_Memcpy(data, impl->buffer + (impl->head * impl->msg_size), copy_size);
        impl->head = (impl->head + 1) % impl->depth;
        impl->count--;

        if (NULL != size_copied)
            *size_copied = copy_size;

        pthread_cond_signal(&impl->not_full);
    }

    pthread_mutex_unlock(&impl->mutex);
    osal_queue_release(impl);

    return result;
}

int32_t OSAL_QueueGetIdByName(osal_id_t *queue_id, const char *queue_name)
{
    if (NULL == queue_id || NULL == queue_name)
        return OSAL_ERR_INVALID_POINTER;

    pthread_mutex_lock(&g_queue_table_mutex);

    for (uint32_t i = 0; i < OS_MAX_QUEUES; i++)
    {
        if (g_osal_queue_table[i].is_used &&
            0 == strcmp(g_osal_queue_table[i].name, queue_name))
        {
            *queue_id = g_osal_queue_table[i].id;
            pthread_mutex_unlock(&g_queue_table_mutex);
            return OSAL_SUCCESS;
        }
    }

    pthread_mutex_unlock(&g_queue_table_mutex);
    return OSAL_ERR_NAME_NOT_FOUND;
}
