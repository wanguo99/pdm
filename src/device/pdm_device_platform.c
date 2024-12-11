#include <linux/platform_device.h>

#include "pdm.h"
#include "pdm_device_priv.h"

/**
 * @brief PLATFORM 设备探测函数
 *
 * 该函数在 PLATFORM 设备被探测到时调用，负责初始化和注册 PDM 设备。
 *
 * @param pdev 指向 PLATFORM 设备的指针
 * @return 成功返回 0，失败返回负错误码
 */
static int pdm_device_platform_probe(struct platform_device *pdev) {
    struct pdm_device *pdmdev;
    int status;

    OSA_DEBUG("PDM Device PLATFORM Device Probe.\n");

    pdmdev = pdm_device_alloc();
    if (!pdmdev) {
        OSA_ERROR("Failed to allocate pdm_device.\n");
        return -ENOMEM;
    }

    pdmdev->dev.parent = &pdev->dev;
    status = pdm_device_register(pdmdev);
    if (status) {
        OSA_ERROR("Failed to register pdm device, status=%d.\n", status);
        goto free_pdmdev;
    }

    OSA_DEBUG("PDM Device PLATFORM Device Probed.\n");
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
static int pdm_device_platform_real_remove(struct platform_device *pdev) {
    struct pdm_device *pdmdev;

    pdmdev = pdm_bus_find_device_by_parent(&pdev->dev);
    if (!pdmdev) {
        OSA_ERROR("Failed to find pdm device from bus.\n");
        return -ENODEV;
    } else {
        OSA_DEBUG("Found SPI PDM Device: %s", dev_name(&pdmdev->dev));
    }

    pdm_device_unregister(pdmdev);

    OSA_DEBUG("PDM Device PLATFORM Device Removed.\n");
    return 0;
}

/**
 * @brief 兼容旧内核版本的 PLATFORM 移除函数
 *
 * 该函数用于兼容 Linux 内核版本低于 6.10.0 的情况。
 *
 * @param pdev PLATFORM 设备指针
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 10, 0)
static int pdm_device_platform_remove(struct platform_device *pdev) {
    return pdm_device_platform_real_remove(pdev);
}
#else
static void pdm_device_platform_remove(struct platform_device *pdev) {
    int status;
    status = pdm_device_platform_real_remove(pdev);
    if (status) {
        OSA_ERROR("pdm_device_platform_real_remove failed.\n");
    }
}
#endif

/**
 * @brief PLATFORM 设备 ID 表
 *
 * 该表定义了支持的 PLATFORM 设备 ID。
 */
static const struct platform_device_id pdm_device_platform_ids[] = {
    { .name = "pdm-device-platform" },
    { .name = "pdm-device-gpio" },
    { .name = "pdm-device-pwm" },
    { .name = "pdm-device-tty" },
    { },  // 终止符
};
MODULE_DEVICE_TABLE(platform, pdm_device_platform_ids);

/**
 * @brief DEVICE_TREE 匹配表
 *
 * 该表定义了支持的 DEVICE_TREE 兼容性字符串。
 */
static const struct of_device_id pdm_device_platform_of_match[] = {
    { .compatible = "led,pdm-device-gpio" },
    { .compatible = "led,pdm-device-pwm" },
    { },  // 终止符
};
MODULE_DEVICE_TABLE(of, pdm_device_platform_of_match);

/**
 * @brief PLATFORM 驱动结构体
 *
 * 该结构体定义了 PLATFORM 驱动的基本信息和操作函数。
 */
static struct platform_driver pdm_device_platform_driver = {
    .probe = pdm_device_platform_probe,
    .remove = pdm_device_platform_remove,
    .driver = {
        .name = "pdm-device-platform",
        .of_match_table = pdm_device_platform_of_match,
    },
    .id_table = pdm_device_platform_ids,
};

/**
 * @brief 初始化 PLATFORM 驱动
 *
 * 该函数用于初始化 PLATFORM 驱动，并将其注册到系统中。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_device_platform_driver_init(void) {
    int status;

    status = platform_driver_register(&pdm_device_platform_driver);
    if (status) {
        OSA_ERROR("Failed to register PDM Device PLATFORM Driver, status=%d.\n", status);
        return status;
    }
    OSA_DEBUG("PDM Device PLATFORM Driver Initialized.\n");
    return 0;
}

/**
 * @brief 退出 PLATFORM 驱动
 *
 * 该函数用于退出 PLATFORM 驱动，并将其从系统中注销。
 */
void pdm_device_platform_driver_exit(void) {
    platform_driver_unregister(&pdm_device_platform_driver);
    OSA_DEBUG("PDM Device PLATFORM Driver Exited.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Device PLATFORM Driver");
