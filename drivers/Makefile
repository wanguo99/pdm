# drivers/Makefile

# 引入配置文件
include $(realpath config.mk)
include ../config.mk

# 构建内核模块
$(MODULE_NAME):
	make -j $(nproc) -C $(KDIR) M=$(PWD) modules

# 安装内核模块，使用内核的 modules_install 目标
install: $(MODULE_NAME)
	$(MAKE) -C $(KDIR) M=$(PWD) INSTALL_MOD_PATH=$(INSTALL_MOD_PATH) modules_install

# 卸载内核模块
uninstall:
	rm -f $(INSTALL_MOD_PATH)/$(MODULE_NAME)

# 清理模块
clean:
	make -C $(KDIR) M=$(PWD) clean

# 打印版本和构建时间信息
info:
	@echo "Module Version: $(MODULE_VERSIONS)"
	@echo "Module Build Time: $(MODULE_BUILD_TIME)"
	@echo "Kernel Version: $(KERNEL_VERSION)"
	@echo "Kernel Directory: $(KDIR)"

.PHONY: clean install uninstall info

