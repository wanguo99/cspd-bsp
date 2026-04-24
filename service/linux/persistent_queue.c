/************************************************************************
 * 持久化命令队列实现
 *
 * 使用简单的二进制格式存储命令，支持系统启动时恢复
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <unistd.h>
#include "persistent_queue.h"
#include "osapi_queue.h"
#include "osapi_log.h"

#define QUEUE_FILE_MAGIC    0x50515545  /* "PQUE" */
#define QUEUE_FILE_VERSION  1
#define MAX_QUEUE_FILE_SIZE (1024 * 1024)  /* 1MB max */

/*
 * 持久化队列内部结构
 */
typedef struct
{
    osal_id_t queue_id;            /* OSAL队列ID */
    char queue_file[256];          /* 队列文件路径 */
    uint32 max_commands;           /* 最大命令数 */
    uint32 current_count;          /* 当前命令数 */
    pthread_mutex_t lock;          /* 互斥锁 */
    uint32 magic;                  /* 魔数验证 */
} persistent_queue_t;

/*
 * 二进制文件格式：
 * [Header(4B)][Version(1B)][Count(4B)][Cmd1][Cmd2]...
 * 每个命令：[cmd_type(1B)][seq_num(2B)][param(4B)][priority(1B)][timeout(4B)][status(1B)][timestamp(8B)]
 */
typedef struct
{
    uint32 magic;
    uint8 version;
    uint32 count;
} __attribute__((packed)) queue_file_header_t;

typedef struct
{
    uint8 cmd_type;
    uint16 seq_num;
    uint32 param;
    uint8 priority;
    uint32 timeout_ms;
    uint8 status;
    uint64 timestamp;
} __attribute__((packed)) queue_file_cmd_t;

static int32 persistent_queue_load_from_file(persistent_queue_t *pq);
static int32 persistent_queue_save_to_file(persistent_queue_t *pq);

/**
 * @brief 初始化持久化命令队列
 */
int32 PersistentQueue_Init(const char *queue_file,
                           uint32 max_commands,
                           persistent_queue_handle_t *handle)
{
    persistent_queue_t *pq;
    int32 ret;
    char queue_name[64];

    if (!queue_file || !handle || max_commands == 0)
    {
        OS_Log_Error("PersistentQueue_Init", "invalid parameters");
        return OS_ERROR;
    }

    pq = (persistent_queue_t *)malloc(sizeof(persistent_queue_t));
    if (!pq)
    {
        OS_Log_Error("PersistentQueue_Init", "malloc failed");
        return OS_ERROR;
    }

    memset(pq, 0, sizeof(persistent_queue_t));
    pq->magic = QUEUE_FILE_MAGIC;
    pq->max_commands = max_commands;
    pq->current_count = 0;
    strncpy(pq->queue_file, queue_file, sizeof(pq->queue_file) - 1);

    /* 初始化互斥锁 */
    if (pthread_mutex_init(&pq->lock, NULL) != 0)
    {
        OS_Log_Error("PersistentQueue_Init", "pthread_mutex_init failed");
        free(pq);
        return OS_ERROR;
    }

    /* 创建OSAL队列 */
    snprintf(queue_name, sizeof(queue_name), "PQUE_%p", (void *)pq);
    ret = OS_QueueCreate(&pq->queue_id, queue_name, max_commands, sizeof(persistent_cmd_t), 0);
    if (ret != OS_SUCCESS)
    {
        OS_Log_Error("PersistentQueue_Init", "OS_QueueCreate failed");
        pthread_mutex_destroy(&pq->lock);
        free(pq);
        return OS_ERROR;
    }

    /* 从文件加载已保存的命令 */
    ret = persistent_queue_load_from_file(pq);
    if (ret != OS_SUCCESS)
    {
        OS_Log_Warn("PersistentQueue_Init", "failed to load from file, starting with empty queue");
    }

    *handle = (persistent_queue_handle_t)pq;
    OS_Log_Info("PersistentQueue_Init", "initialized with max_commands=%u, file=%s",
                max_commands, queue_file);

    return OS_SUCCESS;
}

/**
 * @brief 关闭持久化命令队列
 */
