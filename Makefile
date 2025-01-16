# Makefile (Root Directory)

# 引入配置文件
include $(realpath config.mk)

# 确保子目录路径正确
DRIVERS_PATH := $(PWD)/drivers
LIBRARY_PATH := $(PWD)/library
TEST_PATH := $(PWD)/test

# 默认目标：构建所有模块
all:  drivers library test

# 单独构建 driver 模块
drivers:
	$(MAKE) -C $(DRIVERS_PATH)

# 单独构建 library 模块
library:
	$(MAKE) -C $(LIBRARY_PATH)

# 单独构建 test 模块
test:
	$(MAKE) -C $(TEST_PATH)

# 清理目标（所有模块）
clean:
	$(MAKE) -C $(TEST_PATH) clean
	$(MAKE) -C $(LIBRARY_PATH) clean
	$(MAKE) -C $(DRIVERS_PATH) clean

# 单独清理 test 模块
clean-test:
	$(MAKE) -C $(TEST_PATH) clean

# 单独清理 library 模块
clean-library:
	$(MAKE) -C $(LIBRARY_PATH) clean

# 单独清理 driver 模块
clean-drivers:
	$(MAKE) -C $(DRIVERS_PATH) clean

# 安装目标（所有模块）
install:
	$(MAKE) -C $(TEST_PATH) install DESTDIR=$(DESTDIR)
	$(MAKE) -C $(LIBRARY_PATH) install DESTDIR=$(DESTDIR)
	$(MAKE) -C $(DRIVERS_PATH) install DESTDIR=$(DESTDIR)

# 单独安装 test 模块
install-test:
	$(MAKE) -C $(TEST_PATH) install DESTDIR=$(DESTDIR)

# 单独安装 library 模块
install-library:
	$(MAKE) -C $(LIBRARY_PATH) install DESTDIR=$(DESTDIR)

# 单独安装 driver 模块
install-drivers:
	$(MAKE) -C $(DRIVERS_PATH) install DESTDIR=$(DESTDIR)

# 卸载目标（所有模块）
uninstall:
	$(MAKE) -C $(TEST_PATH) uninstall DESTDIR=$(DESTDIR)
	$(MAKE) -C $(LIBRARY_PATH) uninstall DESTDIR=$(DESTDIR)
	$(MAKE) -C $(DRIVERS_PATH) uninstall DESTDIR=$(DESTDIR)
	@rm -rf $(DESTDIR)

# 单独卸载 test 模块
uninstall-test:
	$(MAKE) -C $(TEST_PATH) uninstall DESTDIR=$(DESTDIR)

# 单独卸载 library 模块
uninstall-library:
	$(MAKE) -C $(LIBRARY_PATH) uninstall DESTDIR=$(DESTDIR)

# 单独卸载 driver 模块
uninstall-drivers:
	$(MAKE) -C $(DRIVERS_PATH) uninstall DESTDIR=$(DESTDIR)

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

.PHONY: all test library drivers clean clean-test clean-library clean-drivers install install-test install-library install-drivers uninstall uninstall-test uninstall-library uninstall-drivers info

