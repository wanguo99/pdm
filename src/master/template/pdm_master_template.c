#include <linux/spi/spi.h>

#include "pdm.h"
#include "master/pdm_template.h"


static struct pdm_master *template_master = NULL;


static int pdm_master_template_probe(struct pdm_device *pdmdev)
{
    int status;

    status = pdm_master_client_add(template_master, pdmdev);
    if (status)
    {
        OSA_INFO("Template Master Add Device Failed.\n");
        return status;
    }

    OSA_INFO("Template PDM Device Probed.\n");
    return 0;
}

static void pdm_master_template_remove(struct pdm_device *pdmdev)
{
    int status;

    status = pdm_master_client_delete(template_master, pdmdev);
    if (status)
    {
        OSA_INFO("Template Master Delete Device Failed.\n");
        return;
    }

    OSA_INFO("Template PDM Device Removed.\n");
    return;

}


static const struct of_device_id of_pdm_master_template_match[] = {
    { .compatible = "template,pdm-device-spi", },
    { .compatible = "template,pdm-device-i2c", },
    { .compatible = "template,pdm-device-pwm", },
    { .compatible = "template,pdm-device-gpio", },
    { .compatible = "template,pdm-device-adc", },
    {},
};
MODULE_DEVICE_TABLE(of, of_pdm_master_template_match);


static struct pdm_driver pdm_master_template_driver = {
    .probe      = pdm_master_template_probe,
    .remove     = pdm_master_template_remove,
    .driver     = {
        .name   = "pdm-device-spi",
        .of_match_table = of_pdm_master_template_match,
    },
};


int pdm_master_template_driver_init(void)
{
    int status;

    template_master = pdm_master_alloc(sizeof(void*));
    if (!template_master)
    {
        OSA_ERROR("Failed to allocate pdm_master.\n");
        return -ENOMEM;
    }


    status = pdm_master_register(template_master);
    if (status) {
        OSA_ERROR("Failed to register Template PDM Master.\n");
        goto err_master_free;
    }

    status = pdm_register_driver(THIS_MODULE, &pdm_master_template_driver);
    if (status) {
        OSA_ERROR("Failed to register Template PDM Master Driver.\n");
        goto err_master_unregister;
    }

    OSA_INFO("Template PDM Master Driver Initialized.\n");
    return 0;

err_master_unregister:
    pdm_master_unregister(template_master);
err_master_free:
    pdm_master_free(template_master);
    return status;
}


void pdm_master_template_driver_exit(void)
{
    pdm_unregister_driver(&pdm_master_template_driver);
    pdm_master_unregister(template_master);
    pdm_master_free(template_master);
    OSA_INFO("Template PDM Master Driver Exited.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("Template PDM Device Driver");