int32 PersistentQueue_Shutdown(persistent_queue_handle_t handle)
{
    persistent_queue_t *pq = (persistent_queue_t *)handle;

    if (!pq || pq->magic != QUEUE_FILE_MAGIC)
    {
        OS_Log_Error("PersistentQueue_Shutdown", "invalid handle");
        return OS_ERROR;
    }

    pthread_mutex_lock(&pq->lock);

    /* 保存队列到文件 */
    persistent_queue_save_to_file(pq);

    /* 销毁OSAL队列 */
    OS_QueueDelete(pq->queue_id);

    pthread_mutex_unlock(&pq->lock);
    pthread_mutex_destroy(&pq->lock);

    free(pq);
    OS_Log_Info("PersistentQueue_Shutdown", "completed");

    return OS_SUCCESS;
}

/**
 * @brief 添加命令到队列
 */
int32 PersistentQueue_Enqueue(persistent_queue_handle_t handle,
                              const persistent_cmd_t *cmd)
{
    persistent_queue_t *pq = (persistent_queue_t *)handle;
    int32 ret;

    if (!pq || pq->magic != QUEUE_FILE_MAGIC || !cmd)
    {
        OS_Log_Error("PersistentQueue_Enqueue", "invalid parameters");
        return OS_ERROR;
    }

    pthread_mutex_lock(&pq->lock);

    ret = OS_QueuePut(pq->queue_id, (const void *)cmd, sizeof(persistent_cmd_t), 0);
    if (ret != OS_SUCCESS)
    {
        pthread_mutex_unlock(&pq->lock);
        OS_Log_Warn("PersistentQueue_Enqueue", "queue put failed, ret=%d", ret);
        return ret;
    }

    pq->current_count++;

    /* 立即保存到文件 */
    persistent_queue_save_to_file(pq);

    pthread_mutex_unlock(&pq->lock);

    OS_Log_Debug("PersistentQueue_Enqueue", "cmd_type=%u, seq_num=%u, priority=%u",
                 cmd->cmd_type, cmd->seq_num, cmd->priority);

    return OS_SUCCESS;
}

/**
 * @brief 从队列取出命令
 */
int32 PersistentQueue_Dequeue(persistent_queue_handle_t handle,
                              persistent_cmd_t *cmd,
                              int32 timeout)
{
    persistent_queue_t *pq = (persistent_queue_t *)handle;
    int32 ret;
    uint32 size_copied;

    if (!pq || pq->magic != QUEUE_FILE_MAGIC || !cmd)
    {
        OS_Log_Error("PersistentQueue_Dequeue", "invalid parameters");
        return OS_ERROR;
    }

    pthread_mutex_lock(&pq->lock);

    ret = OS_QueueGet(pq->queue_id, (void *)cmd, sizeof(persistent_cmd_t), &size_copied, timeout);
    if (ret == OS_SUCCESS)
    {
        pq->current_count--;
    }

    pthread_mutex_unlock(&pq->lock);

    if (ret == OS_SUCCESS)
    {
        OS_Log_Debug("PersistentQueue_Dequeue", "cmd_type=%u, seq_num=%u",
                     cmd->cmd_type, cmd->seq_num);
    }

    return ret;
}

/**
 * @brief 标记命令为已完成
 */
int32 PersistentQueue_MarkCompleted(persistent_queue_handle_t handle,
                                    uint16 seq_num)
{
    persistent_queue_t *pq = (persistent_queue_t *)handle;

    if (!pq || pq->magic != QUEUE_FILE_MAGIC)
    {
        OS_Log_Error("PersistentQueue_MarkCompleted", "invalid handle");
        return OS_ERROR;
    }

    pthread_mutex_lock(&pq->lock);

    /* 保存队列状态到文件 */
    persistent_queue_save_to_file(pq);

    pthread_mutex_unlock(&pq->lock);

    OS_Log_Debug("PersistentQueue_MarkCompleted", "seq_num=%u", seq_num);

    return OS_SUCCESS;
}

/**
 * @brief 标记命令为失败
 */
