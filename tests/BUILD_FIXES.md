# 测试系统编译修复说明

## 已完成的修复

### 1. 重复定义问题
**问题**: 同时包含了原始文件和_fixed文件导致链接错误
**修复**: 在 `tests/CMakeLists.txt` 中排除_fixed文件
```cmake
file(GLOB OSAL_SOURCES ${CSPD_BSP_ROOT}/osal/linux/*.c)
list(FILTER OSAL_SOURCES EXCLUDE REGEX ".*_fixed\\.c$")

file(GLOB HAL_SOURCES ${CSPD_BSP_ROOT}/hal/linux/*.c)
list(FILTER HAL_SOURCES EXCLUDE REGEX ".*_fixed\\.c$")
```

### 2. 测试框架宏问题
**问题**: `TEST_START` 宏未定义，`TEST_END` 宏在return语句中使用do-while导致语法错误
**修复**: 
- 添加了 `TEST_START(name)` 宏
- 将 `TEST_END()` 改为GNU C表达式形式 `({ ... })`

文件: `tests/test_framework.h`

### 3. 协议转换器测试常量错误
**问题**: 使用了不存在的 `PAYLOAD_CHANNEL_PRIMARY` 和 `PAYLOAD_CHANNEL_BACKUP`
**修复**: 改为实际定义的常量
- `PAYLOAD_CHANNEL_PRIMARY` → `PAYLOAD_CHANNEL_ETHERNET`
- `PAYLOAD_CHANNEL_BACKUP` → `PAYLOAD_CHANNEL_UART`

文件: `tests/apps/test_protocol_converter.c`

### 4. CAN网关测试常量错误
**问题**: 使用了不存在的 `CAN_STATUS_OK`
**修复**: 
- 改为 `STATUS_OK`
- 添加 `#include "can_protocol.h"`

文件: `tests/apps/test_can_gateway.c`

### 5. 头文件路径缺失
**问题**: 找不到 `can_protocol.h`
**修复**: 在 `tests/CMakeLists.txt` 中添加config目录
```cmake
include_directories(
    ...
    ${CSPD_BSP_ROOT}/config
)
```

### 6. PayloadService链接错误
**问题**: test_payload_service找不到PayloadService_*函数
**修复**: 在test_payload_service中添加APPS_PROTOCOL_SOURCES
```cmake
add_executable(test_payload_service
    service/test_payload_service.c
    ${APPS_PROTOCOL_SOURCES}  # 添加这一行
    ${SERVICE_SOURCES}
    ${HAL_SOURCES}
    ${OSAL_SOURCES}
)
```

### 7. 未使用函数警告
**问题**: mock_server_thread定义但未使用
**修复**: 添加 `__attribute__((unused))` 属性

文件: `tests/service/test_payload_service.c`

## 编译步骤

在Linux环境下执行：

```bash
# 1. 进入项目根目录
cd /path/to/cspd-bsp

# 2. 创建并进入构建目录
mkdir -p build
cd build

# 3. 配置CMake（启用测试）
cmake -DBUILD_TESTING=ON ..

# 4. 编译所有测试
cmake --build . --target all_tests

# 5. 运行测试
ctest --output-on-failure
```

或者使用提供的脚本：
```bash
chmod +x build_tests.sh
./build_tests.sh
```

## 测试覆盖

修复后的测试系统包含：

- **OSAL层测试** (3个): test_os_task, test_os_queue, test_os_mutex
- **HAL层测试** (1个): test_hal_can
- **Service层测试** (1个): test_payload_service
- **Apps层测试** (2个): test_can_gateway, test_protocol_converter

总计: **7个测试可执行文件，65个测试用例**

## 已知问题

如果仍然遇到编译错误，请检查：

1. 确保在Linux环境下编译（需要pthread、rt库）
2. 确保安装了CMake 3.10+和GCC
3. 确保所有源文件路径正确
4. 检查是否有其他_fixed文件需要排除

## 文件修改清单

- `tests/test_framework.h` - 添加TEST_START宏，修复TEST_END宏
- `tests/CMakeLists.txt` - 排除_fixed文件，添加config路径，修复链接
- `tests/apps/test_protocol_converter.c` - 修正通道常量
- `tests/apps/test_can_gateway.c` - 修正状态常量，添加头文件
- `tests/service/test_payload_service.c` - 添加unused属性
- `build_tests.sh` - 新建测试构建脚本
