# PMC-BSP 编码规范

## 1. 总则

### 1.1 适用范围
本规范适用于 PMC-BSP 项目的所有 C 代码，包括 OSAL、HAL、XConfig、PDL 和 Apps 层。

### 1.2 设计原则
- **航空航天级可靠性**：代码必须具备容错和自愈能力
- **分层隔离**：严格遵循 5 层架构，不允许跨层直接调用
- **模块独立**：各模块只依赖自己的配置，便于多人协作
- **硬件抽象**：上层应用与底层硬件解耦
- **零拷贝传输**：关键路径避免不必要的内存拷贝

### 1.3 强制要求
- 遵循 **Linux Kernel Coding Style**
- 遵循 **MISRA C** 规范（航空航天安全标准）
- 编译选项：`-Wall -Wextra -Werror`（所有警告视为错误）
- C 标准：**C99/C11**

---

## 2. 文件组织

### 2.1 目录结构
每个模块遵循标准目录结构：
```
module/
├── include/              # 头文件
│   ├── api/             # 对外API（可选，仅顶层模块）
│   ├── internal/        # 内部公共头文件（可选）
│   ├── peripheral/      # 外设私有头文件（可选）
│   └── config/          # 模块配置（必需）
├── src/                 # 源代码
│   └── linux/          # Linux平台实现
└── platform/            # 平台配置（仅XConfig）
```

### 2.2 头文件组织
**三层头文件架构**（参考 Linux Kernel）：
- **api/**：对外公共接口（供上层使用）
- **internal/**：内部公共头文件（模块内部共享）
- **peripheral/**：外设私有头文件（设备特定）

**示例**（XConfig 层）：
```c
/* PDL层使用 */
#include "xconfig_api.h"

/* XConfig内部源文件使用 */
#include "internal/xconfig.h"
```

### 2.3 文件头注释
每个文件必须包含标准头注释：
```c
/************************************************************************
 * 模块名称
 *
 * 功能：
 * - 功能点1
 * - 功能点2
 *
 * 职责：（可选）
 * - 职责描述
 *
 * 命名规范：（可选）
 * - PREFIX_*  - 接口说明
 ************************************************************************/
```

**实际示例**：
```c
/************************************************************************
 * 硬件配置库API实现
 *
 * 命名规范：
 * - XCONFIG_*       - 通用接口
 * - XCONFIG_HW_*    - 硬件配置接口
 * - XCONFIG_APP_*   - APP配置接口
 ************************************************************************/
```

---

## 3. 命名规范

### 3.1 分层命名前缀
| 层级 | 前缀 | 示例 |
|------|------|------|
| OSAL | `OS_`, `OSAL_` | `OS_TaskCreate()`, `OSAL_TaskDelay()` |
| HAL | `HAL_` | `HAL_CAN_Init()`, `HAL_UART_Send()` |
| XConfig | `XCONFIG_` | `XCONFIG_Init()`, `XCONFIG_HW_FindMCU()` |
| PDL | `PDL_`, `模块名_` | `SatellitePDL_Init()`, `PayloadBMC_PowerOn()` |
| Apps | `应用名_` | `CanGateway_ProcessMsg()` |

### 3.2 函数命名
**格式**：`<层级前缀>_<模块>_<动作><对象>()`

**示例**：
```c
/* OSAL层 */
int32 OS_TaskCreate(osal_id_t *task_id, const char *name, ...);
int32 OS_QueueCreate(osal_id_t *queue_id, uint32 depth, ...);

/* HAL层 */
int32 HAL_CAN_Init(const char *interface);
int32 HAL_CAN_Send(int32 fd, const can_frame_t *frame);

/* XConfig层 */
int32 XCONFIG_Init(void);
const xconfig_mcu_cfg_t* XCONFIG_HW_FindMCU(const xconfig_board_config_t *board, const char *name);
const xconfig_app_config_t* XCONFIG_APP_Find(const xconfig_board_config_t *board, const char *app_name);

