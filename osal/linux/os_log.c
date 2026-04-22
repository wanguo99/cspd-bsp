/************************************************************************
 * OSAL Linux实现 - 日志系统
 *
 * 提供分级日志功能
 ************************************************************************/

#include "osal.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>

/*
 * 日志级别
 */
typedef enum
{
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL
} log_level_t;

/*
 * 日志配置
 */
static log_level_t g_log_level = LOG_LEVEL_INFO;
static FILE *g_log_file = NULL;
static pthread_mutex_t g_log_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * 日志级别名称
 */
static const char *log_level_names[] = {
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "FATAL"
};

/*
 * 日志级别颜色（终端）
 */
static const char *log_level_colors[] = {
    "\033[36m",  // DEBUG - 青色
    "\033[32m",  // INFO  - 绿色
    "\033[33m",  // WARN  - 黄色
    "\033[31m",  // ERROR - 红色
    "\033[35m"   // FATAL - 紫色
};

static const char *color_reset = "\033[0m";

/**
 * @brief 初始化日志系统
 *
 * @param[in] log_file_path 日志文件路径，NULL表示只输出到终端
 * @param[in] level 日志级别
 *
 * @return OS_SUCCESS 成功
 */
int32 OS_Log_Init(const char *log_file_path, int32 level)
{
    if (level >= LOG_LEVEL_DEBUG && level <= LOG_LEVEL_FATAL)
    {
        g_log_level = level;
    }

    if (log_file_path != NULL)
    {
        g_log_file = fopen(log_file_path, "a");
        if (g_log_file == NULL)
        {
            fprintf(stderr, "无法打开日志文件: %s\n", log_file_path);
            return OS_ERROR;
        }
    }

    return OS_SUCCESS;
}

/**
 * @brief 关闭日志系统
 */
void OS_Log_Shutdown(void)
{
    if (g_log_file != NULL)
    {
        fclose(g_log_file);
        g_log_file = NULL;
    }
}

/**
 * @brief 设置日志级别
 */
void OS_Log_SetLevel(int32 level)
{
    if (level >= LOG_LEVEL_DEBUG && level <= LOG_LEVEL_FATAL)
    {
        g_log_level = level;
    }
}

/**
 * @brief 获取当前时间字符串
 */
static void get_timestamp(char *buffer, size_t size)
{
    struct timeval tv;
    struct tm *tm_info;

    gettimeofday(&tv, NULL);
    tm_info = localtime(&tv.tv_sec);

    snprintf(buffer, size, "%04d-%02d-%02d %02d:%02d:%02d.%03ld",
             tm_info->tm_year + 1900,
             tm_info->tm_mon + 1,
             tm_info->tm_mday,
             tm_info->tm_hour,
             tm_info->tm_min,
             tm_info->tm_sec,
             (long)(tv.tv_usec / 1000));
}

/**
 * @brief 内部日志函数
 */
static void log_internal(log_level_t level, const char *module,
                         const char *format, va_list args)
{
    char timestamp[64];
    char message[1024];

    /* 检查日志级别 */
    if (level < g_log_level)
        return;

    /* 获取时间戳 */
    get_timestamp(timestamp, sizeof(timestamp));

    /* 格式化消息 */
    vsnprintf(message, sizeof(message), format, args);

    /* 加锁 */
    pthread_mutex_lock(&g_log_mutex);

    /* 输出到终端（带颜色） */
    if (module != NULL)
    {
        printf("%s[%s] [%s] [%s]%s %s",
               log_level_colors[level],
               timestamp,
               log_level_names[level],
               module,
               color_reset,
               message);
    }
    else
    {
        printf("%s[%s] [%s]%s %s",
               log_level_colors[level],
               timestamp,
               log_level_names[level],
               color_reset,
               message);
    }

    /* 输出到文件（无颜色） */
    if (g_log_file != NULL)
    {
        if (module != NULL)
        {
            fprintf(g_log_file, "[%s] [%s] [%s] %s",
                   timestamp,
                   log_level_names[level],
                   module,
                   message);
        }
        else
        {
            fprintf(g_log_file, "[%s] [%s] %s",
                   timestamp,
                   log_level_names[level],
                   message);
        }
        fflush(g_log_file);
    }

    /* 解锁 */
    pthread_mutex_unlock(&g_log_mutex);
}

/**
 * @brief 通用日志函数
 */
void OS_Log(int32 level, const char *module, const char *format, ...)
{
    va_list args;

    if (level < LOG_LEVEL_DEBUG || level > LOG_LEVEL_FATAL)
        return;

    va_start(args, format);
    log_internal((log_level_t)level, module, format, args);
    va_end(args);
}

/**
 * @brief DEBUG级别日志
 */
void OS_Log_Debug(const char *module, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_internal(LOG_LEVEL_DEBUG, module, format, args);
    va_end(args);
}

/**
 * @brief INFO级别日志
 */
void OS_Log_Info(const char *module, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_internal(LOG_LEVEL_INFO, module, format, args);
    va_end(args);
}

/**
 * @brief WARN级别日志
 */
void OS_Log_Warn(const char *module, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_internal(LOG_LEVEL_WARN, module, format, args);
    va_end(args);
}

/**
 * @brief ERROR级别日志
 */
void OS_Log_Error(const char *module, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_internal(LOG_LEVEL_ERROR, module, format, args);
    va_end(args);
}

/**
 * @brief FATAL级别日志
 */
void OS_Log_Fatal(const char *module, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_internal(LOG_LEVEL_FATAL, module, format, args);
    va_end(args);
}

/**
 * @brief 简单打印（兼容旧接口）
 */
void OS_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    fflush(stdout);
}
