/************************************************************************
 * OSAL日志API头文件
 ************************************************************************/

#ifndef OSAPI_LOG_H
#define OSAPI_LOG_H

#include "common_types.h"

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
int32 OS_Log_Init(const char *log_file_path, int32 level);

/**
 * @brief 关闭日志系统
 */
void OS_Log_Shutdown(void);

/**
 * @brief 设置日志级别
 *
 * @param[in] level 日志级别
 */
void OS_Log_SetLevel(int32 level);

/**
 * @brief 通用日志函数
 *
 * @param[in] level 日志级别
 * @param[in] module 模块名称
 * @param[in] format 格式化字符串
 * @param[in] ... 可变参数
 */
void OS_Log(int32 level, const char *module, const char *format, ...);

/**
 * @brief DEBUG级别日志
 */
void OS_Log_Debug(const char *module, const char *format, ...);

/**
 * @brief INFO级别日志
 */
void OS_Log_Info(const char *module, const char *format, ...);

/**
 * @brief WARN级别日志
 */
void OS_Log_Warn(const char *module, const char *format, ...);

/**
 * @brief ERROR级别日志
 */
void OS_Log_Error(const char *module, const char *format, ...);

/**
 * @brief FATAL级别日志
 */
void OS_Log_Fatal(const char *module, const char *format, ...);

/**
 * @brief 简单打印（兼容旧接口）
 */
void OS_printf(const char *format, ...);

/*
 * 便捷宏定义
 */
#define LOG_DEBUG(module, ...) OS_Log_Debug(module, __VA_ARGS__)
#define LOG_INFO(module, ...)  OS_Log_Info(module, __VA_ARGS__)
#define LOG_WARN(module, ...)  OS_Log_Warn(module, __VA_ARGS__)
#define LOG_ERROR(module, ...) OS_Log_Error(module, __VA_ARGS__)
#define LOG_FATAL(module, ...) OS_Log_Fatal(module, __VA_ARGS__)

#endif /* OSAPI_LOG_H */
