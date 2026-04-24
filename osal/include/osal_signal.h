/************************************************************************
 * 信号处理API
 ************************************************************************/

#ifndef OSAPI_SIGNAL_H
#define OSAPI_SIGNAL_H

#include "osal_types.h"

/*
 * 信号类型
 */
#define OS_SIGNAL_INT       2   /* SIGINT - 中断信号 (Ctrl+C) */
#define OS_SIGNAL_TERM      15  /* SIGTERM - 终止信号 */
#define OS_SIGNAL_HUP       1   /* SIGHUP - 挂起信号 */
#define OS_SIGNAL_QUIT      3   /* SIGQUIT - 退出信号 */
#define OS_SIGNAL_USR1      10  /* SIGUSR1 - 用户自定义信号1 */
#define OS_SIGNAL_USR2      12  /* SIGUSR2 - 用户自定义信号2 */

/*
 * 信号处理函数类型
 */
typedef void (*os_signal_handler_t)(int32 signum);

/**
 * @brief 注册信号处理函数
 *
 * @param[in] signum  信号编号 (OS_SIGNAL_*)
 * @param[in] handler 信号处理函数，NULL表示使用默认处理
 *
 * @return OS_SUCCESS 成功
 * @return OS_INVALID_POINTER handler为NULL
 * @return OS_ERROR 注册失败
 */
int32 OSAL_SignalRegister(int32 signum, os_signal_handler_t handler);

/**
 * @brief 忽略信号
 *
 * @param[in] signum 信号编号 (OS_SIGNAL_*)
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 设置失败
 */
int32 OSAL_SignalIgnore(int32 signum);

/**
 * @brief 恢复信号的默认处理
 *
 * @param[in] signum 信号编号 (OS_SIGNAL_*)
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 恢复失败
 */
int32 OSAL_SignalDefault(int32 signum);

/**
 * @brief 阻塞信号
 *
 * @param[in] signum 信号编号 (OS_SIGNAL_*)
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 阻塞失败
 */
int32 OSAL_SignalBlock(int32 signum);

/**
 * @brief 解除信号阻塞
 *
 * @param[in] signum 信号编号 (OS_SIGNAL_*)
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 解除失败
 */
int32 OSAL_SignalUnblock(int32 signum);

#endif /* OSAPI_SIGNAL_H */