/* PDL层 */
int32 SatellitePDL_Init(const satellite_service_config_t *config);
int32 PayloadBMC_PowerOn(uint32 slot_id);
```

### 3.3 变量命名
- **全局变量**：`g_` 前缀，如 `g_registry`
- **静态变量**：`s_` 前缀（可选）或直接 `static`
- **局部变量**：小写下划线分隔，如 `task_id`, `config_ptr`
- **常量**：全大写下划线分隔，如 `MAX_BOARD_CONFIGS`

**示例**：
```c
/* 全局变量 */
static xconfig_registry_t g_registry = {0};
static bool g_initialized = false;

/* 局部变量 */
int32 ret;
uint32 success_count = 0;
const xconfig_board_config_t *config = NULL;

/* 常量 */
#define MAX_BOARD_CONFIGS 32
#define CONFIG_COUNT (sizeof(g_all_configs) / sizeof(g_all_configs[0]))
```

### 3.4 类型命名
- **结构体**：`<模块>_<名称>_t`，如 `xconfig_board_config_t`
- **枚举**：`<模块>_<名称>_e`，如 `xconfig_device_type_e`
- **枚举值**：全大写，如 `XCONFIG_DEV_MCU`
- **函数指针**：`<模块>_<名称>_t`，如 `satellite_cmd_callback_t`

**示例**：
```c
/* 结构体 */
typedef struct {
    const xconfig_board_config_t *configs[MAX_BOARD_CONFIGS];
    uint32 count;
    const xconfig_board_config_t *current;
} xconfig_registry_t;

/* 枚举 */
typedef enum {
    XCONFIG_DEV_MCU = 0,
    XCONFIG_DEV_BMC,
    XCONFIG_DEV_SATELLITE,
    XCONFIG_DEV_SENSOR,
    XCONFIG_DEV_STORAGE
} xconfig_device_type_e;

/* 函数指针 */
typedef void (*satellite_cmd_callback_t)(uint8 cmd_type, const uint8 *data, void *user_data);
```

### 3.5 宏命名
- **配置宏**：全大写，如 `TASK_STACK_SIZE_DEFAULT`
- **功能宏**：全大写，如 `LOG_INFO`, `TEST_ASSERT_EQUAL`

---

## 4. 代码风格

### 4.1 缩进与空格
- **缩进**：4 个空格（禁止使用 Tab）
- **行宽**：建议不超过 100 字符
- **函数参数**：过长时换行对齐

**示例**：
```c
/* 正确 */
int32 XCONFIG_Register(const xconfig_board_config_t *config)
{
    if (config == NULL) {
        LOG_ERROR("XCONFIG", "Invalid config pointer");
        return OS_ERROR;
    }
    
    return OS_SUCCESS;
}

/* 参数换行 */
const xconfig_satellite_cfg_t* XCONFIG_HW_FindSatellite(
    const xconfig_board_config_t *board,
    const char *name)
{
    /* ... */
}
```

### 4.2 大括号风格
- **函数**：左大括号另起一行
- **控制语句**：左大括号跟随语句（K&R 风格）

**示例**：
```c
/* 函数 */
int32 XCONFIG_Init(void)
{
    /* ... */
}

/* 控制语句 */
if (config == NULL) {
    return OS_ERROR;
}

for (uint32 i = 0; i < count; i++) {
    /* ... */
}

while (!OSAL_TaskShouldShutdown()) {
    /* ... */
}
```

### 4.3 注释规范
**原则**：默认不写注释，仅在以下情况添加：
- 非显而易见的 WHY（隐藏约束、微妙不变量、特定 Bug 的 Workaround）
- 硬件时序和寄存器操作（必须引用芯片手册章节号）
- 复杂算法的关键步骤

**禁止**：
- 解释 WHAT（代码本身已说明）
- 引用当前任务/调用者（"used by X", "added for Y"）
- 冗长的多行注释块

**示例**：
```c
/* 正确：解释WHY */
/* 优先级：环境变量 > 编译选项 > 默认配置 */
platform = getenv("XCONFIG_PLATFORM");

/* 检查重复注册（避免配置冲突） */
for (uint32 i = 0; i < g_registry.count; i++) {
    /* ... */
}

/* 错误：解释WHAT */
/* 初始化注册表 */  // ❌ 代码已经说明
memset(&g_registry, 0, sizeof(g_registry));

