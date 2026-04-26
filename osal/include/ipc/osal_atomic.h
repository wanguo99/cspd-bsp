/************************************************************************
 * OSAL - 原子操作封装
 *
 * 功能：
 * - 封装原子操作，支持线程安全的计数器
 * - 提供原子加载、存储、增减操作
 * - 便于RTOS移植
 *
 * 设计原则：
 * - 使用固定大小类型（uint32）
 * - 提供常用原子操作接口
 * - 跨平台兼容
 ************************************************************************/

#ifndef OSAL_ATOMIC_H
#define OSAL_ATOMIC_H

#include "osal_types.h"

/*===========================================================================
 * 原子类型定义
 *===========================================================================*/

/**
 * @brief 原子无符号32位整数类型
 */
typedef struct {
    volatile uint32 value;
} osal_atomic_uint32_t;

/*===========================================================================
 * 原子操作接口
 *===========================================================================*/

/**
 * @brief 初始化原子变量
 * @param atomic 原子变量指针
 * @param value 初始值
 */
void OSAL_AtomicInit(osal_atomic_uint32_t *atomic, uint32 value);

/**
 * @brief 原子加载（读取）
 * @param atomic 原子变量指针
 * @return 当前值
 */
uint32 OSAL_AtomicLoad(const osal_atomic_uint32_t *atomic);

/**
 * @brief 原子存储（写入）
 * @param atomic 原子变量指针
 * @param value 要写入的值
 */
void OSAL_AtomicStore(osal_atomic_uint32_t *atomic, uint32 value);

/**
 * @brief 原子加法（fetch_add）
 * @param atomic 原子变量指针
 * @param value 要增加的值
 * @return 增加前的值
 */
uint32 OSAL_AtomicFetchAdd(osal_atomic_uint32_t *atomic, uint32 value);

/**
 * @brief 原子减法（fetch_sub）
 * @param atomic 原子变量指针
 * @param value 要减少的值
 * @return 减少前的值
 */
uint32 OSAL_AtomicFetchSub(osal_atomic_uint32_t *atomic, uint32 value);

/**
 * @brief 原子自增（++）
 * @param atomic 原子变量指针
 * @return 自增后的值
 */
uint32 OSAL_AtomicIncrement(osal_atomic_uint32_t *atomic);

/**
 * @brief 原子自减（--）
 * @param atomic 原子变量指针
 * @return 自减后的值
 */
uint32 OSAL_AtomicDecrement(osal_atomic_uint32_t *atomic);

/**
 * @brief 原子比较并交换（CAS）
 * @param atomic 原子变量指针
 * @param expected 期望值
 * @param desired 目标值
 * @return true 交换成功，false 交换失败
 */
bool OSAL_AtomicCompareExchange(osal_atomic_uint32_t *atomic, uint32 expected, uint32 desired);

#endif /* OSAL_ATOMIC_H */
