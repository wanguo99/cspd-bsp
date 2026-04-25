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

### 3.1 文件命名

#### 源代码文件命名

| 层级 | 格式 | 示例 |
|------|------|------|
| OSAL | `osal_<submodule>.c/h` | `osal_task.c`, `osal_queue.h` |
| HAL | `hal_<device>.c/h` | `hal_can.c`, `hal_serial.h` |
| XConfig | `xconfig_<module>.c/h` | `xconfig_api.c`, `xconfig_register.h` |
| PDL | `pdl_<peripheral>.c/h` | `pdl_satellite.c`, `pdl_bmc.h` |
| Apps | `apps_<application>.c/h` | `apps_can_gateway.c`, `apps_protocol_converter.h` |

#### 测试文件命名

| 格式 | 示例 | 说明 |
|------|------|------|
| `test_<module>_<submodule>.c` | `test_osal_task.c`, `test_hal_can.c` | 测试文件以 `test_` 开头 |

**测试模块声明**：
```c
TEST_MODULE_BEGIN(test_<module>_<submodule>)
    /* 测试用例 */
TEST_MODULE_END(test_<module>_<submodule>)
```

### 3.2 分层命名规范

#### 对外API命名（Public API）

| 层级 | 格式 | 示例 | 说明 |
|------|------|------|------|
| OSAL | `OSAL_<Module><Function>()` | `OSAL_TaskCreate()`, `OSAL_QueuePut()` | 大写前缀+大驼峰 |
| HAL | `HAL_<Device><Function>()` | `HAL_CANInit()`, `HAL_SerialOpen()` | 大写前缀+大驼峰 |
| XConfig | `XCONFIG_<Module><Function>()` | `XCONFIG_Init()`, `XCONFIG_Register()` | 大写前缀+大驼峰 |
| PDL | `PDL_<Peripheral><Function>()` | `PDL_SatelliteInit()`, `PDL_BMCPowerOn()` | 大写前缀+大驼峰 |
| Apps | `<Application><Function>()` | `CanGatewayInit()`, `ProtocolConverterInit()` | 大驼峰 |

#### 内部函数命名（Internal/Private）

| 层级 | 格式 | 示例 | 说明 |
|------|------|------|------|
| OSAL | `osal_<module>_<function>()` | `osal_task_find_by_id()` | 小写前缀+小写+下划线 |
| HAL | `hal_<device>_<function>()` | `hal_can_set_filter()` | 小写前缀+小写+下划线 |
| XConfig | `xconfig_<module>_<function>()` | `xconfig_find_config()` | 小写前缀+小写+下划线 |
| PDL | `pdl_<peripheral>_<function>()` | `pdl_satellite_parse_frame()` | 小写前缀+小写+下划线 |
| Apps | `<application>_<function>()` | `can_gateway_process_rx()` | 小写+下划线 |

**重要规则：**
- ✅ 内部函数使用 `static` 限制作用域
- ✅ 使用小写+下划线格式（符合C标准和Linux内核风格）
- ❌ **禁止**使用下划线开头（`_xxx` 或 `__xxx`），这些被C标准保留

#### 测试用例命名（重要）

**格式**：`test_<module>_<function>_<scenario>`

**规则**：
- `test_` 前缀（固定）
- `<module>` 小写（如 osal, hal, pdl, apps）
- `<function>` 小写+下划线（如 file_open_close, task_create, can_init）
- `<scenario>` 小写+下划线（如 success, null_handle, invalid_param）
- **全部使用小写+下划线，无大写字母**

**示例**：

```c
/* OSAL层测试 */
TEST_MODULE_BEGIN(test_osal_task)
    TEST_CASE(test_osal_task_create_success)
    TEST_CASE(test_osal_task_create_null_handle)
    TEST_CASE(test_osal_task_delete_success)
    TEST_CASE(test_osal_task_delay_success)
TEST_MODULE_END(test_osal_task)

/* HAL层测试 */
TEST_MODULE_BEGIN(test_hal_can)
    TEST_CASE(test_hal_can_init_success)
    TEST_CASE(test_hal_can_init_null_handle)
    TEST_CASE(test_hal_can_send_success)
    TEST_CASE(test_hal_can_recv_timeout)
TEST_MODULE_END(test_hal_can)

/* PDL层测试 */
TEST_MODULE_BEGIN(test_pdl_satellite)
    TEST_CASE(test_pdl_satellite_init_success)
    TEST_CASE(test_pdl_satellite_init_null_config)
    TEST_CASE(test_pdl_satellite_send_command_success)
TEST_MODULE_END(test_pdl_satellite)

/* Apps层测试 */
TEST_MODULE_BEGIN(test_apps_can_gateway)
    TEST_CASE(test_apps_can_gateway_init_success)
    TEST_CASE(test_apps_can_gateway_process_message_success)
TEST_MODULE_END(test_apps_can_gateway)
```

