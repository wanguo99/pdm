// SPDX-License-Identifier: GPL-2.0-only
/*
 * LEDs driver for GPIOs
 *
 * Copyright (C) 2007 8D Technologies inc.
 * Raphael Assenat <raph@8d.com>
 * Copyright (C) 2008 Freescale Semiconductor, Inc.
 */
#include <linux/container_of.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/leds.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/overflow.h>
#include <linux/pinctrl/consumer.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/slab.h>
#include <linux/types.h>

#include "pdm.h"
#include "pdm_template.h"


static int pdm_template_gpio_probe(struct platform_device *pdev)
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
    strcpy(pdmdev->compatible, "pdm,template-gpio");
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

    OSA_INFO("Template GPIO Device Probed.\n");
    return 0;

unregister_pdmdev:
    pdm_template_master_unregister_device(pdmdev);

free_pdmdev:
    pdm_device_free(pdmdev);

    return ret;

}

static int pdm_template_gpio_remove(struct platform_device *pdev)
{
    struct pdm_device *pdmdev = pdm_template_master_find_pdmdev(pdev);
    if (NULL == pdmdev) {
        OSA_ERROR("Failed to find pdm device from master.\n");
        return -EINVAL;
    }

    pdm_template_master_unregister_device(pdmdev);
    pdm_device_free(pdmdev);

    OSA_INFO("Template GPIO Device Removed.\n");
    return 0;
}

/**
 * @ dts节点配置示例
 *  template-gpio-0 {
 *      compatible = "pdm,template-gpio";
 *      status = "okay";
 *  };
*/
static const struct of_device_id of_gpio_leds_match[] = {
	{ .compatible = "pdm,template-gpio", },
	{},
};

MODULE_DEVICE_TABLE(of, of_gpio_leds_match);

static struct platform_driver pdm_template_gpio_driver = {
	.probe		= pdm_template_gpio_probe,
	.remove	= pdm_template_gpio_remove,
	.driver		= {
		.name	= "pdm-template-gpio",
		.of_match_table = of_gpio_leds_match,
	},
};


/**
 * @brief 初始化 GPIO 驱动
 *
 * 该函数用于初始化 GPIO 驱动，注册 GPIO 驱动到系统。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_template_gpio_driver_init(void) {
    int ret;

    ret = platform_driver_register(&pdm_template_gpio_driver);
    if (ret) {
        OSA_ERROR("Failed to register Template GPIO Driver.\n");
        return ret;
    }
    OSA_INFO("Template GPIO Driver Initialized.\n");
    return 0;
}

/**
 * @brief 退出 GPIO 驱动
 *
 * 该函数用于退出 GPIO 驱动，注销 GPIO 驱动。
 */
void pdm_template_gpio_driver_exit(void) {
    platform_driver_unregister(&pdm_template_gpio_driver);
    OSA_INFO("Template GPIO Driver Exited.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Template GPIO Driver");
