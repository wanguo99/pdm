#include <linux/platform_device.h>

#include "pdm.h"


/**
 * @brief PLATFORM 设备探测函数
 *
 * 该函数在 PLATFORM 设备被探测到时调用，负责初始化和注册 PDM 设备。
 *
 * @param pdev 指向 PLATFORM 设备的指针
 * @return 成功返回 0，失败返回负错误码
 */
static int pdm_device_platform_probe(struct platform_device *pdev)
{
    struct pdm_device *pdmdev;
    const char *compatible;
    int status;

    pdmdev = pdm_device_alloc(sizeof(void*));
    if (!pdmdev) {
        OSA_ERROR("Failed to allocate pdm_device.\n");
        return -ENOMEM;
    }

    status = of_property_read_string(pdev->dev.of_node, "compatible", &compatible);
    if (status) {
        pr_err("Failed to read compatible property: %d\n", status);
        goto free_pdmdev;
    }

    strcpy(pdmdev->compatible, compatible);
    pdmdev->real_device = pdev;
    status = pdm_device_register(pdmdev);
    if (status) {
        OSA_ERROR("Failed to register pdm device, status=%d.\n", status);
        goto free_pdmdev;
    }

    OSA_INFO("PDM Device PLATFORM Device Probed.\n");
    return 0;

free_pdmdev:
    pdm_device_free(pdmdev);
    return status;
}

/**
 * @brief PLATFORM 设备移除函数
 *
 * 该函数在 PLATFORM 设备被移除时调用，负责注销和释放 PDM 设备。
 *
 * @param pdev 指向 PLATFORM 设备的指针
 * @return 成功返回 0，失败返回负错误码
 */
static int pdm_device_platform_remove(struct platform_device *pdev)
{
#if 0

    struct pdm_device *pdmdev = pdm_device_master_find_pdmdev(pdev);
    if (NULL == pdmdev) {
        OSA_ERROR("Failed to find pdm device from master.\n");
        return -EINVAL;
    }

    pdm_device_master_unregister_device(pdmdev);
    pdm_device_free(pdmdev);
#endif
    OSA_INFO("PDM Device PLATFORM Device Removed.\n");
    return 0;
}

static const struct platform_device_id pdm_device_platform_ids[] = {
	{ .name = "pdm-device-platform",  },
	{ .name = "pdm-device-gpio", },
	{ .name = "pdm-device-pwm",  },
	{ .name = "pdm-device-uart", },
	{ .name = "pdm-device-adc",  },
	{ .name = "pdm-device-dac",  },
	{},
};
MODULE_DEVICE_TABLE(platform, pdm_device_platform_ids);

static struct platform_driver pdm_device_platform_driver = {
	.probe		= pdm_device_platform_probe,
	.remove	= pdm_device_platform_remove,
	.driver		= {
		.name	= "pdm-platform-device",
	},
	.id_table   = pdm_device_platform_ids,
};


/**
 * @brief 初始化 PLATFORM 驱动
 *
 * 该函数用于初始化 PLATFORM 驱动，注册 PLATFORM 驱动到系统。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_device_platform_driver_init(void) {
    int status;

    status = platform_driver_register(&pdm_device_platform_driver);
    if (status) {
        OSA_ERROR("Failed to register PDM Device PLATFORM Driver.\n");
        return status;
    }
    OSA_INFO("PDM Device PLATFORM Driver Initialized.\n");
    return 0;
}

/**
 * @brief 退出 PLATFORM 驱动
 *
 * 该函数用于退出 PLATFORM 驱动，注销 PLATFORM 驱动。
 */
void pdm_device_platform_driver_exit(void) {
    platform_driver_unregister(&pdm_device_platform_driver);
    OSA_INFO("PDM Device PLATFORM Driver Exited.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Device PLATFORM Driver");
