#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/version.h>

#include "pdm.h"
#include "pdm_template.h"


struct pdm_template_device *g_pstTemplateDev;


#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
static int pdm_template_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id) {
#else
static int pdm_template_i2c_probe(struct i2c_client *client) {
#endif

    struct pdm_device *pdmdev;
    int ret;

    printk(KERN_INFO "TEMPLATE I2C Device probed\n");

    g_pstTemplateDev = kzalloc(sizeof(struct pdm_template_device), GFP_KERNEL);
    if (!g_pstTemplateDev) {
        pr_err("Failed to allocate memory for template_dev\n");
        return -ENOMEM;
    }

    pdmdev = pdm_device_alloc();
    if (!pdmdev) {
        pr_err("Failed to allocate pdm_device\n");
        goto free_template_dev;
    }

    g_pstTemplateDev->pdmdev = pdmdev;
    g_pstTemplateDev->client.i2cdev = client;
    ret = pdm_template_master_add_device(g_pstTemplateDev);
    if (ret) {
        pr_err("Failed to add template device, ret=%d\n", ret);
        goto free_pdmdev;
    }

    ret = pdm_device_register(g_pstTemplateDev->pdmdev);
    if (ret) {
        pr_err("Failed to register pdm_device, ret=%d\n", ret);
        goto master_del_pdmdev;
    }

    return 0;

master_del_pdmdev:
    pdm_template_master_del_device(g_pstTemplateDev);

free_pdmdev:
    pdm_device_free(g_pstTemplateDev->pdmdev);

free_template_dev:
    kfree(g_pstTemplateDev);
    return ret;
}


#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
static int pdm_template_i2c_remove(struct i2c_client *client) {
#else
static void pdm_template_i2c_remove(struct i2c_client *client) {
#endif

    pdm_device_register(g_pstTemplateDev->pdmdev);
    pdm_template_master_del_device(g_pstTemplateDev);
    pdm_device_free(g_pstTemplateDev->pdmdev);
    kfree(g_pstTemplateDev);

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
    return 0;
#endif
}


static const struct i2c_device_id pdm_template_i2c_id[] = {
    { "pdm_template", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, pdm_template_i2c_id);

static const struct of_device_id pdm_template_i2c_matches[] = {
	{ .compatible = "pdm_template,i2c" },
	{ }
};
MODULE_DEVICE_TABLE(of, pdm_template_i2c_matches);

static struct i2c_driver pdm_template_i2c_driver = {
    .driver = {
        .name = "pdm_template",
		.of_match_table = pdm_template_i2c_matches,
        .owner = THIS_MODULE,
    },
    .probe = pdm_template_i2c_probe,
    .remove = pdm_template_i2c_remove,
    .id_table = pdm_template_i2c_id,
};

int pdm_template_i2c_driver_init(void) {
    int ret;

    printk(KERN_INFO "TEMPLATE I2C Driver initialized\n");

    ret = i2c_add_driver(&pdm_template_i2c_driver);
    if (ret) {
        printk(KERN_ERR "Failed to register TEMPLATE I2C driver\n");
    }

    return ret;
}

void pdm_template_i2c_driver_exit(void) {
    printk(KERN_INFO "TEMPLATE I2C Driver exited\n");

    i2c_del_driver(&pdm_template_i2c_driver);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wanguo");
MODULE_DESCRIPTION("PDM TEMPLATE I2C Driver.");