/* 错误：引用任务 */
/* 这个函数用于CAN网关 */  // ❌ 属于PR描述，不属于代码
```

### 4.4 函数文档注释
**对外 API** 必须使用 Doxygen 风格注释：
```c
/**
 * @brief 注册硬件配置
 *
 * @param config 配置指针
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 失败
 */
int32 XCONFIG_Register(const xconfig_board_config_t *config);
```

**内部函数** 可省略文档注释（函数名已足够清晰）。

---

## 5. 错误处理

### 5.1 返回值规范
- **成功**：`OS_SUCCESS` (0)
- **失败**：`OS_ERROR` (-1) 或具体错误码
- **所有函数返回值必须检查**

**示例**：
```c
/* 正确 */
int32 ret = XCONFIG_Register(config);
if (ret != OS_SUCCESS) {
    LOG_ERROR("XCONFIG", "Failed to register config");
    return OS_ERROR;
}

/* 错误：未检查返回值 */
XCONFIG_Register(config);  // ❌
```

### 5.2 参数校验
**对外 API** 必须校验所有参数：
```c
int32 XCONFIG_Register(const xconfig_board_config_t *config)
{
    if (!g_initialized) {
        LOG_ERROR("XCONFIG", "Library not initialized");
        return OS_ERROR;
    }

    if (config == NULL) {
        LOG_ERROR("XCONFIG", "Invalid config pointer");
        return OS_ERROR;
    }

    /* ... */
}
```

**内部函数** 可使用断言（Debug 模式）：
```c
static void internal_process(const data_t *data)
{
    assert(data != NULL);  /* Debug模式检查 */
    /* ... */
}
```

### 5.3 边界检查
数组访问必须检查边界：
```c
/* 正确 */
if (id >= board->mcu_count) {
    return NULL;
}
return board->mcus[id];

/* 错误：未检查边界 */
return board->mcus[id];  // ❌ 可能越界
```

---

## 6. 内存管理

### 6.1 禁止事项
- **关键路径禁止动态分配**：中断处理、DMA 传输禁止使用 `malloc`/`kmalloc`
- **必须检查返回值**：所有 `malloc` 必须检查是否为 NULL
- **禁止内存泄漏**：每个 `malloc` 必须有对应的 `free`

### 6.2 推荐做法
- **预分配内存池**：关键路径使用静态缓冲区或内存池
- **栈上分配**：小对象优先使用栈（注意栈大小限制）

**示例**：
```c
/* 正确：预分配 */
static uint8 g_rx_buffer[1024];

/* 正确：栈上分配 */
void process_message(void)
{
    can_frame_t frame;  /* 小对象，栈上分配 */
    /* ... */
}

/* 正确：检查malloc返回值 */
void *buffer = malloc(size);
if (buffer == NULL) {
    LOG_ERROR("MODULE", "Memory allocation failed");
    return OS_ERROR;
}
/* 使用buffer */
free(buffer);
```

---

## 7. 日志规范

### 7.1 日志接口
**禁止**直接使用 `printf`/`fprintf`，**必须**使用 OSAL 日志接口：

```c
/* 简单输出 */
OS_printf("message\n");

/* 带模块名的日志 */
LOG_DEBUG("MODULE", "format", ...);
LOG_INFO("MODULE", "format", ...);
LOG_WARN("MODULE", "format", ...);
LOG_ERROR("MODULE", "format", ...);
LOG_FATAL("MODULE", "format", ...);
```

### 7.2 日志级别
| 级别 | 用途 | 示例 |
|------|------|------|
| DEBUG | 调试信息 | 函数入口/出口、变量值 |
| INFO | 正常信息 | 初始化成功、配置加载 |
| WARN | 警告信息 | 配置重复、非致命错误 |
| ERROR | 错误信息 | 操作失败、参数无效 |
| FATAL | 致命错误 | 系统崩溃、无法恢复 |

### 7.3 日志示例
```c
/* 初始化 */
LOG_INFO("XCONFIG", "Hardware configuration library initialized");

/* 错误 */
LOG_ERROR("XCONFIG", "Failed to register config[%d]: %s/%s/%s",
          i, config->platform, config->product, config->version);

