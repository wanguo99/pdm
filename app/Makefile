# app/Makefile

# 引入配置文件
include $(PWD)/config.mk

# 配置应用程序
APP_NAME := pdm_test
APP_SRC := pdm_test.c
LIBRARY_DIR := $(PWD)/library

# 编译目标
$(APP_NAME): $(APP_SRC)
	$(CC) $(CFLAGS) -o $@ $^ -L$(LIBRARY_DIR) -lpdm

# 安装目标
install:
	$(INSTALL) -d $(DESTDIR)/bin
	$(INSTALL) $(APP_NAME) $(DESTDIR)/bin/

# 卸载目标
uninstall:
	rm -f $(DESTDIR)/bin/$(APP_NAME)

# 清理目标
clean:
	rm -f $(APP_NAME)

