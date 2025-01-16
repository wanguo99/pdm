# library/Makefile

include $(PWD)/config.mk

# 配置库
LIBRARY_NAME := libpdm.so
LIBRARY_SRC := libpdm.c

INSTALL_PATH := $(DESTDIR)/usr/local


# 编译共享库
$(LIBRARY_NAME): $(LIBRARY_SRC)
	$(CC) -fPIC -shared $(CFLAGS) -o $@ $^

# 安装目标
install:
	$(INSTALL) -d $(INSTALL_PATH)/lib
	$(INSTALL) $(LIBRARY_NAME) $(INSTALL_PATH)/lib/

# 卸载目标
uninstall:
	rm -f $(INSTALL_PATH)/lib/$(LIBRARY_NAME)

# 清理目标
clean:
	rm -f $(LIBRARY_NAME)

