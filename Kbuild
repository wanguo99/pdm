# Module name
obj-m := $(MODULE_NAME).o

# Source files directory
SRCDIR := src

# Include directories
INCDIR := ${src}/include

# Compiler flags for the module
CFLAGS_MODULE += ${PRIVATE_CFLAGS}
CFLAGS_MODULE += \
    -I$(INCDIR) \
    -I$(INCDIR)/osa \
    -I$(INCDIR)/core \
    -I$(INCDIR)/private \
    -I$(INCDIR)/uapi

# Source files (relative to SRCDIR)
SRC = \
    $(SRCDIR)/core/pdm_core.c \
    $(SRCDIR)/core/pdm_component.c \
    $(SRCDIR)/core/pdm_bus.c \
    $(SRCDIR)/core/pdm_device.c \
    $(SRCDIR)/core/pdm_adapter.c \
    $(SRCDIR)/core/pdm_client.c \
    $(SRCDIR)/device/pdm_device_i2c.c \
    $(SRCDIR)/device/pdm_device_platform.c \
    $(SRCDIR)/device/pdm_device_spi.c \
    $(SRCDIR)/switch/pdm_switch.c \
    $(SRCDIR)/switch/pdm_switch_gpio.c \
    $(SRCDIR)/dimmer/pdm_dimmer.c \
    $(SRCDIR)/dimmer/pdm_dimmer_pwm.c \
    $(SRCDIR)/nvmem/pdm_nvmem.c \
    $(SRCDIR)/nvmem/pdm_nvmem_spi.c \
    $(SRCDIR)/sensor/pdm_sensor.c \
    $(SRCDIR)/sensor/pdm_sensor_ap3216c.c \
    $(SRCDIR)/sensor/pdm_sensor_icm20608.c

# Add objects to the module's object list
$(MODULE_NAME)-objs := $(SRC:.c=.o)