**反例（禁止使用）**：

```c
/* ❌ 错误1：使用大写字母 */
TEST_CASE(test_HAL_CAN_Init_Success)        // ❌ 有大写字母
TEST_CASE(test_OSAL_FileOpen_Success)       // ❌ 有大写字母
TEST_CASE(TEST_HAL_CANInit_Success)         // ❌ 有大写字母

/* ❌ 错误2：使用驼峰命名 */
TEST_CASE(test_hal_canInit_success)         // ❌ 使用驼峰canInit
TEST_CASE(test_osal_taskCreate_success)     // ❌ 使用驼峰taskCreate

/* ❌ 错误3：缺少下划线分隔 */
TEST_CASE(test_halcaninit_success)          // ❌ 缺少下划线

/* ✅ 正确的命名 */
TEST_CASE(test_hal_can_init_success)         // ✓ 全部小写+下划线
TEST_CASE(test_osal_file_open_success)       // ✓ 全部小写+下划线
TEST_CASE(test_pdl_satellite_init_success)   // ✓ 全部小写+下划线
TEST_CASE(test_apps_can_gateway_init_success) // ✓ 全部小写+下划线
```

### 3.3 函数命名详解

#### 对外API函数（Public API）

**格式**：`<MODULE>_<Module><Function>()`

**示例**：
```c
/* OSAL层 - 对外API */
int32 OSAL_TaskCreate(osal_id_t *task_id, const char *name, ...);
int32 OSAL_TaskDelete(osal_id_t task_id);
int32 OSAL_QueueCreate(osal_id_t *queue_id, uint32 depth, ...);
int32 OSAL_QueuePut(osal_id_t queue_id, const void *data, uint32 size, int32 timeout);
int32 OSAL_MutexLock(osal_id_t mutex_id);

/* HAL层 - 对外API */
int32 HAL_CANInit(const char *interface);
int32 HAL_CANSend(int32 fd, const can_frame_t *frame);
int32 HAL_SerialOpen(const char *device, const hal_serial_config_t *config);

/* XConfig层 - 对外API */
int32 XCONFIG_Init(void);
int32 XCONFIG_Register(const xconfig_board_config_t *config);
const xconfig_mcu_cfg_t* XCONFIG_HWFindMCU(const char *name);

/* PDL层 - 对外API */
int32 PDL_SatelliteInit(const satellite_config_t *config);
int32 PDL_SatelliteSendCommand(uint8 cmd_type, const uint8 *data);
int32 PDL_BMCPowerOn(uint32 slot_id);
int32 PDL_BMCGetStatus(uint32 slot_id, uint8 *status);
```

#### 内部函数（Internal/Private）

**格式**：`<module>_<submodule>_<function>()`

**示例**：
```c
/* OSAL层 - 内部函数 */
static int32 osal_task_table_init(void);
static osal_id_t osal_task_find_free_slot(void);
static osal_task_record_t* osal_task_find_by_id(osal_id_t task_id);
static void osal_task_cleanup(osal_id_t task_id);

static int32 osal_queue_acquire(osal_queue_record_t *queue);
static void osal_queue_release(osal_queue_record_t *queue);

/* HAL层 - 内部函数 */
static int32 hal_can_set_filter(int fd, uint32 filter_id);
static void hal_can_update_stats(hal_can_handle_t *handle);
static int32 hal_serial_configure_termios(int fd, const hal_serial_config_t *config);

/* PDL层 - 内部函数 */
static int32 pdl_satellite_parse_frame(const can_frame_t *frame);
static void pdl_satellite_update_stats(void);
static int32 pdl_bmc_send_ipmi_command(uint8 cmd, const uint8 *data, uint32 len);
```

**命名规则：**
- 对外API：大写前缀 + 大驼峰（`OSAL_TaskCreate`）
- 内部函数：小写前缀 + 小写+下划线（`osal_task_find_by_id`）
- 内部函数必须声明为 `static`

### 3.4 变量命名

#### 全局变量
- **格式**：`g_<module>_<name>`
- **示例**：
```c
/* OSAL层全局变量 */
static osal_task_record_t g_task_table[OS_MAX_TASKS];
static pthread_mutex_t g_task_table_mutex;
static bool g_osal_initialized = false;

/* XConfig层全局变量 */
static xconfig_registry_t g_registry = {0};
static bool g_xconfig_initialized = false;
```

