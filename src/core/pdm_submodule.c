#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/list.h>

#include "pdm.h"
#include "pdm_submodule.h"

#include "pdm_cpld.h"
#include "pdm_lcd.h"
#include "pdm_led.h"

struct list_head pdm_submodule_driver_list;           // 保存已经注册的驱动信息

#define DEBUG_SUB_DRIVER_SWITCH 0	// 是否开启子模块初始化

static struct pdm_subdriver sub_drivers[] = {
#if DEBUG_SUB_DRIVER_SWITCH
	/* CPLD master and device driver */
    { .name = "cpld-master", .init = pdm_cpld_master_init, .exit = pdm_cpld_master_exit },
    { .name = "cpld-spi-device", .init = pdm_cpld_spi_driver_init, .exit = pdm_cpld_spi_driver_exit },

    /* LCD master and device driver */
    { .name = "lcd-master", .init = pdm_lcd_master_init, .exit = pdm_lcd_master_exit },
    { .name = "lcd-i2c-device", .init = pdm_lcd_i2c_driver_init, .exit = pdm_lcd_i2c_driver_exit },

    /* LED master and device driver */
    { .name = "led-master", .init = pdm_led_master_init, .exit = pdm_led_master_exit },
    { .name = "led-i2c-device", .init = pdm_led_gpio_driver_init, .exit = pdm_led_gpio_driver_exit },
#endif
    {}
};

// 注册单个子模块驱动
static int pdm_submodule_register_driver(struct pdm_subdriver *driver) {
    int ret;

    if (driver->init) {
        ret = driver->init();
        if (ret) {
            printk(KERN_INFO "%s: Driver register failed. ret = %d.",
                        driver->name ? driver->name : "Unknown", ret);
            return ret;
        }
    }

    list_add_tail(&driver->list, &pdm_submodule_driver_list);
    printk(KERN_INFO "%s: Driver registered.", driver->name ? driver->name : "Unknown");
    return 0;
}

// 注销单个子模块驱动
static int pdm_submodule_unregister_driver(struct pdm_subdriver *driver) {

    if (driver->exit) {
        driver->exit();
    }

    list_del(&driver->list);
    printk(KERN_INFO "%s: Driver unregistered.", driver->name ? driver->name : "Unknown");
    return 0;
}


// 初始化所有子模块驱动
int pdm_submodule_register_drivers(void)
{
    int i, ret;

    INIT_LIST_HEAD(&pdm_submodule_driver_list);

    for (i = 0; sub_drivers[i].init; i++) {
        ret = pdm_submodule_register_driver(&sub_drivers[i]);
        if (ret) {
            pr_err("Failed to register driver %d\n", i);
            pdm_submodule_unregister_drivers();
            return ret;
        }
    }

    return 0;
}


// 注销所有子模块驱动
void pdm_submodule_unregister_drivers(void)
{
    struct pdm_subdriver *driver, *tmp;

    list_for_each_entry_safe_reverse(driver, tmp, &pdm_submodule_driver_list, list) {
        pdm_submodule_unregister_driver(driver);
    }
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wanguo");
MODULE_DESCRIPTION("PDM Submodule Driver Framework.");
