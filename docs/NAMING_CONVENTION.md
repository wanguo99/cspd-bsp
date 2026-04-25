# PMC-BSP 命名规范（最终版）

## 统一命名格式：全部小写+下划线

### 1. 源代码文件命名

#### OSAL层
- **文件**：`osal_<submodule>.c/h`
- **函数**：`osal_<submodule>_<function>()`
- 示例：
  - `osal_task.c` → `osal_task_create()`, `osal_task_delete()`
  - `osal_queue.c` → `osal_queue_create()`, `osal_queue_put()`
  - `osal_mutex.c` → `osal_mutex_lock()`, `osal_mutex_unlock()`

#### HAL层
- **文件**：`hal_<device>.c/h`
- **函数**：`hal_<device>_<function>()`
- 示例：
  - `hal_can.c` → `hal_can_init()`, `hal_can_send()`
  - `hal_serial.c` → `hal_serial_open()`, `hal_serial_write()`

#### XConfig层
- **文件**：`xconfig_<module>.c/h`
- **函数**：`xconfig_<module>_<function>()`
- 示例：
  - `xconfig_api.c` → `xconfig_init()`, `xconfig_register()`

#### PDL层
- **文件**：`pdl_<peripheral>.c/h`
- **函数**：`pdl_<peripheral>_<function>()`
- 示例：
  - `pdl_satellite.c` → `pdl_satellite_init()`, `pdl_satellite_send_command()`
  - `pdl_bmc.c` → `pdl_bmc_power_on()`, `pdl_bmc_get_status()`
  - `pdl_mcu.c` → `pdl_mcu_get_version()`, `pdl_mcu_reset()`

#### Apps层
- **文件**：`apps_<application>.c/h`
- **函数**：`<application>_<function>()`
- 示例：
  - `apps_can_gateway.c` → `can_gateway_init()`, `can_gateway_process_message()`
  - `apps_protocol_converter.c` → `protocol_converter_init()`

### 2. 测试文件命名（重要！）

#### 文件命名
- **格式**：`test_<module>_<submodule>.c`
- **模块声明**：`TEST_MODULE_BEGIN(test_<module>_<submodule>)`

#### 测试用例命名（关键规范）
- **格式**：`test_<module>_<function>_<scenario>`
- **规则**：
  - `test_` 前缀（固定）
  - `<module>` 小写（如 osal, hal, pdl, apps）
  - `<function>` 小写+下划线（如 file_open_close, task_create）
  - `<scenario>` 小写+下划线（如 success, null_handle, invalid_param）
  - **全部使用小写+下划线，无大写字母**

#### 示例

**OSAL层测试**
```c
// 文件：test_osal_task.c
TEST_MODULE_BEGIN(test_osal_task)
    TEST_CASE(test_osal_task_create_success)
    TEST_CASE(test_osal_task_create_null_handle)
    TEST_CASE(test_osal_task_delete_success)
    TEST_CASE(test_osal_task_delay_success)
TEST_MODULE_END(test_osal_task)
```

**HAL层测试**
```c
// 文件：test_hal_can.c
TEST_MODULE_BEGIN(test_hal_can)
    TEST_CASE(test_hal_can_init_success)
    TEST_CASE(test_hal_can_init_null_handle)
    TEST_CASE(test_hal_can_send_success)
    TEST_CASE(test_hal_can_recv_timeout)
TEST_MODULE_END(test_hal_can)
```

**PDL层测试**
```c
// 文件：test_pdl_satellite.c
TEST_MODULE_BEGIN(test_pdl_satellite)
    TEST_CASE(test_pdl_satellite_init_success)
    TEST_CASE(test_pdl_satellite_init_null_config)
    TEST_CASE(test_pdl_satellite_send_command_success)
TEST_MODULE_END(test_pdl_satellite)
```

**Apps层测试**
```c
// 文件：test_apps_can_gateway.c
TEST_MODULE_BEGIN(test_apps_can_gateway)
    TEST_CASE(test_apps_can_gateway_init_success)
    TEST_CASE(test_apps_can_gateway_process_message_success)
TEST_MODULE_END(test_apps_can_gateway)
```

### 3. 命名规范对比表

| 层级 | 文件名 | 函数名 | 测试用例名 |
|------|--------|--------|-----------|
| OSAL | `osal_task.c` | `osal_task_create()` | `test_osal_task_create_success` |
| HAL | `hal_can.c` | `hal_can_init()` | `test_hal_can_init_success` |
| PDL | `pdl_satellite.c` | `pdl_satellite_init()` | `test_pdl_satellite_init_success` |
| Apps | `apps_can_gateway.c` | `can_gateway_init()` | `test_apps_can_gateway_init_success` |

