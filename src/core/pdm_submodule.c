#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/list.h>

#include "pdm.h"
#include "pdm_submodule.h"
#include "pdm_template.h"

struct list_head pdm_submodule_driver_list;

static struct pdm_subdriver sub_drivers[] = {

    /* TEMPLATE master and device driver */
    // { .name = "TEMPLATE-Master", .init = pdm_template_master_init, .exit = pdm_template_master_exit },
    // { .name = "TEMPLATE-Spi-Driver", .init = pdm_template_i2c_driver_init, .exit = pdm_template_i2c_driver_exit },

    {}
};

static int pdm_submodule_register_driver(struct pdm_subdriver *driver) {
    int iRet;
    if (driver->init) {
        iRet = driver->init();
        if (iRet) {
            printk(KERN_INFO "%s: Driver register failed. ret = %d.",
                        driver->name ? driver->name : "Unknown", iRet);
            return iRet;
        }
    }
    list_add_tail(&driver->list, &pdm_submodule_driver_list);
    return 0;
}

static int pdm_submodule_unregister_driver(struct pdm_subdriver *driver) {
    if (driver->exit) {
        driver->exit();
    }
    list_del(&driver->list);
    return 0;
}

int pdm_submodule_register_drivers(void)
{
    int i, ret;

    INIT_LIST_HEAD(&pdm_submodule_driver_list);

    for (i = 0; sub_drivers[i].init; i++) {
        ret = pdm_submodule_register_driver(&sub_drivers[i]);
        if (ret) {
            osa_error("Failed to register driver %d\n", i);
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
