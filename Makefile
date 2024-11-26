# 内核源代码路径
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
# 获取内核版本
KERNEL_VERSION := $(shell $(MAKE) -C $(KERNELDIR) kernelversion | grep -v make)

# 获取模块名
MODULE_NAME ?= $(shell grep 'MODULE_NAME := ' $(CURDIR)/Kbuild | awk -F':=' '{print $$2}' | tr -d '[:space:]')

# 目标安装目录
DESTDIR ?= $(CURDIR)/_install
# 头文件
HEADER_INSTALL_DIR := $(DESTDIR)/include/$(MODULE_NAME)
# ko
MODULE_INSTALL_DIR := $(DESTDIR)/lib/modules/$(KERNEL_VERSION)
# 符号表
SYMBOL_INSTALL_DIR := $(DESTDIR)/lib/modules/$(KERNEL_VERSION)/symvers/$(MODULE_NAME)

# 日志打印开关, 默认只打印文件名和行号，函数名需要手动开启
DEBUG_OSA_LOG_ENABLE ?= 1
DEBUG_OSA_LOG_WITH_FILE_LINE ?= 1
DEBUG_OSA_LOG_WITH_FUNCTION ?= 0

# 检查 osa_log_enable 参数，启用或禁用日志打印
ifeq ($(strip $(osa_log_enable)),0)
DEBUG_OSA_LOG_ENABLE := 0
else ifeq ($(strip $(osa_log_enable)),1)
DEBUG_OSA_LOG_ENABLE := 1
endif

# 检查 osa_log_with_file_line 参数，启用或禁用文件名和行号打印
ifeq ($(strip $(osa_log_with_file_line)),0)
DEBUG_OSA_LOG_WITH_FILE_LINE := 0
else ifeq ($(strip $(osa_log_with_file_line)),1)
DEBUG_OSA_LOG_WITH_FILE_LINE := 1
endif

# 检查 osa_log_with_function 参数，启用或禁用函数名打印
ifeq ($(strip $(osa_log_with_function)),0)
DEBUG_OSA_LOG_WITH_FUNCTION := 0
else ifeq ($(strip $(osa_log_with_function)),1)
DEBUG_OSA_LOG_WITH_FUNCTION := 1
endif

# 默认目标
all: modules

# 编译规则
modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURDIR) KBUILD_EXTRA_SYMBOLS="$(EXTRA_SYMBOLS)" \
		CFLAGS_MODULE="-DDEBUG_OSA_LOG_ENABLE=$(DEBUG_OSA_LOG_ENABLE) -DDEBUG_OSA_LOG_WITH_FILE_LINE=$(DEBUG_OSA_LOG_WITH_FILE_LINE) -DDEBUG_OSA_LOG_WITH_FUNCTION=$(DEBUG_OSA_LOG_WITH_FUNCTION)" modules

# 清理规则
clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURDIR) clean

install: modules
	@echo "Installing modules to $(DESTDIR)"
	@mkdir -p $(HEADER_INSTALL_DIR) $(MODULE_INSTALL_DIR) $(SYMBOL_INSTALL_DIR)
	@cp -a $(CURDIR)/include/* $(HEADER_INSTALL_DIR)
	@cp -a $(CURDIR)/$(MODULE_NAME).ko $(MODULE_INSTALL_DIR)
	@cp -a $(CURDIR)/Module.symvers $(SYMBOL_INSTALL_DIR)

# 卸载目标
uninstall:
	@echo "Uninstalling modules from $(DESTDIR)"
	@rm -rf $(DESTDIR)

.PHONY: all modules clean install uninstall