/* 警告 */
LOG_WARN("XCONFIG", "Config already registered: %s/%s/%s",
         config->platform, config->product, config->version);
```

---

## 8. 任务编程

### 8.1 任务入口函数
**标准模板**：
```c
static void task_entry(void *arg)
{
    osal_id_t task_id = OS_TaskGetId();
    
    LOG_INFO("MODULE", "Task started");
    
    while (!OS_TaskShouldShutdown()) {
        /* 执行任务逻辑 */
        do_work();
        
        /* 延时 */
        OS_TaskDelay(100);
    }
    
    /* 清理资源 */
    cleanup();
    
    LOG_INFO("MODULE", "Task stopped");
}
```

### 8.2 优雅退出
**禁止**：
- 使用 `while(1)` 无限循环
- 使用 `pthread_cancel` 强制取消

**必须**：
- 检查 `OS_TaskShouldShutdown()`
- 退出前清理资源

**实际示例**：
```c
static void heartbeat_task(void *arg)
{
    satellite_service_context_t *ctx = (satellite_service_context_t *)arg;

    LOG_INFO("SAT", "Heartbeat task started");

    while (!OSAL_TaskShouldShutdown()) {
        /* 发送心跳 */
        if (satellite_can_send_heartbeat(ctx->can_handle, STATUS_OK) == OS_SUCCESS) {
            ctx->tx_count++;
        } else {
            ctx->error_count++;
        }

        /* 延迟 */
        OSAL_TaskDelay(ctx->config.heartbeat_interval_ms);
    }

    LOG_INFO("SAT", "Heartbeat task stopped");
}
```

---

## 9. 配置管理

### 9.1 配置文件位置
**配置下沉原则**：每个模块的配置文件放在模块内部的 `include/config/` 目录。

**示例**：
```
osal/include/config/task_config.h      # OSAL任务配置
hal/include/config/can_config.h        # HAL CAN配置
apps/can_gateway/include/config/app_config.h  # 应用配置
```

### 9.2 配置文件格式
```c
/************************************************************************
 * 模块配置
 ************************************************************************/

#ifndef MODULE_CONFIG_H
#define MODULE_CONFIG_H

/* 配置项 */
#define CONFIG_ITEM_1  100
#define CONFIG_ITEM_2  "value"

#endif /* MODULE_CONFIG_H */
```

### 9.3 依赖隔离
- 各模块**只依赖自己的配置**
- **禁止**引用其他模块的配置文件

---

## 10. 安全编程

### 10.1 禁止事项
- **命令注入**：禁止直接拼接用户输入到 `system()`
- **缓冲区溢出**：使用 `strncpy`/`snprintf` 而非 `strcpy`/`sprintf`
- **SQL 注入**：使用参数化查询
- **XSS**：Web 接口必须转义输出

### 10.2 字符串操作
```c
/* 正确 */
strncpy(dest, src, sizeof(dest) - 1);
dest[sizeof(dest) - 1] = '\0';

snprintf(buffer, sizeof(buffer), "format %s", str);

/* 错误 */
strcpy(dest, src);  // ❌ 可能溢出
sprintf(buffer, "format %s", str);  // ❌ 可能溢出
```

### 10.3 硬件操作
- **必须假设硬件会失败**：Flash 会坏、FPGA 会复位、链路会断开
- **必须包含看门狗和复位恢复逻辑**
- **寄存器操作必须引用芯片手册章节号**

---

## 11. 测试规范

### 11.1 测试框架
使用项目统一测试框架：
```c
#include "unittest_framework.h"

TEST_MODULE_BEGIN(test_module_name)

TEST_CASE(test_case_name)
{
    /* 准备 */
    int32 ret;
    
    /* 执行 */
    ret = function_under_test();
    
    /* 验证 */
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
}

TEST_MODULE_END()
```

### 11.2 测试覆盖
- **新功能必须编写单元测试**
- **Bug 修复必须添加回归测试**
- **关键路径必须覆盖边界条件**

---

## 12. 版本控制

### 12.1 提交信息格式
```
<type>: <subject>

<body>

<footer>
```

**Type**：
- `feat`: 新功能
- `fix`: Bug 修复
- `refactor`: 重构
- `docs`: 文档
- `test`: 测试
- `chore`: 构建/工具

**示例**：
```
feat: 添加XConfig硬件配置库

