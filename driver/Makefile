# Makefile for driver

# Target info
MODULE_NAME ?= pdm

# Module info
MODULE_VERSIONS := $(shell git describe --tags --always --dirty --long 2>/dev/null || echo unknown)
MODULE_BUILD_TIME := $(shell date +'%Y-%m-%d %H:%M:%S')

# Kernel info
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
KERNEL_VERSION := $(shell $(MAKE) --silent -C $(KERNELDIR) kernelversion)

# Install configuration
DESTDIR ?= $(CURDIR)/_install
HEADER_INSTALL_DIR := $(DESTDIR)/include/$(MODULE_NAME)
MODULE_INSTALL_DIR := $(DESTDIR)/lib/modules/$(KERNEL_VERSION)
SYMBOL_INSTALL_DIR := $(DESTDIR)/lib/modules/$(KERNEL_VERSION)/symvers/$(MODULE_NAME)

# Log configuration
DEBUG_OSA_LOG_ENABLE := $(if $(findstring 0,$(osa_log_enable)),0,1)
DEBUG_OSA_LOG_WITH_FILE_LINE := $(if $(findstring 0,$(osa_log_with_file_line)),0,1)
DEBUG_OSA_LOG_WITH_FUNCTION := $(if $(findstring 1,$(osa_log_with_function)),1,0)

# Module info macro definitions
PRIVATE_CFLAGS += -DMODULE_VERSIONS="\"${MODULE_VERSIONS}\"" \
				  -DMODULE_BUILD_TIME="\"${MODULE_BUILD_TIME}\""

# Log configuration macro definitions
PRIVATE_CFLAGS += -DDEBUG_OSA_LOG_ENABLE=$(DEBUG_OSA_LOG_ENABLE) \
    -DDEBUG_OSA_LOG_WITH_FILE_LINE=$(DEBUG_OSA_LOG_WITH_FILE_LINE) \
    -DDEBUG_OSA_LOG_WITH_FUNCTION=$(DEBUG_OSA_LOG_WITH_FUNCTION)

export PRIVATE_CFLAGS MODULE_NAME

all: modules

modules:
	$(MAKE) -j$(nproc) -C $(KERNELDIR) M=$(CURDIR) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURDIR) clean

install:
	@if [ ! -f $(CURDIR)/$(MODULE_NAME).ko ]; then \
		echo "Error: $(MODULE_NAME).ko not found in $(CURDIR)"; \
		exit 1; \
	fi
	@mkdir -p $(MODULE_INSTALL_DIR)
	@cp -a $(CURDIR)/$(MODULE_NAME).ko $(MODULE_INSTALL_DIR)/

uninstall:
	@rm -f $(MODULE_INSTALL_DIR)/$(MODULE_NAME).ko
	@if [ -z "$$(find $(DESTDIR) -type f)" ]; then \
		rm -rf $(DESTDIR); \
	fi
.PHONY: all modules clean install uninstall
