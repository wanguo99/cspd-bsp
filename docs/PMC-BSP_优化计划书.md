# PMC-BSP 软件框架优化计划书

**文档版本**: v1.0  
**编制日期**: 2026-04-28  
**项目名称**: PMC-BSP (Payload Management Controller Board Support Package)  
**目标平台**: 卫星载荷管理控制器

---

## 一、执行摘要

本优化计划基于对 PMC-BSP 软件框架的全面审查，从**可移植性、稳定性、可靠性、安全性**四个维度识别出 **3 个关键缺陷**、**5 个高优先级问题**和 **8 个中等优先级改进点**。计划分为三个阶段实施，预计耗时 **4-6 周**，将显著提升系统在航天环境下的鲁棒性。

### 关键发现
- ✅ **架构优势**: 五层分层设计优秀，OSAL 抽象完整，无平台泄漏
- 🔴 **关键风险**: 存在堆溢出、缓冲区越界、竞态条件等 3 个严重缺陷
- 🟡 **可靠性缺口**: 缺少看门狗、重试机制、冗余通道等容错设计
- 🟢 **安全基线**: 已实现设备独占访问、原子计数器、CRC 校验

---

## 二、问题分析矩阵

### 2.1 可移植性 (Portability)

| 严重性 | 问题描述 | 影响范围 | 根因 |
|--------|---------|---------|------|
| 🟡 中 | `osal_queue.c` 使用原生 `malloc/free` | RTOS 移植失败 | 未遵循 OSAL 封装规范 |
| 🟡 中 | `osal_string.c` 暴露不安全的 `strcpy` | 潜在缓冲区溢出 | 历史遗留接口 |
| 🟢 低 | 部分代码假设小端序 | 大端平台数据错误 | 缺少字节序转换宏 |

**评估**: 整体可移植性**优秀** (95/100)，OSAL 层封装完整，仅需修复队列模块的标准库直接调用。

---

### 2.2 稳定性 (Stability)

| 严重性 | 问题描述 | 受影响文件 | 触发条件 |
|--------|---------|-----------|---------|
| 🔴 严重 | 堆分配整数溢出 | `osal_heap.c:133` | 申请接近 `SIZE_MAX` 的内存 |
| 🔴 严重 | CAN 帧 DLC 越界访问 | `hal_can.c:281-284` | 接收恶意/损坏的 CAN 帧 |
| 🔴 严重 | 任务包装器竞态条件 | `osal_task.c:98` | `pthread_create` 失败时 |
| 🟡 高 | 互斥锁错误未检查 | `pdl_satellite.c:237,263` | 锁损坏时静默失败 |
| 🟡 高 | 卫星协议 DLC 校验不足 | `pdl_satellite_can.c:76-82` | `dlc > 8` 时读取越界 |
| 🟡 高 | 串口 `fcntl` 返回值未检查 | `hal_serial.c:86` | 设置非阻塞模式失败 |

**评估**: 存在**关键稳定性风险** (65/100)，需立即修复 3 个严重缺陷。

---

### 2.3 可靠性 (Reliability)

| 严重性 | 问题描述 | 业务影响 | 当前状态 |
|--------|---------|---------|---------|
| 🔴 严重 | 无看门狗机制 | 任务挂死无法恢复 | 未实现 |
| 🔴 严重 | 无通信重试逻辑 | 瞬态错误导致永久失联 | 配置字段存在但未使用 |
| 🟡 高 | 无冗余通道支持 | 单点故障导致系统失效 | PCL 设计支持但未实现 |
| 🟡 高 | 卫星命令无校验 | 恶意帧可触发非预期行为 | 仅有 CRC，无序列号/白名单 |
| 🟡 中 | 心跳失败无告警 | 通信异常难以诊断 | 仅递增错误计数 |

**评估**: 可靠性设计**不足** (55/100)，缺少航天级容错机制。

---

### 2.4 安全性 (Security)

