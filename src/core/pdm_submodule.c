#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/list.h>

#include "pdm.h"
#include "pdm_submodule.h"
#include "pdm_template.h"

static LIST_HEAD(pdm_submodule_driver_list);

static struct pdm_subdriver sub_drivers[] = {
    { .name = "Template Master", .init = pdm_template_master_init, .exit = pdm_template_master_exit },
    // 可以按照需要添加更多的驱动
    { .name = "Template Spi Driver", .init = pdm_template_i2c_driver_init, .exit = pdm_template_i2c_driver_exit },
    { }
};

static int pdm_submodule_register_driver(struct pdm_subdriver *driver) {
    int ret = 0;
    if (driver->init) {
        ret = driver->init();
        if (ret) {
            OSA_ERROR("Failed to register driver %s, ret = %d.\n", driver->name ? driver->name : "Unknown", ret);
            return ret;
        }
    }
    list_add_tail(&driver->list, &pdm_submodule_driver_list);
    return 0;
}

static void pdm_submodule_unregister_driver(struct pdm_subdriver *driver) {
    if (driver->exit) {
        driver->exit();
    }
    list_del(&driver->list);
}

int pdm_submodule_register_drivers(void)
{
    int i, ret = 0;

    for (i = 0; sub_drivers[i].name; i++) {
        ret = pdm_submodule_register_driver(&sub_drivers[i]);
        if (ret) {
            OSA_ERROR("Failed to register driver %s at index %d\n", sub_drivers[i].name, i);
            pdm_submodule_unregister_drivers();
            return ret;
        }
    }
    return 0;
}

void pdm_submodule_unregister_drivers(void)
{
    struct pdm_subdriver *driver, *tmp;
    list_for_each_entry_safe_reverse(driver, tmp, &pdm_submodule_driver_list, list) {
        pdm_submodule_unregister_driver(driver);
    }
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wanguo");
MODULE_DESCRIPTION("PDM Submodule Driver.");
