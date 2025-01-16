# library/Makefile

include $(PWD)/config.mk

# 配置库
LIBRARY_NAME := libpdm.so
LIBRARY_SRC := libpdm.c

# 默认目标
all: $(LIBRARY_NAME)

# 编译共享库
$(LIBRARY_NAME): $(LIBRARY_SRC)
	$(CC) -fPIC -shared $(CFLAGS) -o $@ $^

# 安装目标
install:
	$(INSTALL) -d $(DESTDIR)/lib
	$(INSTALL) $(LIBRARY_NAME) $(DESTDIR)/lib/

# 卸载目标
uninstall:
	rm -f $(DESTDIR)/lib/$(LIBRARY_NAME)

# 清理目标
clean:
	rm -f $(LIBRARY_NAME)

