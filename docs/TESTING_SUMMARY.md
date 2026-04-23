# CSPD-BSP 测试系统快速指南

## 唯一测试入口

项目只有一个测试可执行文件：`unit-test`

## 快速开始

### 构建测试
```bash
./build.sh -d
```

### 运行测试（交互模式）
```bash
cd build
./bin/unit-test
```

### 运行所有测试（命令行）
```bash
cd build
./bin/unit-test -a
```

## 常用命令

```bash
./bin/unit-test -i              # 交互式菜单
./bin/unit-test -a              # 运行所有测试
./bin/unit-test -L OSAL         # 运行OSAL层测试
./bin/unit-test -m test_os_file # 运行指定模块
./bin/unit-test -l              # 列出所有测试
./bin/unit-test -h              # 查看帮助
```

## 测试层次

- **OSAL层**: 6个模块（task, queue, mutex, file, network, signal）
- **HAL层**: 1个模块（CAN driver）
- **Service层**: 1个模块（payload service）
- **Apps层**: 2个模块（CAN gateway, protocol converter）

详细文档请参考 [TESTING.md](TESTING.md)
