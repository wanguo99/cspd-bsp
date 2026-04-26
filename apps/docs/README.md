# Apps - 应用层

## 概述

Apps层是PMC-BSP的应用层，展示如何正确使用OSAL/HAL/PDL接口构建应用程序。

## 应用列表

### sample_app - 示例应用
最小化示例应用，展示OSAL基本用法：
- 任务创建和管理
- 消息队列通信
- 信号处理和优雅退出
- 日志系统使用

详细文档：[sample_app/README.md](../sample_app/README.md)

## 快速开始

```bash
# 编译
./build.sh

# 运行示例应用
./output/target/bin/sample_app
```

## 文档导航

- **[架构设计](ARCHITECTURE.md)** - 应用层设计原则
- **[使用指南](USAGE_GUIDE.md)** - 如何创建新应用

## 目录结构

```
apps/
├── sample_app/              # 示例应用
│   ├── README.md           # 应用说明
│   ├── CMakeLists.txt      # 构建配置
│   └── src/
│       └── main.c          # 主程序
├── docs/                   # 应用层文档
│   ├── README.md           # 本文件
│   ├── ARCHITECTURE.md     # 架构设计
│   └── USAGE_GUIDE.md      # 使用指南
└── CMakeLists.txt          # 总构建配置
```

## 设计原则

1. **平台无关**：应用层必须保持完全平台无关
2. **使用抽象接口**：通过OSAL/HAL/PDL访问底层
3. **优雅退出**：支持信号处理和资源清理
4. **错误处理**：检查所有返回值

## 依赖关系

```
应用层 (Apps) ← 你在这里
    ↓
外设驱动层 (PDL)
    ↓
硬件配置层 (XConfig)
    ↓
硬件抽象层 (HAL)
    ↓
操作系统抽象层 (OSAL)
```

## 相关文档

- [OSAL层文档](../../osal/docs/README.md)
- [HAL层文档](../../hal/docs/README.md)
- [PDL层文档](../../pdl/docs/README.md)
- [sample_app文档](../sample_app/README.md)
