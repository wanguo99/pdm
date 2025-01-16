# test/Makefile

# 引入配置文件
include $(PWD)/config.mk

# 配置应用程序
APP_NAME := pdm_test

# 手动列出所有源文件
APP_SRC := src/pdm_test.c \
		src/pdm_test_dimmer.c \
		src/pdm_test_switch.c

# 头文件目录
APP_INCLUDE_DIR := include
LIBAPI_INCLUDE_DIR := $(PWD)/library/include/api

# 链接库路径和库
LDFLAGS := -L$(PWD)/library -lpdm

# CFLAGS 添加头文件目录
CFLAGS += -I$(APP_INCLUDE_DIR) \
		-I$(LIBAPI_INCLUDE_DIR)

# 编译目标
$(APP_NAME): $(APP_SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# 安装目标
install: $(APP_NAME)
	@echo "Installing application and headers..."
	$(INSTALL) -d $(DESTDIR)/usr/local/bin
	$(INSTALL) $(APP_NAME) $(DESTDIR)/usr/local/bin/

# 卸载目标
uninstall:
	@echo "Uninstalling application and headers..."
	rm -f $(DESTDIR)/usr/local/bin/$(APP_NAME)
	rm -rf $(DESTDIR)/usr/local/include/pdm_test

# 清理目标
clean:
	@echo "Cleaning build artifacts..."
	rm -f $(APP_NAME)
