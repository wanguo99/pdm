# test/Makefile

# 引入配置文件
include $(PWD)/config.mk

# 配置应用程序
APP_NAME := pdm_test
APP_SRC := pdm_test.c

INSTALL_PATH := $(DESTDIR)/usr/local

LDFLAGS := -L$(PWD)/library -lpdm

# 编译目标
$(APP_NAME): $(APP_SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ 

# 安装目标
install:
	$(INSTALL) -d $(INSTALL_PATH)/bin
	$(INSTALL) $(APP_NAME) $(INSTALL_PATH)/bin/

# 卸载目标
uninstall:
	rm -f $(INSTALL_PATH)/bin/$(APP_NAME)

# 清理目标
clean:
	rm -f $(APP_NAME)

