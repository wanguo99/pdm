#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/version.h>

#include "pdm.h"
#include "pdm_cpld.h"


struct pdm_cpld_device *g_pstCpldDev;


#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
static int pdm_cpld_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id) {
#else
static int pdm_cpld_i2c_probe(struct i2c_client *client) {
#endif

    struct pdm_device *pdmdev;
    int ret;

    printk(KERN_INFO "CPLD I2C Device probed\n");

    g_pstCpldDev = kzalloc(sizeof(struct pdm_cpld_device), GFP_KERNEL);
    if (!g_pstCpldDev) {
        pr_err("Failed to allocate memory for cpld_dev\n");
        return -ENOMEM;
    }

    pdmdev = pdm_device_alloc();
    if (!pdmdev) {
        pr_err("Failed to allocate pdm_device\n");
        goto free_cpld_dev;
    }

    g_pstCpldDev->pdmdev = pdmdev;
    g_pstCpldDev->client.i2cdev = client;

    ret = pdm_cpld_master_add_device(g_pstCpldDev);
    if (ret) {
        pr_err("Failed to add cpld device, ret=%d\n", ret);
        goto free_pdmdev;
    }

    ret = pdm_device_register(g_pstCpldDev->pdmdev);
    if (ret) {
        pr_err("Failed to register pdm_device, ret=%d\n", ret);
        goto master_del_pdmdev;
    }

    return 0;

master_del_pdmdev:
    pdm_cpld_master_del_device(g_pstCpldDev);

free_pdmdev:
    pdm_device_free(g_pstCpldDev->pdmdev);

free_cpld_dev:
    kfree(g_pstCpldDev);
    return ret;
}


#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
static int pdm_cpld_i2c_remove(struct i2c_client *client) {
#else
static void pdm_cpld_i2c_remove(struct i2c_client *client) {
#endif

    pdm_device_register(g_pstCpldDev->pdmdev);
    pdm_cpld_master_del_device(g_pstCpldDev);
    pdm_device_free(g_pstCpldDev->pdmdev);
    kfree(g_pstCpldDev);

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