int32 PersistentQueue_MarkFailed(persistent_queue_handle_t handle,
                                 uint16 seq_num)
{
    persistent_queue_t *pq = (persistent_queue_t *)handle;

    if (!pq || pq->magic != QUEUE_FILE_MAGIC)
    {
        OS_Log_Error("PersistentQueue_MarkFailed", "invalid handle");
        return OS_ERROR;
    }

    pthread_mutex_lock(&pq->lock);

    /* 保存队列状态到文件 */
    persistent_queue_save_to_file(pq);

    pthread_mutex_unlock(&pq->lock);

    OS_Log_Warn("PersistentQueue_MarkFailed", "seq_num=%u", seq_num);

    return OS_SUCCESS;
}

/**
 * @brief 获取队列中的命令数
 */
int32 PersistentQueue_GetCount(persistent_queue_handle_t handle,
                               uint32 *count)
{
    persistent_queue_t *pq = (persistent_queue_t *)handle;

    if (!pq || pq->magic != QUEUE_FILE_MAGIC || !count)
    {
        OS_Log_Error("PersistentQueue_GetCount", "invalid parameters");
        return OS_ERROR;
    }

    pthread_mutex_lock(&pq->lock);

    *count = pq->current_count;

    pthread_mutex_unlock(&pq->lock);

    return OS_SUCCESS;
}

/**
 * @brief 清空队列
 */
int32 PersistentQueue_Clear(persistent_queue_handle_t handle)
{
    persistent_queue_t *pq = (persistent_queue_t *)handle;
    char queue_name[64];
    int32 ret;

    if (!pq || pq->magic != QUEUE_FILE_MAGIC)
    {
        OS_Log_Error("PersistentQueue_Clear", "invalid handle");
        return OS_ERROR;
    }

    pthread_mutex_lock(&pq->lock);

    OS_QueueDelete(pq->queue_id);

    snprintf(queue_name, sizeof(queue_name), "PQUE_%p", (void *)pq);
    ret = OS_QueueCreate(&pq->queue_id, queue_name, pq->max_commands, sizeof(persistent_cmd_t), 0);
    if (ret != OS_SUCCESS)
    {
        OS_Log_Error("PersistentQueue_Clear", "OS_QueueCreate failed");
        pthread_mutex_unlock(&pq->lock);
        return OS_ERROR;
    }

    pq->current_count = 0;

    /* 删除队列文件 */
    unlink(pq->queue_file);

    pthread_mutex_unlock(&pq->lock);

    OS_Log_Info("PersistentQueue_Clear", "queue cleared");

    return OS_SUCCESS;
}

/**
 * @brief 将队列持久化到磁盘
 */
int32 PersistentQueue_Flush(persistent_queue_handle_t handle)
{
    persistent_queue_t *pq = (persistent_queue_t *)handle;
    int32 ret;

    if (!pq || pq->magic != QUEUE_FILE_MAGIC)
    {
        OS_Log_Error("PersistentQueue_Flush", "invalid handle");
        return OS_ERROR;
    }

    pthread_mutex_lock(&pq->lock);

    ret = persistent_queue_save_to_file(pq);

    pthread_mutex_unlock(&pq->lock);

    return ret;
}

/**
 * @brief 从文件加载队列
 */
