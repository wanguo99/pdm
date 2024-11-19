#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/version.h>

#include "pdm.h"
#include "pdm_cpld.h"

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
static int pdm_cpld_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id) {
#else
static int pdm_cpld_i2c_probe(struct i2c_client *client) {
#endif
    struct pdm_cpld_device *cpld_dev;
    int ret;

    printk(KERN_INFO "CPLD I2C Device probed\n");

    ret = pdm_cpld_device_alloc(&cpld_dev);
    if (ret) {
        return ret;
    }

    cpld_dev->i2c_client = client;

    // Initialize any CPLD-specific data here

    // Register the device
    ret = pdm_cpld_device_register(cpld_dev);
    if (ret) {
        pdm_cpld_device_free(cpld_dev);
        return ret;
    }

    i2c_set_clientdata(client, cpld_dev);

    return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
static int pdm_cpld_i2c_remove(struct i2c_client *client) {
#else
static void pdm_cpld_i2c_remove(struct i2c_client *client) {
#endif
    struct pdm_cpld_device *cpld_dev = i2c_get_clientdata(client);

    printk(KERN_INFO "CPLD I2C Device removed\n");

    pdm_cpld_device_unregister(cpld_dev);
    pdm_cpld_device_free(cpld_dev);

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
    return 0;
#endif
}


static const struct i2c_device_id pdm_cpld_i2c_id[] = {
    { "pdm_cpld", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, pdm_cpld_i2c_id);

static struct i2c_driver pdm_cpld_i2c_driver = {
    .driver = {
        .name = "pdm_cpld",
        .owner = THIS_MODULE,
    },
    .probe = pdm_cpld_i2c_probe,
    .remove = pdm_cpld_i2c_remove,
    .id_table = pdm_cpld_i2c_id,
};

int pdm_cpld_i2c_driver_init(void) {
    int ret;

    printk(KERN_INFO "CPLD I2C Driver initialized\n");

    ret = i2c_add_driver(&pdm_cpld_i2c_driver);
    if (ret) {
        printk(KERN_ERR "Failed to register CPLD I2C driver\n");
    }

    return ret;
}

void pdm_cpld_i2c_driver_exit(void) {
    printk(KERN_INFO "CPLD I2C Driver exited\n");

    i2c_del_driver(&pdm_cpld_i2c_driver);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wanguo");
MODULE_DESCRIPTION("PDM CPLD I2C Driver.");