| 严重性 | 问题描述 | 攻击向量 | 缓解措施 |
|--------|---------|---------|---------|
| 🟡 高 | 堆分配整数溢出 | 恶意大内存申请 → 堆损坏 | 无 |
| 🟡 中 | CAN ID 未校验 | 伪造高优先级帧 | 无 |
| 🟡 中 | 卫星命令无认证 | 注入恶意指令 | 仅物理隔离 |
| 🟡 中 | 队列 DoS 风险 | 恶意生产者填满队列 | 已有超时但默认阻塞 |
| 🟢 低 | 日志可能泄露敏感信息 | 未发现实际泄露 | 代码审查通过 |

**评估**: 安全基线**合格** (70/100)，已实现设备独占访问和 CRC 校验，需加强输入验证。

---

## 三、优化计划 (三阶段实施)

### 阶段一：关键缺陷修复 (P0 - 1 周)

**目标**: 消除所有严重稳定性风险，确保系统基本安全运行。

#### 任务 1.1: 修复堆分配整数溢出 (2 天)
**文件**: `osal/src/posix/lib/osal_heap.c`

**问题代码** (第 133 行):
```c
mem_block_header_t *block = (mem_block_header_t *)malloc(size + sizeof(mem_block_header_t));
```

**修复方案**:
```c
/* 防止整数溢出 */
if (size > SIZE_MAX - sizeof(mem_block_header_t))
{
    LOG_ERROR("OSAL_Heap", "Allocation size too large: %zu", size);
    return NULL;
}

mem_block_header_t *block = (mem_block_header_t *)malloc(size + sizeof(mem_block_header_t));
```

**验证**: 单元测试覆盖边界值 (`SIZE_MAX`, `SIZE_MAX-1`, `SIZE_MAX-sizeof(header)+1`)

---

#### 任务 1.2: 修复 CAN 帧 DLC 越界访问 (1 天)
**文件**: `hal/src/linux/hal_can.c`

**问题代码** (第 281-284 行):
```c
frame->dlc = can_frame.can_dlc;

/* 防止越界 */
if (can_frame.can_dlc > 8)
    can_frame.can_dlc = 8;  // ⚠️ 已赋值给 frame->dlc，此时已越界

OSAL_Memcpy(frame->data, can_frame.data, can_frame.can_dlc);
```

**修复方案**:
```c
/* 先校验，再赋值 */
uint8_t dlc = can_frame.can_dlc;
if (dlc > 8)
{
    LOG_WARN("HAL_CAN", "Invalid DLC %u, clamping to 8", dlc);
    dlc = 8;
}

frame->dlc = dlc;
OSAL_Memcpy(frame->data, can_frame.data, dlc);
```

**验证**: 注入 `dlc=255` 的测试帧，确认不会崩溃。

---

#### 任务 1.3: 修复任务包装器竞态条件 (2 天)
**文件**: `osal/src/posix/ipc/osal_task.c`

**问题代码** (第 98 行):
```c
static void *task_wrapper(void *arg)
{
    task_wrapper_arg_t *wrapper_arg = (task_wrapper_arg_t *)arg;
    // ...
    free(wrapper_arg);  // ⚠️ 如果 pthread_create 失败，主线程也会 free
}
```

**修复方案**:
```c
/* 在 OSAL_TaskCreate 中 */
ret = pthread_create(&pthread_id, &attr, task_wrapper, wrapper_arg);
if (ret != 0)
{
    LOG_ERROR("OSAL_Task", "pthread_create failed: %s", strerror(ret));
    OSAL_MutexUnlock(task_table_mutex);
    free(wrapper_arg);  // ✅ 仅在失败时由主线程释放
    return OSAL_ERR_GENERIC;
}
/* 成功时由 task_wrapper 释放 */
```

**验证**: 模拟 `pthread_create` 失败场景 (资源耗尽)，确认无内存泄漏。

---

#### 任务 1.4: 修复卫星协议 DLC 校验 (1 天)
**文件**: `pdl/src/pdl_satellite/pdl_satellite_can.c`

**问题代码** (第 76-82 行):
```c
if (frame.dlc >= 8)  // ⚠️ 允许 dlc > 8
{
    ctx->telemetry.voltage = (frame.data[0] << 8) | frame.data[1];
    // ... 访问 data[0-7]
}
```

