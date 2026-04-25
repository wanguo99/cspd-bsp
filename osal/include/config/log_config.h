/************************************************************************
 * 日志配置
 ************************************************************************/

#ifndef LOG_CONFIG_H
#define LOG_CONFIG_H

/* 日志级别 */
typedef enum
{
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL
} log_level_t;

/* 当前日志级别 */
#define LOG_LEVEL               LOG_LEVEL_INFO

/* 日志文件路径 */
#define LOG_FILE_PATH           "/var/log/pmc-bsp.log"

/* 日志文件最大大小(MB) */
#define LOG_FILE_MAX_SIZE_MB    10

/* 日志文件备份数量 */
#define LOG_FILE_BACKUP_COUNT   5

#endif /* LOG_CONFIG_H */