#### 静态变量
- **格式**：`s_<name>` 或直接使用 `static` + 描述性名称
- **示例**：
```c
/* 文件作用域静态变量 */
static uint32 s_init_count = 0;
static bool s_debug_enabled = false;

/* 或者直接使用描述性名称 */
static uint32 init_count = 0;
static bool debug_enabled = false;
```

#### 局部变量
- **格式**：小写+下划线
- **示例**：
```c
int32 ret;
uint32 success_count = 0;
osal_id_t task_id;
const xconfig_board_config_t *config = NULL;
osal_task_record_t *record = NULL;
```

#### 常量
- **格式**：全大写+下划线
- **示例**：
```c
#define MAX_BOARD_CONFIGS 32
#define OS_MAX_TASKS 64
#define CONFIG_COUNT (sizeof(g_all_configs) / sizeof(g_all_configs[0]))
#define DEFAULT_TIMEOUT_MS 1000
```

### 3.5 类型命名
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

### 3.6 宏命名
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

### 11.2 完整测试文件示例

```c
// 文件：test_osal_file.c

#include "unittest_framework.h"
#include "osal_file.h"

TEST_MODULE_BEGIN(test_osal_file)

TEST_CASE(test_osal_file_open_close_success)
{
    int32 fd;
    int32 ret;
    
    /* 打开文件 */
    ret = osal_file_open("/tmp/test.txt", O_RDWR | O_CREAT, &fd);
    TEST_ASSERT_EQUAL(0, ret);
    
    /* 关闭文件 */
    ret = osal_file_close(fd);
    TEST_ASSERT_EQUAL(0, ret);
}

TEST_CASE(test_osal_file_open_invalid_path)
{
    int32 fd;
    int32 ret;
    
    /* 打开无效路径 */
    ret = osal_file_open("/invalid/path/test.txt", O_RDWR, &fd);
    TEST_ASSERT_NOT_EQUAL(0, ret);
}

TEST_CASE(test_osal_file_read_write_success)
{
    int32 fd;
    int32 ret;
    char write_buf[] = "test data";
    char read_buf[32] = {0};
    
    /* 打开文件 */
    ret = osal_file_open("/tmp/test.txt", O_RDWR | O_CREAT, &fd);
    TEST_ASSERT_EQUAL(0, ret);
    
    /* 写入数据 */
    ret = osal_file_write(fd, write_buf, sizeof(write_buf));
    TEST_ASSERT_EQUAL(sizeof(write_buf), ret);
    
    /* 读取数据 */
    osal_file_lseek(fd, 0, SEEK_SET);
    ret = osal_file_read(fd, read_buf, sizeof(write_buf));
    TEST_ASSERT_EQUAL(sizeof(write_buf), ret);
    TEST_ASSERT_STRING_EQUAL(write_buf, read_buf);
    
    /* 关闭文件 */
    osal_file_close(fd);
}

TEST_MODULE_END(test_osal_file)
```

### 11.3 测试命名检查清单

#### 文件命名
- [ ] 文件名使用小写+下划线：`module_submodule.c`
- [ ] 测试文件以`test_`开头：`test_module_submodule.c`

#### 函数命名
- [ ] 函数名使用小写+下划线：`module_submodule_function()`
- [ ] 无大写字母，无驼峰命名

#### 测试用例命名
- [ ] 以`test_`开头（固定前缀）
- [ ] 模块名小写：`test_osal_`, `test_hal_`, `test_pdl_`, `test_apps_`
- [ ] 函数名小写+下划线：`file_open_close`, `task_create`, `can_init`
- [ ] 场景名小写+下划线：`success`, `null_handle`, `invalid_param`
- [ ] **全部小写，无大写字母，无驼峰命名**

### 11.4 测试覆盖
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

### A.1 内部函数命名错误

```c
/* 错误1：使用下划线开头（违反C标准） */
static int32 _osal_task_find_by_id(osal_id_t id);  // ❌ 保留标识符
static void __osal_queue_acquire(void);            // ❌ 保留标识符

/* 错误2：内部函数使用对外API格式 */
static int32 OSAL_TaskFindById(osal_id_t id);     // ❌ 应该用小写

/* 错误3：对外API使用内部函数格式 */
int32 osal_task_create(osal_id_t *task_id, ...);  // ❌ 应该用大写

/* 正确：内部函数使用小写+下划线 */
static int32 osal_task_find_by_id(osal_id_t id);  // ✓
static void osal_queue_acquire(void);              // ✓

/* 正确：对外API使用大写+大驼峰 */
int32 OSAL_TaskCreate(osal_id_t *task_id, ...);   // ✓
```

### A.2 跨层调用
```c
/* 错误：Apps层直接调用HAL层 */
HAL_CANSend(fd, &frame);  // ❌

/* 正确：通过PDL层 */
PDL_SatelliteSendCommand(cmd);  // ✓
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
