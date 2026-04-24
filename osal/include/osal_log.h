/************************************************************************
 * OSAL日志API头文件
 ************************************************************************/

#ifndef OSAPI_LOG_H
#define OSAPI_LOG_H

#include "osal_types.h"

/*
 * 日志级别
 */
#define OS_LOG_LEVEL_DEBUG  0
#define OS_LOG_LEVEL_INFO   1
#define OS_LOG_LEVEL_WARN   2
#define OS_LOG_LEVEL_ERROR  3
#define OS_LOG_LEVEL_FATAL  4

/**
 * @brief 初始化日志系统
 *
 * @param[in] log_file_path 日志文件路径，NULL表示只输出到终端
 * @param[in] level 日志级别
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 失败
 */
int32 OSAL_LogInit(const char *log_file_path, int32 level);

/**
 * @brief 关闭日志系统
 */
void OSAL_LogShutdown(void);

/**
 * @brief 设置日志级别
 *
 * @param[in] level 日志级别
 */
void OSAL_LogSetLevel(int32 level);

/**
 * @brief 设置日志文件最大大小
 *
 * @param[in] size_bytes 最大文件大小（字节）
 */
void OSAL_LogSetMaxFileSize(uint32 size_bytes);

/**
 * @brief 设置最大日志文件数
 *
 * @param[in] max_files 最大备份文件数
 */
void OSAL_LogSetMaxFiles(uint32 max_files);

/**
 * @brief 通用日志函数
 *
 * @param[in] level 日志级别
 * @param[in] module 模块名称
 * @param[in] format 格式化字符串
 * @param[in] ... 可变参数
 */
void OSAL_Log(int32 level, const char *module, const char *format, ...);

/**
 * @brief DEBUG级别日志
 */
void OSAL_LogDebug(const char *module, const char *format, ...);

/**
 * @brief INFO级别日志
 */
void OSAL_LogInfo(const char *module, const char *format, ...);

/**
 * @brief WARN级别日志
 */
void OSAL_LogWarn(const char *module, const char *format, ...);

/**
 * @brief ERROR级别日志
 */
void OSAL_LogError(const char *module, const char *format, ...);

/**
 * @brief FATAL级别日志
 */
void OSAL_LogFatal(const char *module, const char *format, ...);

/**
 * @brief 简单打印（兼容旧接口）
 */
void OSAL_Printf(const char *format, ...);

/*
 * 便捷宏定义
 */
#define LOG_DEBUG(module, ...) OSAL_LogDebug(module, __VA_ARGS__)
#define LOG_INFO(module, ...)  OSAL_LogInfo(module, __VA_ARGS__)
#define LOG_WARN(module, ...)  OSAL_LogWarn(module, __VA_ARGS__)
#define LOG_ERROR(module, ...) OSAL_LogError(module, __VA_ARGS__)
#define LOG_FATAL(module, ...) OSAL_LogFatal(module, __VA_ARGS__)

#endif /* OSAPI_LOG_H */
