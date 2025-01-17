# library/Makefile

include $(PWD)/config.mk

# 配置库
LIBRARY_NAME := libpdm.so

# 手动列出所有源文件
LIBRARY_SRC := src/libpdm_dimmer.c \
			src/libpdm_switch.c

# 手动列出所有头文件目录
LIBRARY_API_HEADERS_DIR := include/api
LIBRARY_PRIVATE_HEADERS_DIR := include/private

# CFLAGS 添加所有头文件目录
CFLAGS += -I$(LIBRARY_API_HEADERS_DIR) \
		-I$(LIBRARY_PRIVATE_HEADERS_DIR)

# 编译共享库
$(LIBRARY_NAME): $(LIBRARY_SRC)
	$(CC) -fPIC -shared $(CFLAGS) -o $@ $^

# 安装目标
install: $(LIBRARY_NAME)
	@echo "Installing library and headers..."
	@$(INSTALL) -d $(DESTDIR)/usr/local/lib
	@$(INSTALL) -m 644 $(LIBRARY_NAME) $(DESTDIR)/usr/local/lib
	@$(INSTALL) -d $(DESTDIR)/usr/local/include
	@$(INSTALL) -m 644 -t $(DESTDIR)/usr/local/include $(LIBRARY_API_HEADERS_DIR)/libpdm.h

# 卸载目标
uninstall:
	@echo "Uninstalling library and headers..."
	@rm -f $(DESTDIR)/usr/local/lib/$(LIBRARY_NAME)
	@rm -rf $(DESTDIR)/usr/local/include/libpdm.h

# 清理目标
clean:
	@echo "Cleaning build artifacts..."
	@rm -f $(LIBRARY_NAME)
