################################################################################
#
# bsp
#
################################################################################

BSP_VERSION = 1.0.0
BSP_SITE = $(call github,wanguo99,bsp,$(BSP_VERSION))
# 如果使用本地源码，取消下面的注释
# BSP_SITE = /path/to/local/bsp
# BSP_SITE_METHOD = local

BSP_LICENSE = MIT
BSP_LICENSE_FILES = LICENSE
BSP_INSTALL_STAGING = YES

# 依赖项
BSP_DEPENDENCIES = host-pkgconf

# CMake 配置选项
BSP_CONF_OPTS = \
	-DCMAKE_BUILD_TYPE=Release \
	-DBUILD_TESTING=OFF \
	-DBUILD_SHARED_LIBS=ON

# 如果启用测试，则编译测试程序
ifeq ($(BR2_PACKAGE_BSP_TESTS),y)
BSP_CONF_OPTS += -DBUILD_TESTING=ON
endif

# 安装示例应用
ifeq ($(BR2_PACKAGE_BSP_SAMPLE_APP),y)
define BSP_INSTALL_SAMPLE_APP
	$(INSTALL) -D -m 0755 $(@D)/build/release/bin/bsp-sample-app \
		$(TARGET_DIR)/usr/bin/bsp-sample-app
endef
BSP_POST_INSTALL_TARGET_HOOKS += BSP_INSTALL_SAMPLE_APP
endif

# 安装测试程序
ifeq ($(BR2_PACKAGE_BSP_TESTS),y)
define BSP_INSTALL_TESTS
	$(INSTALL) -D -m 0755 $(@D)/build/release/bin/bsp-unit-test \
		$(TARGET_DIR)/usr/bin/bsp-unit-test
endef
BSP_POST_INSTALL_TARGET_HOOKS += BSP_INSTALL_TESTS
endif

# 安装配置文件
define BSP_INSTALL_CONFIG
	$(INSTALL) -D -m 0644 $(BSP_PKGDIR)/bsp.conf \
		$(TARGET_DIR)/etc/bsp.conf
endef
BSP_POST_INSTALL_TARGET_HOOKS += BSP_INSTALL_CONFIG

# 安装 init 脚本
define BSP_INSTALL_INIT_SYSV
	$(INSTALL) -D -m 0755 $(BSP_PKGDIR)/S90bsp \
		$(TARGET_DIR)/etc/init.d/S90bsp
endef

$(eval $(cmake-package))
