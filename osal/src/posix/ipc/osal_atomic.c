/************************************************************************
 * OSAL POSIX实现 - 原子操作
 ************************************************************************/

#include "ipc/osal_atomic.h"
#include <stdatomic.h>

void OSAL_AtomicInit(osal_atomic_uint32_t *atomic, uint32_t value)
{
    atomic_init((_Atomic uint32_t *)&atomic->value, value);
}

uint32_t OSAL_AtomicLoad(const osal_atomic_uint32_t *atomic)
{
    return atomic_load((_Atomic uint32_t *)&atomic->value);
}

void OSAL_AtomicStore(osal_atomic_uint32_t *atomic, uint32_t value)
{
    atomic_store((_Atomic uint32_t *)&atomic->value, value);
}

uint32_t OSAL_AtomicFetchAdd(osal_atomic_uint32_t *atomic, uint32_t value)
{
    return atomic_fetch_add((_Atomic uint32_t *)&atomic->value, value);
}

uint32_t OSAL_AtomicFetchSub(osal_atomic_uint32_t *atomic, uint32_t value)
{
    return atomic_fetch_sub((_Atomic uint32_t *)&atomic->value, value);
}

uint32_t OSAL_AtomicIncrement(osal_atomic_uint32_t *atomic)
{
    return atomic_fetch_add((_Atomic uint32_t *)&atomic->value, 1) + 1;
}

uint32_t OSAL_AtomicDecrement(osal_atomic_uint32_t *atomic)
{
    return atomic_fetch_sub((_Atomic uint32_t *)&atomic->value, 1) - 1;
}

bool OSAL_AtomicCompareExchange(osal_atomic_uint32_t *atomic, uint32_t expected, uint32_t desired)
{
    return atomic_compare_exchange_strong((_Atomic uint32_t *)&atomic->value, &expected, desired);
}
