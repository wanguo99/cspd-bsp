# BSP Buildroot 集成指南

本目录包含将 BSP 集成到 Buildroot 构建系统所需的配置文件。

## 文件说明

### 1. bsp.mk
Buildroot package makefile，定义了如何下载、配置、编译和安装 BSP。

**主要配置项：**
- `PMC_BSP_VERSION`: 版本号或 Git 提交哈希
- `PMC_BSP_SITE`: 源码仓库地址
- `PMC_BSP_SITE_METHOD`: 下载方式（git/wget/local）
- `PMC_BSP_DEPENDENCIES`: 依赖的其他 Buildroot packages
- `PMC_BSP_CONF_OPTS`: CMake 配置选项

### 2. Config.in
Buildroot menuconfig 配置文件，定义了 BSP 在配置菜单中的选项。

**可配置选项：**
- `BR2_PACKAGE_PMC_BSP`: 启用 BSP package
- `BR2_PACKAGE_PMC_BSP_SAMPLE_APP`: 安装示例应用程序
- `BR2_PACKAGE_PMC_BSP_TESTS`: 安装单元测试

**依赖要求：**
- C++ 工具链支持
- 线程支持
- 动态库支持（不支持纯静态链接）

### 3. bsp.conf
运行时配置文件，将被安装到目标系统的 `/etc/bsp.conf`。

**配置项：**
- 日志级别和输出方式
- CAN 总线参数
- 串口参数
- I2C/SPI 总线配置
- 看门狗配置

### 4. S90bsp
SysV init 启动脚本，将被安装到 `/etc/init.d/S90bsp`。

**功能：**
- 系统启动时自动启动 BSP 示例应用
- 支持 start/stop/restart 命令
- 使用 start-stop-daemon 管理进程

## 集成步骤

### 方法一：外部 Package（推荐用于开发）

1. **将配置文件复制到 Buildroot 外部 package 目录：**
   ```bash
   mkdir -p /path/to/buildroot-external/package/bsp
   cp bsp.mk /path/to/buildroot-external/package/bsp/
   cp Config.in /path/to/buildroot-external/package/bsp/
   ```

2. **在外部 package 的 Config.in 中添加：**
   ```
   source "$BR2_EXTERNAL_<NAME>_PATH/package/bsp/Config.in"
   ```

3. **配置 Buildroot：**
   ```bash
   cd /path/to/buildroot
   make BR2_EXTERNAL=/path/to/buildroot-external menuconfig
   ```

4. **在 menuconfig 中启用 BSP：**
   ```
   Target packages
     └─> Miscellaneous
         └─> [*] bsp
             [*]   install sample application
             [ ]   install unit tests
   ```

5. **编译：**
   ```bash
   make
   ```

### 方法二：集成到 Buildroot 主树

1. **复制文件到 Buildroot package 目录：**
   ```bash
   mkdir -p /path/to/buildroot/package/bsp
   cp bsp.mk /path/to/buildroot/package/bsp/
   cp Config.in /path/to/buildroot/package/bsp/
   ```

2. **编辑 `package/Config.in`，添加：**
   ```
   menu "Miscellaneous"
       source "package/bsp/Config.in"
       ...
   endmenu
   ```

3. **后续步骤同方法一的步骤 3-5**

### 方法三：使用本地源码（开发调试）

如果需要使用本地源码而不是从 Git 仓库下载：

1. **修改 `bsp.mk` 中的源码位置：**
   ```makefile
   PMC_BSP_SITE = /path/to/bsp
   PMC_BSP_SITE_METHOD = local
   PMC_BSP_VERSION = local
   ```

2. **或者在 Buildroot 配置中设置 override：**
   ```bash
   echo 'PMC_BSP_OVERRIDE_SRCDIR = /path/to/bsp' >> local.mk
   ```

## 目标系统文件布局

编译完成后，BSP 将在目标系统中安装以下文件：

