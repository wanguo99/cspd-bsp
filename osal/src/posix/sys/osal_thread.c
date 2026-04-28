/************************************************************************
 * OSAL - POSIX线程封装实现
 ************************************************************************/

#include "sys/osal_thread.h"
#include <pthread.h>

int32_t OSAL_pthread_create(osal_thread_t *thread,
                            void *attr,
                            osal_thread_func_t start_routine,
                            void *arg)
{
    pthread_t pt;
    int32_t ret = pthread_create(&pt, (pthread_attr_t *)attr, start_routine, arg);
    if (0 == ret && NULL != thread) {
        *thread = (osal_thread_t)pt;
    }
    return ret;
}

int32_t OSAL_pthread_join(osal_thread_t thread, void **retval)
{
    pthread_t pt = (pthread_t)thread;
    return pthread_join(pt, retval);
}
