/************************************************************************
 * OSAL POSIX实现 - 原子操作
 ************************************************************************/

#include "ipc/osal_atomic.h"
#include <stdatomic.h>

void OSAL_AtomicInit(osal_atomic_uint32_t *atomic, uint32 value)
{
    atomic_init((_Atomic uint32 *)&atomic->value, value);
}

uint32 OSAL_AtomicLoad(const osal_atomic_uint32_t *atomic)
{
    return atomic_load((_Atomic uint32 *)&atomic->value);
}

void OSAL_AtomicStore(osal_atomic_uint32_t *atomic, uint32 value)
{
    atomic_store((_Atomic uint32 *)&atomic->value, value);
}

uint32 OSAL_AtomicFetchAdd(osal_atomic_uint32_t *atomic, uint32 value)
{
    return atomic_fetch_add((_Atomic uint32 *)&atomic->value, value);
}

uint32 OSAL_AtomicFetchSub(osal_atomic_uint32_t *atomic, uint32 value)
{
    return atomic_fetch_sub((_Atomic uint32 *)&atomic->value, value);
}

uint32 OSAL_AtomicIncrement(osal_atomic_uint32_t *atomic)
{
    return atomic_fetch_add((_Atomic uint32 *)&atomic->value, 1) + 1;
}

uint32 OSAL_AtomicDecrement(osal_atomic_uint32_t *atomic)
{
    return atomic_fetch_sub((_Atomic uint32 *)&atomic->value, 1) - 1;
}

bool OSAL_AtomicCompareExchange(osal_atomic_uint32_t *atomic, uint32 expected, uint32 desired)
{
    return atomic_compare_exchange_strong((_Atomic uint32 *)&atomic->value, &expected, desired);
}