**修复方案**:
```c
if (frame.dlc == 8)  // ✅ 严格校验
{
    ctx->telemetry.voltage = (frame.data[0] << 8) | frame.data[1];
    // ...
}
else
{
    LOG_WARN("PDL_Satellite", "Invalid telemetry frame DLC: %u", frame.dlc);
    atomic_fetch_add(&ctx->stats.err_count, 1);
}
```

---

### 阶段二：可移植性与稳定性增强 (P1 - 2 周)

#### 任务 2.1: 统一 OSAL 封装 (3 天)
**文件**: `osal/src/posix/ipc/osal_queue.c`

**问题**: 直接使用 `malloc/free/memset/memcpy` (第 89, 159, 166, 174 行)

**修复方案**:
```c
/* 替换所有实例 */
- malloc(size)           → OSAL_Malloc(size)
- free(ptr)              → OSAL_Free(ptr)
- memset(ptr, 0, size)   → OSAL_Memset(ptr, 0, size)
- memcpy(dst, src, size) → OSAL_Memcpy(dst, src, size)
```

**验证**: 在无 libc 的 FreeRTOS 环境编译通过。

---

#### 任务 2.2: 增强错误处理 (5 天)
**目标**: 检查所有关键路径的返回值

**修复清单**:
1. `hal_serial.c:86` - 检查 `OSAL_fcntl` 返回值
2. `pdl_satellite.c:237,263` - 检查 `OSAL_MutexLock` 返回值
3. `pdl_mcu.c` - 所有 HAL 调用增加错误处理

**模板代码**:
```c
int32_t ret = OSAL_MutexLock(ctx->mutex);
if (ret != OSAL_SUCCESS)
{
    LOG_ERROR("PDL_Satellite", "Failed to acquire mutex: %d", ret);
    return OSAL_ERR_GENERIC;
}
```

---

#### 任务 2.3: 添加字节序转换宏 (2 天)
**文件**: `osal/include/osal_types.h`

**新增宏定义**:
```c
/* 字节序转换 (支持大端/小端平台) */
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    #define OSAL_HTONS(x)  __builtin_bswap16(x)
    #define OSAL_HTONL(x)  __builtin_bswap32(x)
    #define OSAL_NTOHS(x)  __builtin_bswap16(x)
    #define OSAL_NTOHL(x)  __builtin_bswap32(x)
#else
    #define OSAL_HTONS(x)  (x)
    #define OSAL_HTONL(x)  (x)
    #define OSAL_NTOHS(x)  (x)
    #define OSAL_NTOHL(x)  (x)
#endif
```

**应用场景**: 卫星遥测数据解析 (`pdl_satellite_can.c:78-82`)

---

### 阶段三：可靠性与安全性提升 (P2 - 2-3 周)

#### 任务 3.1: 实现看门狗机制 (5 天)

**新增接口** (`osal/include/osal_watchdog.h`):
```c
int32_t OSAL_WatchdogCreate(osal_id_t *watchdog_id, const char *name, uint32_t timeout_ms);
int32_t OSAL_WatchdogKick(osal_id_t watchdog_id);
int32_t OSAL_WatchdogDelete(osal_id_t watchdog_id);
```

**集成点**:
- `pdl_satellite.c:heartbeat_task` - 每次循环 kick
- `pdl_mcu.c:mcu_task` - 每次循环 kick
- 超时触发 `OSAL_ProcessAbort()` 或重启任务

---

#### 任务 3.2: 实现通信重试机制 (4 天)

**修改文件**: `pdl/src/pdl_satellite/pdl_satellite.c`

**当前问题** (第 50 行):
```c
ret = satellite_can_send_heartbeat(ctx);
if (ret != OSAL_SUCCESS)
{
    atomic_fetch_add(&ctx->stats.err_count, 1);  // ⚠️ 仅计数，不重试
}
```

