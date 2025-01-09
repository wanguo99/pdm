obj-m := $(MODULE_NAME).o

CFLAGS_MODULE += ${PRIVATE_CFLAGS}

CFLAGS_MODULE += -I$(src)/include \
	-I$(src)/include/osa \
	-I$(src)/include/core \
	-I$(src)/include/private \
	-I$(src)/include/uapi

$(MODULE_NAME)-objs := src/core/pdm_core.o \
	src/core/pdm_component.o \
	src/core/pdm_bus.o \
	src/core/pdm_device.o \
	src/core/pdm_adapter.o \
	src/core/pdm_client.o

$(MODULE_NAME)-objs += src/device/pdm_device_i2c.o \
	src/device/pdm_device_platform.o \
	src/device/pdm_device_spi.o

$(MODULE_NAME)-objs += src/switch/pdm_switch.o \
	src/switch/pdm_switch_gpio.o

$(MODULE_NAME)-objs += src/dimmer/pdm_dimmer.o \
	src/dimmer/pdm_dimmer_pwm.o

$(MODULE_NAME)-objs += src/nvmem/pdm_nvmem.o \
	src/nvmem/pdm_nvmem_spi.o
