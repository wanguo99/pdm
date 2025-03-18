# module info
MODULE_NAME := pdm
MODULE_VERSIONS := $(shell git describe --tags --always --dirty --long 2>/dev/null || echo unknown)
MODULE_BUILD_TIME := $(shell date +'%Y-%m-%d %H:%M:%S')

# kernel info
KDIR ?= /lib/modules/$(shell uname -r)/build
KERNEL_VERSION := $(shell $(MAKE) -C $(KDIR) kernelversion)

# install configuration
DESTDIR ?= $(CURDIR)/_install
HEADER_INSTALL_DIR := $(DESTDIR)/include/$(MODULE_NAME)
MODULE_INSTALL_DIR := $(DESTDIR)/lib/modules/$(KERNEL_VERSION)
SYMBOL_INSTALL_DIR := $(DESTDIR)/lib/modules/$(KERNEL_VERSION)/symvers/$(MODULE_NAME)

# module info micro defination
PRIVATE_CFLAGS += -DMODULE_VERSIONS="\"${MODULE_VERSIONS}\"" \
                  -DMODULE_BUILD_TIME="\"${MODULE_BUILD_TIME}\""

# log configuration micro defination
PRIVATE_CFLAGS += -DDEBUG_OSA_LOG_ENABLE=1 \
    -DDEBUG_OSA_LOG_WITH_FILE_LINE=1 \
    -DDEBUG_OSA_LOG_WITH_FUNCTION=0
