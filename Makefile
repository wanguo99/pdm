# Makefile (Root Directory)

# 引入配置文件
include $(realpath config.mk)

# 确保子目录路径正确
MODULES_PATH := $(PWD)/modules
LIBS_PATH := $(PWD)/libs
TEST_PATH := $(PWD)/test

# 默认目标：构建所有模块
all:  modules libs test

# 单独构建 driver 模块
modules:
	$(MAKE) -C $(MODULES_PATH)

# 单独构建 libs 模块
libs:
	$(MAKE) -C $(LIBS_PATH)

# 单独构建 test 模块
test:
	$(MAKE) -C $(TEST_PATH)

# 清理目标（所有模块）
clean:
	$(MAKE) -C $(TEST_PATH) clean
	$(MAKE) -C $(LIBS_PATH) clean
	$(MAKE) -C $(MODULES_PATH) clean

# 单独清理 test 模块
clean-test:
	$(MAKE) -C $(TEST_PATH) clean

# 单独清理 libs 模块
clean-libs:
	$(MAKE) -C $(LIBS_PATH) clean

# 单独清理 driver 模块
clean-modules:
	$(MAKE) -C $(MODULES_PATH) clean

# 安装目标（所有模块）
install:
	$(MAKE) -C $(TEST_PATH) install DESTDIR=$(DESTDIR)
	$(MAKE) -C $(LIBS_PATH) install DESTDIR=$(DESTDIR)
	$(MAKE) -C $(MODULES_PATH) install DESTDIR=$(DESTDIR)

# 单独安装 test 模块
install-test:
	$(MAKE) -C $(TEST_PATH) install DESTDIR=$(DESTDIR)

# 单独安装 libs 模块
install-libs:
	$(MAKE) -C $(LIBS_PATH) install DESTDIR=$(DESTDIR)

# 单独安装 driver 模块
install-modules:
	$(MAKE) -C $(MODULES_PATH) install DESTDIR=$(DESTDIR)

# 卸载目标（所有模块）
uninstall:
	$(MAKE) -C $(TEST_PATH) uninstall DESTDIR=$(DESTDIR)
	$(MAKE) -C $(LIBS_PATH) uninstall DESTDIR=$(DESTDIR)
	$(MAKE) -C $(MODULES_PATH) uninstall DESTDIR=$(DESTDIR)
	@rm -rf $(DESTDIR)

# 单独卸载 test 模块
uninstall-test:
	$(MAKE) -C $(TEST_PATH) uninstall DESTDIR=$(DESTDIR)

# 单独卸载 libs 模块
uninstall-libs:
	$(MAKE) -C $(LIBS_PATH) uninstall DESTDIR=$(DESTDIR)

# 单独卸载 driver 模块
uninstall-modules:
	$(MAKE) -C $(MODULES_PATH) uninstall DESTDIR=$(DESTDIR)

# 显示版本、构建时间和内核信息
info:
	@echo "======================================================"
	@echo "                   Module Information                "
	@echo "======================================================"
	@printf "Module Version:    %-40s\n" "$(MODULE_VERSIONS)"
	@printf "Module Build Time: %-40s\n" "$(MODULE_BUILD_TIME)"
	@printf "Kernel Version:    %-40s\n" "$(KERNEL_VERSION)"
	@printf "Cross Compiler:    %-40s\n" "$(CROSS_COMPILE)"
	@printf "Installation Path: %-40s\n" "$(DESTDIR)"
	@echo "======================================================"

.PHONY: all test libs modules clean clean-test clean-libs clean-modules install install-test install-libs install-modules uninstall uninstall-test uninstall-libs uninstall-modules info

