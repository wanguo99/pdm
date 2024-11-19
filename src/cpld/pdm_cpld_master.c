#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/version.h>

#include "pdm.h"
#include "pdm_cpld.h"

static struct pdm_master *pdm_cpld_master;
static struct class *pdm_cpld_class;

int pdm_cpld_device_register(struct pdm_cpld_device *cpld_dev) {
    int ret;

    if (!pdm_cpld_master) {
        return -ENODEV;
    }

    cpld_dev->pdm_dev->master = pdm_cpld_master;

    // Register the device with the PDM core
    ret = pdm_device_register(cpld_dev->pdm_dev);
    if (ret) {
        return ret;
    }

    // Add the device to the master's list
    list_add_tail(&cpld_dev->pdm_dev->node, &pdm_cpld_master->slaves);

    return 0;
}

void pdm_cpld_device_unregister(struct pdm_cpld_device *cpld_dev) {
    if (cpld_dev && cpld_dev->pdm_dev) {
        // Remove the device from the master's list
        list_del(&cpld_dev->pdm_dev->node);

        // Unregister the device with the PDM core
        pdm_device_unregister(cpld_dev->pdm_dev);
        pdm_device_free(cpld_dev->pdm_dev);
    }
}

int pdm_cpld_device_alloc(struct pdm_cpld_device **cpld_dev) {
    int ret;
    struct pdm_device *base_dev;

    *cpld_dev = kzalloc(sizeof(**cpld_dev), GFP_KERNEL);
    if (!*cpld_dev) {
        return -ENOMEM;
    }

    ret = pdm_device_alloc(&base_dev, pdm_cpld_master);
    if (ret) {
        kfree(*cpld_dev);
        return ret;
    }

    (*cpld_dev)->pdm_dev = base_dev;

    return 0;
}

void pdm_cpld_device_free(struct pdm_cpld_device *cpld_dev) {
    if (cpld_dev) {
        if (cpld_dev->pdm_dev) {
            pdm_device_free(cpld_dev->pdm_dev);
        }
        kfree(cpld_dev);
    }
}

int pdm_cpld_master_init(void) {
    int ret;

    printk(KERN_INFO "CPLD Master initialized\n");

    ret = pdm_master_alloc(&pdm_cpld_master, "pdm_cpld");
    if (ret) {
        printk(KERN_ERR "[WANGUO] (%s:%d) Failed to allocate PDM master\n", __func__, __LINE__);
        return ret;
    }

    INIT_LIST_HEAD(&pdm_cpld_master->slaves);

    // Create the class for the PDM CPLD devices
#if LINUX_VERSION_CODE > KERNEL_VERSION(6, 7, 0)
    pdm_cpld_class = class_create("pdm_cpld_class");
#else
    pdm_cpld_class = class_create(THIS_MODULE, "pdm_cpld_class");
#endif
    if (IS_ERR(pdm_cpld_class)) {
        ret = PTR_ERR(pdm_cpld_class);
        printk(KERN_ERR "[WANGUO] (%s:%d) Failed to create class: %d\n", __func__, __LINE__, ret);
        pdm_master_free(pdm_cpld_master);
        return ret;
    }

    // Set the device name
    ret = dev_set_name(&pdm_cpld_master->dev, "pdm_cpld");
    if (ret) {
        printk(KERN_ERR "[WANGUO] (%s:%d) Failed to set device name: %d\n", __func__, __LINE__, ret);
        class_destroy(pdm_cpld_class);
        pdm_master_free(pdm_cpld_master);
        return ret;
    }

    pdm_cpld_master->dev.class = pdm_cpld_class;

    // Register the master device
    ret = pdm_master_register(pdm_cpld_master);
    if (ret) {
        printk(KERN_ERR "[WANGUO] (%s:%d) Failed to register PDM master: %d\n", __func__, __LINE__, ret);
        class_destroy(pdm_cpld_class);
        pdm_master_free(pdm_cpld_master);
        return ret;
    }

    return 0;
}

void pdm_cpld_master_exit(void) {
    printk(KERN_INFO "CPLD Master exited\n");

    // Unregister the master device
    pdm_master_unregister(pdm_cpld_master);

    // Free the master device
    pdm_master_free(pdm_cpld_master);

    // Destroy the class
    class_destroy(pdm_cpld_class);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wanguo");
MODULE_DESCRIPTION("PDM CPLD Master Module.");
