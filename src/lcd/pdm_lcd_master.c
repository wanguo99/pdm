#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/version.h>

#include "pdm.h"
#include "pdm_lcd.h"

static struct pdm_master *pdm_lcd_master;
static struct class *pdm_lcd_class;

int pdm_lcd_device_register(struct pdm_lcd_device *lcd_dev) {
    int ret;

    if (!pdm_lcd_master) {
        return -ENODEV;
    }

    lcd_dev->pdm_dev->master = pdm_lcd_master;

    // Register the device with the PDM core
    ret = pdm_device_register(lcd_dev->pdm_dev);
    if (ret) {
        return ret;
    }

    // Add the device to the master's list
    list_add_tail(&lcd_dev->pdm_dev->node, &pdm_lcd_master->slaves);

    return 0;
}

void pdm_lcd_device_unregister(struct pdm_lcd_device *lcd_dev) {
    if (lcd_dev && lcd_dev->pdm_dev) {
        // Remove the device from the master's list
        list_del(&lcd_dev->pdm_dev->node);

        // Unregister the device with the PDM core
        pdm_device_unregister(lcd_dev->pdm_dev);
        pdm_device_free(lcd_dev->pdm_dev);
    }
}

int pdm_lcd_device_alloc(struct pdm_lcd_device **lcd_dev) {
    int ret;
    struct pdm_device *base_dev;

    *lcd_dev = kzalloc(sizeof(**lcd_dev), GFP_KERNEL);
    if (!*lcd_dev) {
        return -ENOMEM;
    }

    ret = pdm_device_alloc(&base_dev, pdm_lcd_master);
    if (ret) {
        kfree(*lcd_dev);
        return ret;
    }

    (*lcd_dev)->pdm_dev = base_dev;

    return 0;
}

void pdm_lcd_device_free(struct pdm_lcd_device *lcd_dev) {
    if (lcd_dev) {
        if (lcd_dev->pdm_dev) {
            pdm_device_free(lcd_dev->pdm_dev);
        }
        kfree(lcd_dev);
    }
}

int pdm_lcd_master_init(void) {
    int ret;

    printk(KERN_INFO "LCD Master initialized\n");

    ret = pdm_master_alloc(&pdm_lcd_master, "pdm_lcd");
    if (ret) {
        printk(KERN_ERR "[WANGUO] (%s:%d) Failed to allocate PDM master\n", __func__, __LINE__);
        return ret;
    }

    INIT_LIST_HEAD(&pdm_lcd_master->slaves);

    // Create the class for the PDM LCD devices
#if LINUX_VERSION_CODE > KERNEL_VERSION(6, 7, 0)
    pdm_lcd_class = class_create("pdm_lcd_class");
#else
    pdm_lcd_class = class_create(THIS_MODULE, "pdm_lcd_class");
#endif
    if (IS_ERR(pdm_lcd_class)) {
        ret = PTR_ERR(pdm_lcd_class);
        printk(KERN_ERR "[WANGUO] (%s:%d) Failed to create class: %d\n", __func__, __LINE__, ret);
        pdm_master_free(pdm_lcd_master);
        return ret;
    }

    // Set the device name
    ret = dev_set_name(&pdm_lcd_master->dev, "pdm_lcd");
    if (ret) {
        printk(KERN_ERR "[WANGUO] (%s:%d) Failed to set device name: %d\n", __func__, __LINE__, ret);
        class_destroy(pdm_lcd_class);
        pdm_master_free(pdm_lcd_master);
        return ret;
    }

    pdm_lcd_master->dev.class = pdm_lcd_class;

    // Register the master device
    ret = pdm_master_register(pdm_lcd_master);
    if (ret) {
        printk(KERN_ERR "[WANGUO] (%s:%d) Failed to register PDM master: %d\n", __func__, __LINE__, ret);
        class_destroy(pdm_lcd_class);
        pdm_master_free(pdm_lcd_master);
        return ret;
    }

    return 0;
}

void pdm_lcd_master_exit(void) {
    printk(KERN_INFO "LCD Master exited\n");

    // Unregister the master device
    pdm_master_unregister(pdm_lcd_master);

    // Free the master device
    pdm_master_free(pdm_lcd_master);

    // Destroy the class
    class_destroy(pdm_lcd_class);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wanguo");
MODULE_DESCRIPTION("PDM LCD Master Module.");
