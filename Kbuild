# Specify the target module to be compiled
MODULE_NAME := pdm
obj-m := $(MODULE_NAME).o

# Include header file paths
ccflags-y += 	-I$(src)/include \
				-I$(src)/include/osa \
				-I$(src)/include/core \
				-I$(src)/include/private \
				-I$(src)/include/uapi

# Define object files for the core components of the target module
$(MODULE_NAME)-objs := 	src/core/pdm_core.o \
						src/core/pdm_component.o \
						src/core/pdm_bus.o \
						src/core/pdm_device.o \
						src/core/pdm_adapter.o \
						src/core/pdm_client.o

# Add device driver object files
$(MODULE_NAME)-objs += 	src/device/pdm_device_i2c.o \
						src/device/pdm_device_platform.o \
						src/device/pdm_device_spi.o

# Add LED driver object files
$(MODULE_NAME)-objs +=	src/led/pdm_led.o \
						src/led/pdm_led_gpio.o

# Optionally specify extra header and symbol table files
# Uncomment the following lines if OSA is used as an external module
# OSA_DIR ?= $(src)/../osa
# KBUILD_EXTRA_SYMBOLS := $(OSA_DIR)/Module.symvers