```
/usr/lib/
├── libosal.so          # OSAL 库
├── libhal.so           # HAL 库
├── libpcl.so           # PCL 库
└── libpdl.so           # PDL 库

/usr/include/bsp/   # 头文件（如果启用开发文件）
├── osal/
├── hal/
├── pcl/
└── pdl/

/usr/bin/
├── pmc-sample-app      # 示例应用（可选）
└── pmc-unit-test       # 单元测试（可选）

/etc/
├── bsp.conf        # 配置文件
└── init.d/
    └── S90bsp      # 启动脚本
```

## 交叉编译配置

BSP 支持多种架构的交叉编译，Buildroot 会自动传递正确的工具链配置：

- **ARM32**: `arm-linux-gnueabihf-gcc`
- **ARM64**: `aarch64-linux-gnu-gcc`
- **RISC-V 64**: `riscv64-linux-gnu-gcc`
- **x86_64**: `x86_64-linux-gnu-gcc`

CMake 会自动检测目标架构并应用相应的编译优化选项。

## 依赖关系

BSP 在 Buildroot 中的依赖：

- **必需依赖：**
  - C++ 工具链（`BR2_INSTALL_LIBSTDCPP`）
  - 线程支持（`BR2_TOOLCHAIN_HAS_THREADS`）
  - 动态库支持（`!BR2_STATIC_LIBS`）

- **可选依赖：**
  - 无（BSP 是自包含的）

## 常见问题

### 1. 编译失败：找不到 pthread
**原因：** 工具链未启用线程支持  
**解决：** 在 menuconfig 中启用 `Toolchain -> Enable NPTL support`

### 2. 运行时错误：找不到共享库
**原因：** 动态库路径未配置  
**解决：** 确保 `/usr/lib` 在 `LD_LIBRARY_PATH` 中，或运行 `ldconfig`

### 3. 启动脚本不执行
**原因：** 脚本权限不正确  
**解决：** 检查 `S90bsp` 是否有执行权限（应为 755）

### 4. 交叉编译架构不匹配
**原因：** CMake 未正确检测目标架构  
**解决：** 检查 Buildroot 的 `BR2_ARCH` 配置，确保与目标平台一致

## 自定义配置

### 修改 CMake 配置选项

编辑 `bsp.mk` 中的 `PMC_BSP_CONF_OPTS`：

```makefile
PMC_BSP_CONF_OPTS = \
	-DCMAKE_BUILD_TYPE=Release \
	-DBUILD_SHARED_LIBS=ON \
	-DBUILD_TESTING=OFF \
	-DCUSTOM_OPTION=value
```

### 添加额外的安装文件

在 `bsp.mk` 中添加 `define PMC_BSP_INSTALL_TARGET_CMDS`：

```makefile
define PMC_BSP_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/build/apps/sample_app $(TARGET_DIR)/usr/bin/pmc-sample-app
	$(INSTALL) -D -m 0644 $(PMC_BSP_PKGDIR)/bsp.conf $(TARGET_DIR)/etc/bsp.conf
	$(INSTALL) -D -m 0755 $(PMC_BSP_PKGDIR)/S90bsp $(TARGET_DIR)/etc/init.d/S90bsp
	# 添加自定义安装命令
	$(INSTALL) -D -m 0644 $(@D)/custom-file $(TARGET_DIR)/etc/custom-file
endef
```

## 版本管理

### 使用特定版本

修改 `bsp.mk`：

```makefile
PMC_BSP_VERSION = v1.0.0
PMC_BSP_SITE = https://github.com/your-org/bsp.git
PMC_BSP_SITE_METHOD = git
```

### 使用特定 Git 提交

```makefile
PMC_BSP_VERSION = abc123def456
```

### 使用 tarball

```makefile
PMC_BSP_VERSION = 1.0.0
PMC_BSP_SITE = https://github.com/your-org/bsp/releases/download/v$(PMC_BSP_VERSION)
PMC_BSP_SOURCE = bsp-$(PMC_BSP_VERSION).tar.gz
PMC_BSP_SITE_METHOD = wget
```

## 技术支持

如有问题，请参考：
- BSP 主文档：`../README.md`
- Buildroot 官方文档：https://buildroot.org/downloads/manual/manual.html
- 交叉编译指南：`../CROSS_COMPILE_GUIDE.md`