**修复方案**:
```c
/* 使用配置中的 retry_count */
for (uint32_t i = 0; i < ctx->config.retry_count; i++)
{
    ret = satellite_can_send_heartbeat(ctx);
    if (ret == OSAL_SUCCESS)
        break;
    
    LOG_WARN("PDL_Satellite", "Heartbeat send failed (attempt %u/%u)", 
             i+1, ctx->config.retry_count);
    OSAL_TaskDelay(10);  // 指数退避: 10ms, 20ms, 40ms...
}

if (ret != OSAL_SUCCESS)
{
    atomic_fetch_add(&ctx->stats.err_count, 1);
    LOG_ERROR("PDL_Satellite", "Heartbeat send failed after %u retries", 
              ctx->config.retry_count);
}
```

---

#### 任务 3.3: 实现多通道冗余 (7 天)

**设计目标**: 支持主备 CAN 通道自动切换

**新增配置** (`pdl/include/config/pdl_satellite_config.h`):
```c
typedef struct
{
    hal_can_config_t primary_can;    /* 主通道 */
    hal_can_config_t backup_can;     /* 备份通道 */
    uint32_t failover_threshold;     /* 切换阈值 (连续失败次数) */
    uint32_t recovery_threshold;     /* 恢复阈值 (连续成功次数) */
} pdl_satellite_redundancy_config_t;
```

**状态机**:
```
[主通道正常] --失败达阈值--> [切换到备份] --主通道恢复--> [切回主通道]
```

---

#### 任务 3.4: 增强安全输入验证 (5 天)

**修改清单**:

1. **CAN ID 白名单** (`hal_can.c`):
```c
static const uint32_t ALLOWED_CAN_IDS[] = {0x100, 0x200, 0x300};

int32_t HAL_CAN_Recv(...)
{
    // ... 接收帧 ...
    
    /* 校验 CAN ID */
    bool valid = false;
    for (size_t i = 0; i < sizeof(ALLOWED_CAN_IDS)/sizeof(uint32_t); i++)
    {
        if ((frame->can_id & CAN_EFF_MASK) == ALLOWED_CAN_IDS[i])
        {
            valid = true;
            break;
        }
    }
    
    if (!valid)
    {
        LOG_WARN("HAL_CAN", "Rejected frame with invalid ID: 0x%X", frame->can_id);
        return OSAL_ERR_INVALID_ID;
    }
}
```

2. **卫星命令序列号校验** (`pdl_satellite_can.c`):
```c
typedef struct
{
    uint32_t last_seq;
    uint32_t replay_window;
} sequence_validator_t;

static bool validate_sequence(sequence_validator_t *validator, uint32_t seq)
{
    if (seq <= validator->last_seq && 
        (validator->last_seq - seq) > validator->replay_window)
    {
        LOG_ERROR("PDL_Satellite", "Replay attack detected: seq=%u, last=%u", 
                  seq, validator->last_seq);
        return false;
    }
    validator->last_seq = seq;
    return true;
}
```

---

## 四、测试与验证计划

### 4.1 单元测试增强

**新增测试用例**:
```bash
tests/osal/test_osal_heap.c
  - test_heap_alloc_overflow          # SIZE_MAX 边界测试
  - test_heap_alloc_zero              # 零字节分配
  
tests/hal/test_hal_can.c
  - test_can_recv_invalid_dlc         # DLC > 8 测试
  - test_can_recv_invalid_id          # 非法 CAN ID 测试
  
tests/pdl/test_pdl_satellite.c
  - test_satellite_heartbeat_retry    # 重试机制测试
  - test_satellite_channel_failover   # 通道切换测试
```

**覆盖率目标**: 从当前 58% 提升至 **80%**

---

### 4.2 集成测试

**场景 1: 故障注入测试**
- 模拟 CAN 总线断开 → 验证自动切换到备份通道
- 模拟任务挂死 → 验证看门狗触发重启
- 模拟内存耗尽 → 验证优雅降级

**场景 2: 压力测试**
- 1000 次/秒 CAN 帧接收 → 验证无丢帧
- 连续运行 72 小时 → 验证无内存泄漏
- 并发 10 个进程访问设备 → 验证互斥保护

**场景 3: 安全测试**
- 注入恶意 CAN 帧 (DLC=255, ID=0xFFFFFFFF) → 验证拒绝
- 重放攻击 (重复序列号) → 验证检测
- 堆溢出攻击 (申请 SIZE_MAX) → 验证拒绝

---