### 4. 反例（禁止使用）

❌ **错误的命名**
```c
// 错误1：使用大写字母
TEST_CASE(test_HAL_CAN_Init_Success)        // ❌ 有大写字母
TEST_CASE(test_OSAL_FileOpen_Success)       // ❌ 有大写字母
TEST_CASE(TEST_HAL_CANInit_Success)         // ❌ 有大写字母

// 错误2：使用驼峰命名
TEST_CASE(test_hal_canInit_success)         // ❌ 使用驼峰canInit
TEST_CASE(test_osal_taskCreate_success)     // ❌ 使用驼峰taskCreate

// 错误3：缺少下划线分隔
TEST_CASE(test_halcaninit_success)          // ❌ 缺少下划线
```

✅ **正确的命名**
```c
TEST_CASE(test_hal_can_init_success)         // ✓ 全部小写+下划线
TEST_CASE(test_osal_file_open_success)       // ✓ 全部小写+下划线
TEST_CASE(test_pdl_satellite_init_success)   // ✓ 全部小写+下划线
TEST_CASE(test_apps_can_gateway_init_success) // ✓ 全部小写+下划线
```

### 5. 命名规范检查清单

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

### 6. 命名规范的好处

1. **一致性**：所有命名遵循统一的小写+下划线模式
2. **可读性**：`test_hal_can_init_success` 清晰易读
3. **可维护性**：便于查找和定位
4. **避免混淆**：不会出现大小写混写的情况
5. **工具友好**：便于grep、find等工具处理
6. **符合C语言传统**：Linux内核风格

### 7. 实际示例对比

#### 修正前（错误）
```c
[RUN ] test_hal_can::test_HAL_CAN_Init_NullHandle     // ❌ 有大写字母
[RUN ] test_osal_file::test_OSAL_FileOpen_Success     // ❌ 有大写字母
[RUN ] test_pdl_satellite::TEST_PDL_SatelliteInit     // ❌ 有大写字母和驼峰
```

#### 修正后（正确）
```c
[RUN ] test_hal_can::test_hal_can_init_null_handle      // ✓ 全部小写+下划线
[RUN ] test_osal_file::test_osal_file_open_success      // ✓ 全部小写+下划线
[RUN ] test_pdl_satellite::test_pdl_satellite_init      // ✓ 全部小写+下划线
```

### 8. 完整示例

```c
// 文件：test_osal_file.c

#include "test_framework.h"
#include "osal_file.h"

TEST_MODULE_BEGIN(test_osal_file)

TEST_CASE(test_osal_file_open_close_success)
{
    int32_t fd;
    int32_t ret;
    
    // 打开文件
    ret = osal_file_open("/tmp/test.txt", O_RDWR | O_CREAT, &fd);
    TEST_ASSERT_EQUAL(0, ret);
    
    // 关闭文件
    ret = osal_file_close(fd);
    TEST_ASSERT_EQUAL(0, ret);
}

TEST_CASE(test_osal_file_open_invalid_path)
{
    int32_t fd;
    int32_t ret;
    
    // 打开无效路径
    ret = osal_file_open("/invalid/path/test.txt", O_RDWR, &fd);
    TEST_ASSERT_NOT_EQUAL(0, ret);
}

TEST_CASE(test_osal_file_read_write_success)
{
    int32_t fd;
    int32_t ret;
    char write_buf[] = "test data";
    char read_buf[32] = {0};
    
    // 打开文件
    ret = osal_file_open("/tmp/test.txt", O_RDWR | O_CREAT, &fd);
    TEST_ASSERT_EQUAL(0, ret);
    
    // 写入数据
    ret = osal_file_write(fd, write_buf, sizeof(write_buf));
    TEST_ASSERT_EQUAL(sizeof(write_buf), ret);
    
    // 读取数据
    osal_file_lseek(fd, 0, SEEK_SET);
    ret = osal_file_read(fd, read_buf, sizeof(write_buf));
    TEST_ASSERT_EQUAL(sizeof(write_buf), ret);
    TEST_ASSERT_STRING_EQUAL(write_buf, read_buf);
    
    // 关闭文件
    osal_file_close(fd);
}

TEST_MODULE_END(test_osal_file)
```

---

**版本**：v3.0（最终版 - 全小写）  
**日期**：2026-04-25  
**维护者**：PMC-BSP 开发团队
