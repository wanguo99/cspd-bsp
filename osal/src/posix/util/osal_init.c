/************************************************************************
 * OSAL POSIX实现 - 初始化和版本
 ************************************************************************/

#include "osal.h"
#include <unistd.h>

#define OS_VERSION_STRING "PMC-BSP OSAL v1.0.0"

/* 外部初始化函数声明 */
extern void osal_task_table_init(void);
extern void osal_queue_table_init(void);
extern void osal_mutex_table_init(void);

/* 初始化API */
int32 OS_API_Init(void)
{
    osal_task_table_init();
    osal_queue_table_init();
    osal_mutex_table_init();
    return OS_SUCCESS;
}

/* 清理API */
int32 OS_API_Teardown(void)
{
    /* 清理资源 */
    return OS_SUCCESS;
}

/* 版本API */
const char *OS_GetVersionString(void)
{
    return OS_VERSION_STRING;
}

/* 空闲循环 */
int32 OS_IdleLoop(void)
{
    while (1)
    {
        sleep(1);
    }

    /* 不应该到达这里 */
    return OS_ERROR;
}