- 实现板级配置注册和查询
- 支持MCU/BMC/卫星平台外设配置
- 添加APP设备映射功能

Closes #123
```

### 12.2 分支管理
- `master`: 主分支（稳定版本）
- `develop`: 开发分支
- `feature/xxx`: 功能分支
- `bugfix/xxx`: Bug 修复分支

---

## 13. 性能优化

### 13.1 关键路径优化
- **零拷贝传输**：避免不必要的 `memcpy`
- **预分配内存**：避免运行时 `malloc`
- **缓存友好**：数据结构按缓存行对齐

### 13.2 性能指标
- CAN 消息延迟：< 10ms
- 命令处理时间：< 100ms
- 内存占用：< 128MB
- CPU 占用（空闲）：< 5%

---

## 14. 文档规范

### 14.1 必需文档
- `README.md`：模块概述、使用方法
- `CLAUDE.md`：项目指导文档（供 AI 助手使用）
- `CODING_STANDARDS.md`：本文档

### 14.2 代码即文档
- **优先使用清晰的命名**而非注释
- **函数名应说明意图**：`XCONFIG_HW_FindMCU` 优于 `find_mcu`
- **避免缩写**：`config` 优于 `cfg`（除非是行业标准）

---

## 15. 检查清单

### 15.1 代码提交前检查
- [ ] 编译无警告（`-Wall -Wextra -Werror`）
- [ ] 所有测试通过
- [ ] 添加了必要的单元测试
- [ ] 更新了相关文档
- [ ] 代码符合本规范
- [ ] 无内存泄漏（Valgrind 检查）
- [ ] 日志使用 OSAL 接口（无 `printf`）
- [ ] 所有返回值已检查
- [ ] 参数已校验（对外 API）

### 15.2 Code Review 检查
- [ ] 是否遵循分层架构
- [ ] 是否有跨层直接调用
- [ ] 错误处理是否完整
- [ ] 是否有潜在的内存泄漏
- [ ] 是否有缓冲区溢出风险
- [ ] 任务是否能优雅退出
- [ ] 是否有不必要的复杂度

---

## 16. 参考资料

- [Linux Kernel Coding Style](https://www.kernel.org/doc/html/latest/process/coding-style.html)
- [MISRA C:2012 Guidelines](https://www.misra.org.uk/)
- [DO-178C](https://en.wikipedia.org/wiki/DO-178C) - 航空软件安全标准
- [IEC 61508](https://en.wikipedia.org/wiki/IEC_61508) - 功能安全标准

---

## 附录：常见错误示例

### A.1 跨层调用
```c
/* 错误：Apps层直接调用HAL层 */
HAL_CAN_Send(fd, &frame);  // ❌

/* 正确：通过PDL层 */
SatellitePDL_SendCommand(cmd);  // ✓
```

### A.2 未检查返回值
```c
/* 错误 */
malloc(size);  // ❌
XCONFIG_Register(config);  // ❌

/* 正确 */
void *ptr = malloc(size);
if (ptr == NULL) {
    return OS_ERROR;
}

int32 ret = XCONFIG_Register(config);
if (ret != OS_SUCCESS) {
    LOG_ERROR("MODULE", "Register failed");
    return OS_ERROR;
}
```

### A.3 不安全的字符串操作
```c
/* 错误 */
strcpy(dest, src);  // ❌
sprintf(buf, "%s", str);  // ❌

/* 正确 */
strncpy(dest, src, sizeof(dest) - 1);
dest[sizeof(dest) - 1] = '\0';

snprintf(buf, sizeof(buf), "%s", str);
```

### A.4 任务无法退出
```c
/* 错误 */
static void task_entry(void *arg)
{
    while (1) {  // ❌ 无法退出
        do_work();
        sleep(1);
    }
}

/* 正确 */
static void task_entry(void *arg)
{
    while (!OS_TaskShouldShutdown()) {  // ✓
        do_work();
        OS_TaskDelay(1000);
    }
}
```

---

**版本**：v1.0  
**日期**：2026-04-25  
**维护者**：PMC-BSP 开发团队