static int32 persistent_queue_load_from_file(persistent_queue_t *pq)
{
    FILE *fp;
    queue_file_header_t header;
    queue_file_cmd_t file_cmd;
    persistent_cmd_t cmd;
    uint32 i;
    struct stat st;

    if (!pq)
        return OS_ERROR;

    /* 检查文件是否存在 */
    if (stat(pq->queue_file, &st) != 0)
    {
        OS_Log_Debug("persistent_queue_load_from_file", "file not found, creating new");
        return OS_SUCCESS;
    }

    /* 检查文件大小 */
    if (st.st_size > MAX_QUEUE_FILE_SIZE)
    {
        OS_Log_Error("persistent_queue_load_from_file", "file too large");
        return OS_ERROR;
    }

    fp = fopen(pq->queue_file, "rb");
    if (!fp)
    {
        OS_Log_Error("persistent_queue_load_from_file", "fopen failed");
        return OS_ERROR;
    }

    /* 读取文件头 */
    if (fread(&header, sizeof(header), 1, fp) != 1)
    {
        OS_Log_Error("persistent_queue_load_from_file", "failed to read header");
        fclose(fp);
        return OS_ERROR;
    }

    /* 验证魔数和版本 */
    if (header.magic != QUEUE_FILE_MAGIC || header.version != QUEUE_FILE_VERSION)
    {
        OS_Log_Error("persistent_queue_load_from_file", "invalid header");
        fclose(fp);
        return OS_ERROR;
    }

    /* 读取命令 */
    for (i = 0; i < header.count && i < pq->max_commands; i++)
    {
        if (fread(&file_cmd, sizeof(file_cmd), 1, fp) != 1)
        {
            OS_Log_Error("persistent_queue_load_from_file", "failed to read command %u", i);
            break;
        }

        /* 转换为内部格式 */
        cmd.cmd_type = (can_cmd_type_t)file_cmd.cmd_type;
        cmd.seq_num = file_cmd.seq_num;
        cmd.param = file_cmd.param;
        cmd.priority = (cmd_priority_t)file_cmd.priority;
        cmd.timeout_ms = file_cmd.timeout_ms;
        cmd.status = (cmd_status_t)file_cmd.status;
        cmd.timestamp = file_cmd.timestamp;

        /* 只加载未完成的命令 */
        if (cmd.status == CMD_STATUS_PENDING || cmd.status == CMD_STATUS_EXECUTING)
        {
            if (OS_QueuePut(pq->queue_id, (const void *)&cmd, sizeof(persistent_cmd_t), 0) != OS_SUCCESS)
            {
                OS_Log_Warn("persistent_queue_load_from_file", "queue full, skipping command %u", i);
                break;
            }
            pq->current_count++;
        }
    }

    fclose(fp);

    OS_Log_Info("persistent_queue_load_from_file", "loaded %u commands", i);

    return OS_SUCCESS;
}

/**
 * @brief 保存队列到文件
 */
static int32 persistent_queue_save_to_file(persistent_queue_t *pq)
{
    FILE *fp;
    queue_file_header_t header;
    queue_file_cmd_t file_cmd;
    persistent_cmd_t cmd;
    uint32 count;
    char temp_file[512];
    uint32 size_copied;

    if (!pq)
        return OS_ERROR;

    count = pq->current_count;

    /* 使用临时文件，避免部分写入 */
    snprintf(temp_file, sizeof(temp_file), "%s.tmp", pq->queue_file);

    fp = fopen(temp_file, "wb");
    if (!fp)
    {
        OS_Log_Error("persistent_queue_save_to_file", "fopen failed");
        return OS_ERROR;
    }

    /* 写入文件头 */
    header.magic = QUEUE_FILE_MAGIC;
    header.version = QUEUE_FILE_VERSION;
    header.count = count;

    if (fwrite(&header, sizeof(header), 1, fp) != 1)
    {
        OS_Log_Error("persistent_queue_save_to_file", "failed to write header");
        fclose(fp);
        unlink(temp_file);
        return OS_ERROR;
    }

    /* 写入所有命令 */
    while (OS_QueueGet(pq->queue_id, (void *)&cmd, sizeof(persistent_cmd_t), &size_copied, OS_CHECK) == OS_SUCCESS)
    {
        /* 转换为文件格式 */
        file_cmd.cmd_type = (uint8)cmd.cmd_type;
        file_cmd.seq_num = cmd.seq_num;
        file_cmd.param = cmd.param;
        file_cmd.priority = (uint8)cmd.priority;
        file_cmd.timeout_ms = cmd.timeout_ms;
        file_cmd.status = (uint8)cmd.status;
        file_cmd.timestamp = cmd.timestamp;

        if (fwrite(&file_cmd, sizeof(file_cmd), 1, fp) != 1)
        {
            OS_Log_Error("persistent_queue_save_to_file", "failed to write command");
            fclose(fp);
            unlink(temp_file);
            return OS_ERROR;
        }

        /* 重新入队 */
        OS_QueuePut(pq->queue_id, (const void *)&cmd, sizeof(persistent_cmd_t), 0);
    }

    fclose(fp);

    /* 原子性替换 */
    if (rename(temp_file, pq->queue_file) != 0)
    {
        OS_Log_Error("persistent_queue_save_to_file", "rename failed");
        unlink(temp_file);
        return OS_ERROR;
    }

    OS_Log_Debug("persistent_queue_save_to_file", "saved %u commands", count);

    return OS_SUCCESS;
}
