#include "pdm.h"
#include "pdm_master_template.h"

static struct pdm_master *template_master = NULL;

/**
 * @brief 模板 PDM 设备探测函数
 *
 * 该函数在 PDM 设备被探测到时调用，负责将设备添加到模板主设备中。
 *
 * @param pdmdev PDM 设备指针
 * @return 成功返回 0，失败返回负错误码
 */
static int pdm_master_template_probe(struct pdm_device *pdmdev)
{
    int status;
    struct pdm_device_template_priv *data;

    status = pdm_master_client_add(template_master, pdmdev);
    if (status) {
        OSA_ERROR("Template Master Add Device Failed, status=%d.\n", status);
        return status;
    }

    status = pdm_device_devdata_alloc(pdmdev, sizeof(struct pdm_device_template_priv));
    if (status) {
        OSA_ERROR("Alloc Device Private Data Failed, status=%d.\n", status);
        goto err_client_del;
    }

    data = (struct pdm_device_template_priv *)pdm_device_devdata_get(pdmdev);
    if (!data)
    {
        OSA_ERROR("Get Device Private Data Failed, status=%d.\n", status);
        goto err_devdata_free;
    }
    data->ops = NULL;

    OSA_DEBUG("Template PDM Device Probed.\n");
    return 0;

err_devdata_free:
    pdm_device_devdata_free(pdmdev);

err_client_del:
    pdm_master_client_delete(template_master, pdmdev);
    return status;
}

/**
 * @brief 模板 PDM 设备移除函数
 *
 * 该函数在 PDM 设备被移除时调用，负责将设备从模板主设备中删除。
 *
 * @param pdmdev PDM 设备指针
 */
static void pdm_master_template_remove(struct pdm_device *pdmdev)
{
    int status;

    pdm_device_devdata_free(pdmdev);

    status = pdm_master_client_delete(template_master, pdmdev);
    if (status) {
        OSA_ERROR("Template Master Delete Device Failed, status=%d.\n", status);
        return;
    }

    OSA_DEBUG("Template PDM Device Removed.\n");
}

/**
 * @brief 设备树匹配表
 *
 * 该表定义了支持的设备树兼容属性。
 */
static const struct of_device_id of_pdm_master_template_match[] = {
    { .compatible = "template,pdm-device-spi", },
    { .compatible = "template,pdm-device-i2c", },
    { .compatible = "template,pdm-device-pwm", },
    { .compatible = "template,pdm-device-gpio", },
    { .compatible = "template,pdm-device-adc", },
    {},
};
MODULE_DEVICE_TABLE(of, of_pdm_master_template_match);

/**
 * @brief 模板 PDM 驱动结构体
 *
 * 该结构体定义了模板 PDM 驱动的基本信息和操作函数。
 */
static struct pdm_driver pdm_master_template_driver = {
    .probe = pdm_master_template_probe,
    .remove = pdm_master_template_remove,
    .driver = {
        .name = "pdm-device-template",
        .of_match_table = of_pdm_master_template_match,
    },
};

/**
 * @brief 初始化模板 PDM 主设备驱动
 *
 * 该函数用于初始化模板 PDM 主设备驱动，分配和注册主设备及驱动。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_master_template_driver_init(void)
{
    int status;

    template_master = pdm_master_alloc(sizeof(struct pdm_master_template_priv));
    if (!template_master) {
        OSA_ERROR("Failed to allocate pdm_master.\n");
        return -ENOMEM;
    }

    strncpy(template_master->name, PDM_MASTER_TEMPLATE_NAME, strlen(PDM_MASTER_TEMPLATE_NAME));
    status = pdm_master_register(template_master);
    if (status) {
        OSA_ERROR("Failed to register Template PDM Master, status=%d.\n", status);
        goto err_master_free;
    }

    status = pdm_bus_register_driver(THIS_MODULE, &pdm_master_template_driver);
    if (status) {
        OSA_ERROR("Failed to register Template PDM Master Driver, status=%d.\n", status);
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

/**
 * @brief 退出模板 PDM 主设备驱动
 *
 * 该函数用于退出模板 PDM 主设备驱动，注销驱动和主设备，释放相关资源。
 */
void pdm_master_template_driver_exit(void)
{
    pdm_bus_unregister_driver(&pdm_master_template_driver);
    pdm_master_unregister(template_master);
    pdm_master_free(template_master);
    OSA_INFO("Template PDM Master Driver Exited.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("Template PDM Device Driver");
