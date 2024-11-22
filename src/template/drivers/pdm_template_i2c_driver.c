#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/version.h>

#include "pdm.h"
#include "pdm_template.h"

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
struct i2c_device_id {
    char name[I2C_NAME_SIZE];
    kernel_ulong_t driver_data;
};
#endif

static int pdm_template_i2c_real_probe(struct i2c_client *client, const struct i2c_device_id *id) {
    struct pdm_device *pdmdev;
    struct pdm_template_device_priv *pstTemplateDevPriv;
    int ret;

    osa_info("Template I2C Device Probed.\n");

    pdmdev = pdm_device_alloc(sizeof(struct pdm_template_device_priv));
    if (!pdmdev) {
        osa_error("Failed to allocate pdm_device.\n");
        return -ENOMEM;
    }

    pdmdev->real_device = client;
    ret = pdm_template_master_register_device(pdmdev);
    if (ret) {
        osa_error("Failed to add template device, ret=%d.\n", ret);
        goto free_pdmdev;
    }

    pstTemplateDevPriv = pdm_device_get_devdata(pdmdev);
    if (!pstTemplateDevPriv) {
        osa_error("Failed to get device private data.\n");
        ret = -EFAULT;
        goto unregister_pdmdev;
    }
    pstTemplateDevPriv->ops = NULL;

    return 0;

unregister_pdmdev:
    pdm_template_master_unregister_device(pdmdev);

free_pdmdev:
    pdm_device_free(pdmdev);

    return ret;
}


static int pdm_template_i2c_real_remove(struct i2c_client *client) {

    struct pdm_device *pdmdev = pdm_template_master_find_pdmdev(client);
    if (NULL == pdmdev){
        osa_error("%s:%d:[%s]  \n", __FILE__, __LINE__, __func__);
        return -ENODEV;
    }

    pdm_template_master_unregister_device(pdmdev);
    pdm_device_free(pdmdev);
    return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
static int pdm_template_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id) {
    return pdm_template_i2c_real_probe(client, id);
}
#else
static int pdm_template_i2c_probe(struct i2c_client *client) {
    return pdm_template_i2c_real_probe(client, NULL);
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
static int pdm_template_i2c_remove(struct i2c_client *client) {
    return pdm_template_i2c_real_remove(client);
}
#else
static void pdm_template_i2c_remove(struct i2c_client *client) {
    (void)pdm_template_i2c_real_remove(client);
}
#endif



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
        .name = "pdm_template_i2c",
		.of_match_table = pdm_template_i2c_matches,
        .owner = THIS_MODULE,
    },
    .probe = pdm_template_i2c_probe,
    .remove = pdm_template_i2c_remove,
    .id_table = pdm_template_i2c_id,
};

int pdm_template_i2c_driver_init(void) {
    int ret;
    ret = i2c_add_driver(&pdm_template_i2c_driver);
    if (ret) {
        osa_error("Failed to register Template I2C driver.\n");
    }

    osa_info("Template I2C Driver initialized.\n");
    return 0;
}

void pdm_template_i2c_driver_exit(void) {
    osa_info("Template I2C Driver exit.\n");
    i2c_del_driver(&pdm_template_i2c_driver);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wanguo");
MODULE_DESCRIPTION("PDM Template I2C Driver.");
