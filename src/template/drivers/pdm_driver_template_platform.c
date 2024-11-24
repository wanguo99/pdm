#include <linux/container_of.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/types.h>

#include "pdm.h"
#include "pdm_template.h"


static int pdm_template_platform_probe(struct platform_device *pdev)
{
    struct pdm_device *pdmdev;
    struct pdm_template_device_priv *pstTemplateDevPriv;
    int ret;

    pdmdev = pdm_device_alloc(sizeof(struct pdm_template_device_priv));
    if (!pdmdev) {
        OSA_ERROR("Failed to allocate pdm_device.\n");
        return -ENOMEM;
    }

    pdmdev->real_device = pdev;
    strcpy(pdmdev->compatible, "pdm,template-platform");
    ret = pdm_template_master_register_device(pdmdev);
    if (ret) {
        OSA_ERROR("Failed to add template device, ret=%d.\n", ret);
        goto free_pdmdev;
    }

    pstTemplateDevPriv = pdm_device_devdata_get(pdmdev);
    if (!pstTemplateDevPriv) {
        OSA_ERROR("Failed to get device private data.\n");
        ret = -EFAULT;
        goto unregister_pdmdev;
    }
    pstTemplateDevPriv->ops = NULL;

    OSA_INFO("Template PLATFORM Device Probed.\n");
    return 0;

unregister_pdmdev:
    pdm_template_master_unregister_device(pdmdev);

free_pdmdev:
    pdm_device_free(pdmdev);

    return ret;

}

static int pdm_template_platform_remove(struct platform_device *pdev)
{
    struct pdm_device *pdmdev = pdm_template_master_find_pdmdev(pdev);
    if (NULL == pdmdev) {
        OSA_ERROR("Failed to find pdm device from master.\n");
        return -EINVAL;
    }

    pdm_template_master_unregister_device(pdmdev);
    pdm_device_free(pdmdev);

    OSA_INFO("Template PLATFORM Device Removed.\n");
    return 0;
}

/**
 * @ dts节点配置示例
 *  template-platform-0 {
 *      compatible = "pdm,template-platform";
 *      status = "okay";
 *  };
*/
static const struct of_device_id of_platform_leds_match[] = {
	{ .compatible = "pdm,template-platform", },
	{},
};

MODULE_DEVICE_TABLE(of, of_platform_leds_match);

static struct platform_driver pdm_template_platform_driver = {
	.probe		= pdm_template_platform_probe,
	.remove	= pdm_template_platform_remove,
	.driver		= {
		.name	= "pdm-template-platform",
		.of_match_table = of_platform_leds_match,
	},
};


/**
 * @brief 初始化 PLATFORM 驱动
 *
 * 该函数用于初始化 PLATFORM 驱动，注册 PLATFORM 驱动到系统。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_template_platform_driver_init(void) {
    int ret;

    ret = platform_driver_register(&pdm_template_platform_driver);
    if (ret) {
        OSA_ERROR("Failed to register Template PLATFORM Driver.\n");
        return ret;
    }
    OSA_INFO("Template PLATFORM Driver Initialized.\n");
    return 0;
}

/**
 * @brief 退出 PLATFORM 驱动
 *
 * 该函数用于退出 PLATFORM 驱动，注销 PLATFORM 驱动。
 */
void pdm_template_platform_driver_exit(void) {
    platform_driver_unregister(&pdm_template_platform_driver);
    OSA_INFO("Template PLATFORM Driver Exited.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Template PLATFORM Driver");
