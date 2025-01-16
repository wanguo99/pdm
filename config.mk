# config.mk

# 安装路径
DESTDIR ?= _install

# 内核目录（交叉编译时可以设置为特定目录）
KDIR ?= /lib/modules/$(shell uname -r)/build

# 模块名称
MODULE_NAME := pdm

# 模块版本和构建时间
MODULE_VERSIONS := "$(shell git describe --tags --always --dirty --long 2>/dev/null || echo unknown)"
MODULE_BUILD_TIME := "$(shell date +'%Y-%m-%d %H:%M:%S')"

# 内核版本
KERNEL_VERSION := $(shell $(MAKE) -s -C $(KDIR) kernelversion)

# 交叉编译工具链前缀
CROSS_COMPILE ?= 

# 构建工具链
CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
LD := $(CROSS_COMPILE)ld
AR := $(CROSS_COMPILE)ar
AS := $(CROSS_COMPILE)as
RANLIB := $(CROSS_COMPILE)ranlib

# 编译选项（可以根据需要添加）
CFLAGS := -Wall -O2 -g

INSTALL ?= /usr/bin/install

