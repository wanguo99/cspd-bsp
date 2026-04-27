# 测试卡死问题分析与解决方案

## 问题描述

运行 `./output/target/bin/unit-test -a` 时，测试会卡在 `test_pdl_bmc_init_network_success`，无法退出。

## 根本原因

### 1. 网络连接阻塞

**位置**: `pdl/src/pdl_bmc/pdl_bmc_redfish.c:77`

```c
if (OSAL_connect(ctx->sockfd, (osal_sockaddr_t *)&server_addr, sizeof(server_addr)) < 0)
{
    OSAL_close(ctx->sockfd);
    OSAL_Free(ctx);
    return OS_ERROR;
}
```

测试尝试连接到 `192.168.1.100:623`，这个 IP 地址不存在，导致：
- `connect()` 系统调用会阻塞等待超时（默认 TCP 超时约 75-120 秒）
- 虽然设置了 `SO_RCVTIMEO` 和 `SO_SNDTIMEO`，但这些选项**不影响 connect() 的超时**
- 每个 BMC 测试都会尝试连接，累积等待时间非常长

### 2. 为什么之前没发现

- 单独运行 HAL 层测试（`-L HAL`）不会触发 BMC 初始化
- 单独运行 PDL 层测试（`-L PDL`）会卡住，但可能被误认为是硬件依赖问题
- 运行所有测试（`-a`）时，在 PDL 测试阶段卡住

## 解决方案

### 方案 1：使用非阻塞 connect（推荐）

修改 `bmc_redfish_init()` 使用非阻塞连接：

```c
int32_t bmc_redfish_init(const char *ip_addr, uint16_t port, uint32_t timeout_ms, void **handle)
{
    // ... 前面的代码 ...

    /* 设置非阻塞模式 */
    int flags = OSAL_fcntl(ctx->sockfd, OSAL_F_GETFL, 0);
    OSAL_fcntl(ctx->sockfd, OSAL_F_SETFL, flags | OSAL_O_NONBLOCK);

    /* 尝试连接 */
    int ret = OSAL_connect(ctx->sockfd, (osal_sockaddr_t *)&server_addr, sizeof(server_addr));
    if (ret < 0 && OSAL_GetErrno() != OSAL_EINPROGRESS)
    {
        OSAL_close(ctx->sockfd);
        OSAL_Free(ctx);
        return OS_ERROR;
    }

    /* 使用 select 等待连接完成（带超时） */
    if (ret < 0)  /* EINPROGRESS */
    {
        osal_fd_set_t writefds;
        osal_timeval_t tv;
        
        OSAL_FD_ZERO(&writefds);
        OSAL_FD_SET(ctx->sockfd, &writefds);
        
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        
        ret = OSAL_select(ctx->sockfd + 1, NULL, &writefds, NULL, &tv);
        if (ret <= 0)  /* 超时或错误 */
        {
            OSAL_close(ctx->sockfd);
            OSAL_Free(ctx);
            return OS_ERROR;
        }
        
        /* 检查连接是否成功 */
        int error = 0;
        osal_size_t len = sizeof(error);
        OSAL_getsockopt(ctx->sockfd, OSAL_SOL_SOCKET, OSAL_SO_ERROR, &error, &len);
        if (error != 0)
        {
            OSAL_close(ctx->sockfd);
            OSAL_Free(ctx);
            return OS_ERROR;
        }
    }

    /* 恢复阻塞模式 */
    OSAL_fcntl(ctx->sockfd, OSAL_F_SETFL, flags);

    *handle = ctx;
    return OS_SUCCESS;
}
```

**优点**：
- 可以精确控制连接超时时间
- 不会长时间阻塞
- 生产代码也受益

**缺点**：
- 代码稍微复杂一些

### 方案 2：修改测试用例（临时方案）

跳过需要网络连接的测试：

```c
TEST_CASE(test_pdl_bmc_init_network_success)
{
    /* 跳过网络连接测试（需要真实的 BMC 设备） */
    TEST_SKIP("Requires real BMC device at 192.168.1.100");
    return;
    
    // ... 原有测试代码 ...
}
```

**优点**：
- 快速解决测试卡死问题
- 不修改生产代码

**缺点**：
- 无法测试网络连接功能
- 只是绕过问题，没有真正解决

### 方案 3：使用 Mock（最佳实践）

创建 Mock 版本的 `bmc_redfish_init()`：

```c
#ifdef UNIT_TEST
/* Mock 版本：不真正连接网络 */
int32_t bmc_redfish_init(const char *ip_addr, uint16_t port, uint32_t timeout_ms, void **handle)
{
    if (ip_addr == NULL || handle == NULL)
        return OS_ERROR;
    
    /* 创建假的上下文 */
    bmc_redfish_context_t *ctx = (bmc_redfish_context_t *)OSAL_Malloc(sizeof(bmc_redfish_context_t));
    if (NULL == ctx)
        return OS_ERROR;
    
    OSAL_Memset(ctx, 0, sizeof(bmc_redfish_context_t));
    ctx->sockfd = -1;  /* 标记为 Mock */
    ctx->timeout_ms = timeout_ms;
    
    *handle = ctx;
    return OS_SUCCESS;
}
#else
/* 真实版本：实际连接网络 */
// ... 原有代码 ...
#endif
```

**优点**：
- 测试快速运行
- 可以测试业务逻辑
- 不影响生产代码

**缺点**：
- 需要维护两套代码
- 无法测试真实的网络连接

## 推荐方案

**短期**：使用方案 2（跳过测试），快速解决卡死问题

**长期**：实施方案 1（非阻塞 connect），改进生产代码质量

## 立即修复

修改测试文件，跳过需要真实硬件的测试：

```bash
# 在测试用例开头添加跳过逻辑
TEST_CASE(test_pdl_bmc_init_network_success)
{
    TEST_SKIP("Requires real BMC device");
    return;
    // ... 原有代码 ...
}
```

## 验证

修复后运行测试：

```bash
./output/target/bin/unit-test -a
# 应该能够正常完成，不再卡死
```

## 相关问题

类似的问题可能存在于：
- `test_pdl_bmc_init_serial_success` - 串口连接
- 其他需要真实硬件的测试

建议统一处理所有硬件依赖测试。
