/************************************************************************
 * OSAL POSIX实现 - 版本信息
 *
 * 注意：OSAL作为用户态库，不需要显式初始化
 * - 静态变量在程序启动时自动初始化为0
 * - 参考pthread、libc等标准库的做法
 ************************************************************************/

#include "osal.h"
#include <unistd.h>

#define OS_VERSION_STRING "PMC-BSP OSAL v1.0.0"

/**
 * @brief 获取OSAL版本字符串
 */
const str_t *OS_GetVersionString(void)
{
    return OS_VERSION_STRING;
}

/**
 * @brief 空闲循环（保留用于兼容性）
 *
 * 此函数不会返回，用于某些RTOS场景
 * 在Linux用户态程序中通常不需要使用
 */
int32_t OS_IdleLoop(void)
{
    while (1)
    {
        sleep(1);
    }

    /* 不应该到达这里 */
    return OS_ERROR;
}
