# 指定要编译的目标模块
MODULE_NAME := pdm
obj-m := $(MODULE_NAME).o

# 定义要编译的目标模块对象文件
$(MODULE_NAME)-objs := 	src/core/pdm_submodule.o \
						src/core/pdm_core.o \
						src/core/pdm_master.o \
						src/core/pdm_device.o


 $(MODULE_NAME)-objs += 	src/template/pdm_template_master.o \
						src/template/pdm_template_i2c_driver.o


# $(MODULE_NAME)-objs += 	src/lcd/pdm_lcd_master.o \
						src/lcd/pdm_lcd_i2c_driver.o

# $(MODULE_NAME)-objs += 	src/led/pdm_led_master.o \
						src/led/pdm_led_gpio_driver.o

# 添加头文件路径
ccflags-y += 	-I$(src)/include

# 指定额外的头文件和符号表文件
# OSA_DIR := $(src)/../osa
# KBUILD_EXTRA_SYMBOLS := $(OSA_DIR)/Module.symvers
# ccflags-y += 	-I$(OSA_DIR)/include
