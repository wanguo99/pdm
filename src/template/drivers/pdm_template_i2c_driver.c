#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/version.h>

#include "pdm.h"
#include "pdm_template.h"





#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
static int pdm_template_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id) {
#else
static int pdm_template_i2c_probe(struct i2c_client *client) {
#endif

    struct pdm_device *pdmdev;
    struct pdm_template_device_priv *pstTemplateDevPriv;
    int ret;

    osa_info("Template I2C Device Probed.\n");

    pdmdev = pdm_device_alloc(sizeof(struct pdm_template_device_priv));
    if (!pdmdev) {
        osa_error("Failed to allocate pdm_device.\n");
        return -ENOMEM;
    }

    // 指定当前pdm_device使用的master
    ret = pdm_template_master_add_device(pdmdev);
    if (ret) {
        osa_error("Failed to add template device, ret=%d.\n", ret);
        goto free_pdmdev;
    }

    ret = pdm_device_register(pdmdev);
    if (ret) {
        osa_error("Failed to register pdm_device, ret=%d.\n", ret);
        goto master_del_pdmdev;
    }

    // 保存物理设备地址
    pdmdev->real_device = client;

    // 设置私有数据
    pstTemplateDevPriv = pdm_device_get_devdata(pdmdev);
    pstTemplateDevPriv->ops = NULL;

    return 0;

master_del_pdmdev:
    pdm_template_master_del_device(pdmdev);

free_pdmdev:
    pdm_device_free(pdmdev);

    return ret;
}


#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
static int pdm_template_i2c_remove(struct i2c_client *client) {
#else
static void pdm_template_i2c_remove(struct i2c_client *client) {
#endif
    struct pdm_device *pdmdev = pdm_template_master_get_pdmdev_of_real_device(client);
    if (NULL == pdmdev)
    {
        osa_error("%s:%d:[%s]  \n", __FILE__, __LINE__, __func__);
        return;
    }

    pdm_device_unregister(pdmdev);
    pdm_template_master_del_device(pdmdev);
    pdm_device_free(pdmdev);

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

    osa_info("Template I2C Driver initialized.\n");

    ret = i2c_add_driver(&pdm_template_i2c_driver);
    if (ret) {
        osa_error("Failed to register TEMPLATE I2C driver.\n");
    }

    return ret;
}

void pdm_template_i2c_driver_exit(void) {
    osa_info("Template I2C Driver exit.\n");

    i2c_del_driver(&pdm_template_i2c_driver);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wanguo");
MODULE_DESCRIPTION("PDM Template I2C Driver.");
