#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/version.h>
#include "pdm.h"
#include "pdm_lcd.h"

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
static int pdm_lcd_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id) {
#else
static int pdm_lcd_i2c_probe(struct i2c_client *client) {
#endif
    struct pdm_lcd_device *lcd_dev;
    int ret;

    printk(KERN_INFO "LCD I2C Device probed\n");

    ret = pdm_lcd_device_alloc(&lcd_dev);
    if (ret) {
        return ret;
    }

    // Initialize any CPLD-specific data here
    lcd_dev->i2c_client = client;


    // Register the device
    ret = pdm_lcd_device_register(lcd_dev);
    if (ret) {
        pdm_lcd_device_free(lcd_dev);
        return ret;
    }

    i2c_set_clientdata(client, lcd_dev);

    return 0;
}


#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
static int pdm_lcd_i2c_remove(struct i2c_client *client) {
#else
static void pdm_lcd_i2c_remove(struct i2c_client *client) {
#endif
    struct pdm_lcd_device *lcd_dev = i2c_get_clientdata(client);

    printk(KERN_INFO "LCD I2C Device removed\n");


    pdm_lcd_device_unregister(lcd_dev);
    pdm_lcd_device_free(lcd_dev);

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
    return 0;
#endif
}



static const struct i2c_device_id pdm_lcd_i2c_id[] = {
    { "pdm_lcd", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, pdm_lcd_i2c_id);

static struct i2c_driver pdm_lcd_i2c_driver = {
    .driver = {
        .name = "pdm_lcd",
        .owner = THIS_MODULE,
    },
    .probe = pdm_lcd_i2c_probe,
    .remove = pdm_lcd_i2c_remove,
    .id_table = pdm_lcd_i2c_id,
};

int pdm_lcd_i2c_driver_init(void) {
    int ret;

    printk(KERN_INFO "LCD I2C Driver initialized\n");

    ret = i2c_add_driver(&pdm_lcd_i2c_driver);
    if (ret) {
        printk(KERN_ERR "Failed to register CPLD I2C driver\n");
    }

    return ret;
}

void pdm_lcd_i2c_driver_exit(void) {
    printk(KERN_INFO "CPLD I2C Driver exited\n");

    i2c_del_driver(&pdm_lcd_i2c_driver);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wanguo");
MODULE_DESCRIPTION("PDM LCD I2C Driver.");
