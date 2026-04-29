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

    union {
        void *osal_attr;
        pthread_attr_t *posix_attr;
    } attr_union;

    attr_union.osal_attr = attr;

    int32_t ret = pthread_create(&pt, attr_union.posix_attr, start_routine, arg);
    if (0 == ret && NULL != thread) {
        union {
            pthread_t posix_thread;
            osal_thread_t osal_thread;
        } thread_union;
        thread_union.posix_thread = pt;
        *thread = thread_union.osal_thread;
    }
    return ret;
}

int32_t OSAL_pthread_join(osal_thread_t thread, void **retval)
{
    union {
        osal_thread_t osal_thread;
        pthread_t posix_thread;
    } thread_union;

    thread_union.osal_thread = thread;
    return pthread_join(thread_union.posix_thread, retval);
}
