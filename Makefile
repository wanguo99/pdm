# Kernel source directory path
KERNELDIR ?= /lib/modules/$(shell uname -r)/build

# Get the kernel version
KERNEL_VERSION := $(shell $(MAKE) -C $(KERNELDIR) kernelversion)

# Get the module name from Kbuild file if defined
MODULE_NAME ?= $(shell grep 'MODULE_NAME := ' $(CURDIR)/Kbuild | awk -F':=' '{print $$2}' | tr -d '[:space:]')

# Target installation directory
DESTDIR ?= $(CURDIR)/_install

# Header files installation directory
HEADER_INSTALL_DIR := $(DESTDIR)/include/$(MODULE_NAME)

# Kernel module (.ko) installation directory
MODULE_INSTALL_DIR := $(DESTDIR)/lib/modules/$(KERNEL_VERSION)

# Symbol table installation directory
SYMBOL_INSTALL_DIR := $(DESTDIR)/lib/modules/$(KERNEL_VERSION)/symvers/$(MODULE_NAME)

# Log printing options: default to print filename and line number; function name needs to be manually enabled
DEBUG_OSA_LOG_ENABLE ?= 1
DEBUG_OSA_LOG_WITH_FILE_LINE ?= 1
DEBUG_OSA_LOG_WITH_FUNCTION ?= 0

ifeq ($(strip $(osa_log_enable)),0)
DEBUG_OSA_LOG_ENABLE := 0
else ifeq ($(strip $(osa_log_enable)),1)
DEBUG_OSA_LOG_ENABLE := 1
endif

ifeq ($(strip $(osa_log_with_file_line)),0)
DEBUG_OSA_LOG_WITH_FILE_LINE := 0
else ifeq ($(strip $(osa_log_with_file_line)),1)
DEBUG_OSA_LOG_WITH_FILE_LINE := 1
endif

ifeq ($(strip $(osa_log_with_function)),0)
DEBUG_OSA_LOG_WITH_FUNCTION := 0
else ifeq ($(strip $(osa_log_with_function)),1)
DEBUG_OSA_LOG_WITH_FUNCTION := 1
endif

# Define debug-specific CFLAGS for OSA logging
DEBUG_OSA_LOG_CFLAGS = -DDEBUG_OSA_LOG_ENABLE=$(DEBUG_OSA_LOG_ENABLE) \
                       -DDEBUG_OSA_LOG_WITH_FILE_LINE=$(DEBUG_OSA_LOG_WITH_FILE_LINE) \
                       -DDEBUG_OSA_LOG_WITH_FUNCTION=$(DEBUG_OSA_LOG_WITH_FUNCTION)

# Default target
all: modules

# Compilation rules
modules:
	@echo "Building modules with DEBUG_OSA_LOG_CFLAGS=$(DEBUG_OSA_LOG_CFLAGS)"
	$(MAKE) -j$(nproc) -C $(KERNELDIR) M=$(CURDIR) CFLAGS_MODULE="$(DEBUG_OSA_LOG_CFLAGS)" modules

# Cleaning rules
clean:
	@echo "Cleaning up build artifacts"
	$(MAKE) -C $(KERNELDIR) M=$(CURDIR) clean

# Installation rules
install: modules
	@echo "Installing modules to $(DESTDIR)"
	@mkdir -p $(HEADER_INSTALL_DIR) $(MODULE_INSTALL_DIR) $(SYMBOL_INSTALL_DIR)
	@cp -a $(CURDIR)/include/* $(HEADER_INSTALL_DIR)
	@cp -a $(CURDIR)/$(MODULE_NAME).ko $(MODULE_INSTALL_DIR)
	@cp -a $(CURDIR)/Module.symvers $(SYMBOL_INSTALL_DIR)

# Uninstallation target
uninstall:
	@echo "Uninstalling modules from $(DESTDIR)"
	@rm -rf $(DESTDIR)

.PHONY: all modules clean install uninstall
