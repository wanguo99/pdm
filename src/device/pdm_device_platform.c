#include <linux/platform_device.h>

#include "pdm.h"
#include "pdm_device_priv.h"

/**
 * @brief Probes the PLATFORM device and initializes the PDM device.
 *
 * This function is called when a PLATFORM device is detected, responsible for initializing and registering the PDM device.
 *
 * @param pdev Pointer to the PLATFORM device structure.
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_device_platform_probe(struct platform_device *pdev) {
    struct pdm_device *pdmdev;
    int status;

    OSA_DEBUG("PDM Device PLATFORM Device Probe.\n");

    pdmdev = pdm_device_alloc(&pdev->dev);
    if (IS_ERR(pdmdev)) {
        OSA_ERROR("Failed to allocate pdm_device.\n");
        return PTR_ERR(pdmdev);
    }

    status = pdm_device_register(pdmdev);
    if (status) {
        OSA_ERROR("Failed to register pdm device, status=%d.\n", status);
        pdm_device_free(pdmdev);
        return status;
    }

    OSA_DEBUG("PDM Device PLATFORM Device Probed.\n");
    return 0;
}

/**
 * @brief Removes the PLATFORM device and unregisters the PDM device.
 *
 * This function is called when a PLATFORM device is removed, responsible for unregistering and freeing the PDM device.
 *
 * @param pdev Pointer to the PLATFORM device structure.
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_device_platform_real_remove(struct platform_device *pdev) {
    struct pdm_device *pdmdev = pdm_bus_find_device_by_parent(&pdev->dev);

    if (!pdmdev) {
        OSA_ERROR("Failed to find pdm device from bus.\n");
        return -ENODEV;
    }

    pdm_device_unregister(pdmdev);
    pdm_device_free(pdmdev);
    OSA_DEBUG("PDM Device PLATFORM Device Removed.\n");
    return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 10, 0)
/**
 * @brief Compatibility remove function for older kernel versions.
 *
 * This function is used for Linux kernel versions below 6.10.0.
 *
 * @param pdev Pointer to the PLATFORM device structure.
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_device_platform_remove(struct platform_device *pdev) {
    return pdm_device_platform_real_remove(pdev);
}
#else
/**
 * @brief Removes the PLATFORM device and unregisters the PDM device.
 *
 * This function is called when a PLATFORM device is removed, responsible for unregistering and freeing the PDM device.
 *
 * @param pdev Pointer to the PLATFORM device structure.
 */
static void pdm_device_platform_remove(struct platform_device *pdev) {
    int status;

    status = pdm_device_platform_real_remove(pdev);
    if (status) {
        OSA_ERROR("pdm_device_platform_real_remove failed.\n");
    }
}
#endif

/**
 * @brief PLATFORM device ID table.
 *
 * Defines the supported PLATFORM device IDs.
 */
static const struct platform_device_id pdm_device_platform_ids[] = {
    { .name = "pdm-device-platform" },
    { .name = "pdm-device-gpio" },
    { .name = "pdm-device-pwm" },
    { .name = "pdm-device-tty" },
    { }
};
MODULE_DEVICE_TABLE(platform, pdm_device_platform_ids);

/**
 * @brief DEVICE_TREE match table.
 *
 * Defines the supported DEVICE_TREE compatibility strings.
 */
static const struct of_device_id pdm_device_platform_of_match[] = {
    { .compatible = "led,pdm-device-gpio" },
    { .compatible = "led,pdm-device-pwm" },
    { }
};
MODULE_DEVICE_TABLE(of, pdm_device_platform_of_match);

/**
 * @brief PLATFORM driver structure.
 *
 * Defines the basic information and operation functions of the PLATFORM driver.
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
 * @brief Initializes the PLATFORM driver.
 *
 * Registers the PLATFORM driver with the system.
 *
 * @return Returns 0 on success; negative error code on failure.
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
 * @brief Exits the PLATFORM driver.
 *
 * Unregisters the PLATFORM driver from the system.
 */
void pdm_device_platform_driver_exit(void) {
    platform_driver_unregister(&pdm_device_platform_driver);
    OSA_DEBUG("PDM Device PLATFORM Driver Exited.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Device PLATFORM Driver");
