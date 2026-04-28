# PMC-BSP TODO List

## 高优先级

### 1. 禁止使用类型强制转换 - 修复 OSAL_TaskCreate 参数类型问题

**问题描述**：
当前代码中大量使用 `(uint32_t *)` 强制转换来传递用户数据给 `OSAL_TaskCreate`，这违反了类型安全原则，容易导致：
- 类型不匹配导致的未定义行为
- 编译器无法检测类型错误
- 代码可读性差

**当前问题代码示例**：
```c
// 错误：将结构体指针强制转换为 uint32_t*
OSAL_TaskCreate(&task_id, "task_name", 
                task_func, (uint32_t *)&thread_data,  // 类型不安全
                32 * 1024, 100, 0);
```

**影响范围**：
- 测试代码：`tests/osal/test_osal_task.c`, `tests/osal/test_osal_atomic.c`, `tests/osal/test_osal_log.c`
- PDL代码：`pdl/src/pdl_satellite/pdl_satellite.c`
- 其他模块：约76处类型强制转换

**解决方案**：

#### 方案1：修改 OSAL_TaskCreate API（推荐）
将 `stack_pointer` 参数改为 `void *user_arg`，明确其用途：

```c
// 修改前
int32_t OSAL_TaskCreate(osal_id_t *task_id,
                    const str_t *task_name,
                    osal_task_entry function_pointer,
                    uint32_t *stack_pointer,  // 误导性参数名
                    uint32_t stack_size,
                    uint32_t priority,
                    uint32_t flags);

// 修改后
int32_t OSAL_TaskCreate(osal_id_t *task_id,
                    const str_t *task_name,
                    osal_task_entry function_pointer,
                    void *user_arg,  // 明确的用户参数
                    uint32_t stack_size,
                    uint32_t priority,
                    uint32_t flags);
```

**优点**：
- 类型安全，无需强制转换
- API语义清晰
- 符合POSIX pthread_create的设计

**缺点**：
- 需要修改所有调用点
- 需要更新文档

#### 方案2：保持API不变，使用宏封装
```c
#define OSAL_TaskCreate(id, name, func, arg, stack, prio, flags) \
    OSAL_TaskCreate_Internal(id, name, func, (uint32_t *)(arg), stack, prio, flags)
```

**优点**：
- 调用代码无需修改

**缺点**：
- 仍然存在类型不安全问题
- 治标不治本

**实施步骤**：
1. [ ] 修改 `osal/include/ipc/osal_task.h` 中的 API 声明
2. [ ] 修改 `osal/src/posix/ipc/osal_task.c` 中的实现
3. [ ] 更新所有测试代码中的调用
4. [ ] 更新 PDL 代码中的调用
5. [ ] 搜索并修复所有其他调用点
6. [ ] 更新文档和注释
7. [ ] 运行完整测试套件验证

**预计工作量**：2-3小时

**相关文件**：
- `osal/include/ipc/osal_task.h`
- `osal/src/posix/ipc/osal_task.c`
- `tests/osal/test_osal_task.c`
- `tests/osal/test_osal_atomic.c`
- `tests/osal/test_osal_log.c`
- `pdl/src/pdl_satellite/pdl_satellite.c`
- 其他使用 OSAL_TaskCreate 的文件

**参考**：
- POSIX pthread_create: `int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);`
- 最近修复的 bug：commit 5c24ac1 "修复：解决osal_atomic多线程测试失败问题"

---

## 中优先级

### 2. 代码审查：检查其他类型强制转换
- [ ] 审查所有 `(void *)` 转换
- [ ] 审查所有 `(char *)` 转换
- [ ] 审查所有函数指针转换
- [ ] 建立类型转换使用规范

---

## 低优先级

### 3. 改进测试框架
- [x] 添加 `-p` 参数打印测试用例名
- [ ] 添加测试覆盖率统计
- [ ] 添加性能测试支持

---

**创建日期**：2026-04-28
**最后更新**：2026-04-28