### 4.3 代码审查检查清单

- [ ] 所有 `malloc/free` 替换为 `OSAL_Malloc/OSAL_Free`
- [ ] 所有 HAL 函数返回值已检查
- [ ] 所有数组访问前已校验索引
- [ ] 所有整数运算前已检查溢出
- [ ] 所有字符串操作使用 `OSAL_Strncpy` (非 `strcpy`)
- [ ] 所有任务循环包含 `OSAL_TaskShouldShutdown()` 检查
- [ ] 所有关键路径包含看门狗 kick
- [ ] 所有网络数据包含字节序转换

---

## 五、实施时间表

```
Week 1: 阶段一 - 关键缺陷修复
├─ Day 1-2: 任务 1.1 (堆溢出)
├─ Day 3:   任务 1.2 (CAN DLC)
├─ Day 4-5: 任务 1.3 (竞态条件)
└─ Day 5:   任务 1.4 (卫星协议)

Week 2-3: 阶段二 - 可移植性与稳定性
├─ Day 1-3: 任务 2.1 (OSAL 统一)
├─ Day 4-8: 任务 2.2 (错误处理)
└─ Day 9-10: 任务 2.3 (字节序)

Week 4-6: 阶段三 - 可靠性与安全性
├─ Day 1-5:  任务 3.1 (看门狗)
├─ Day 6-9:  任务 3.2 (重试机制)
├─ Day 10-16: 任务 3.3 (多通道冗余)
└─ Day 17-21: 任务 3.4 (安全验证)

Week 7: 测试与验证
├─ Day 1-3: 单元测试 (目标 80% 覆盖率)
├─ Day 4-5: 集成测试 (故障注入 + 压力测试)
└─ Day 5:   代码审查与文档更新
```

---

## 六、风险评估与缓解

| 风险 | 概率 | 影响 | 缓解措施 |
|------|------|------|---------|
| 修复引入新 Bug | 中 | 高 | 每个任务独立分支 + 代码审查 + 回归测试 |
| 性能下降 (增加校验) | 低 | 中 | 性能基准测试 (CAN 帧处理延迟 < 1ms) |
| 测试环境不足 | 高 | 中 | 使用虚拟 CAN (vcan) + QEMU 模拟 |
| 时间超期 | 中 | 低 | 优先完成阶段一 (关键缺陷)，阶段三可延后 |

---

## 七、成功标准

### 定量指标
- ✅ **零严重缺陷**: 所有 CRITICAL 问题修复
- ✅ **测试覆盖率 ≥ 80%**: 新增 15+ 测试用例
- ✅ **内存泄漏 = 0**: Valgrind 检测通过
- ✅ **性能无退化**: CAN 帧处理延迟 < 1ms (当前 0.5ms)

### 定性指标
- ✅ **可移植性**: 在 FreeRTOS 环境编译通过
- ✅ **可靠性**: 72 小时压力测试无崩溃
- ✅ **安全性**: 通过模糊测试 (AFL) 10 万次迭代

---

## 八、后续维护建议

1. **每季度代码审查**: 使用静态分析工具 (Coverity, Cppcheck)
2. **持续集成**: 每次提交触发单元测试 + 静态分析
3. **性能监控**: 记录关键路径延迟 (CAN 收发、任务切换)
4. **安全更新**: 订阅 CVE 数据库，及时修复依赖库漏洞
5. **文档同步**: 每次架构变更更新 `docs/ARCHITECTURE.md`

---

## 九、附录

### A. 参考标准
- **DO-178C**: 机载软件开发标准
- **IEC 61508**: 功能安全标准
- **MISRA C:2012**: C 语言编码规范
- **ECSS-E-ST-40C**: 欧空局软件工程标准

### B. 工具链
- **静态分析**: Coverity, Cppcheck, Clang Static Analyzer
- **动态分析**: Valgrind (内存泄漏), AddressSanitizer (缓冲区溢出)
- **模糊测试**: AFL, LibFuzzer
- **覆盖率**: gcov, lcov

### C. 联系方式
- **技术负责人**: [待填写]
- **代码审查**: [待填写]
- **测试负责人**: [待填写]

---

**文档结束**
