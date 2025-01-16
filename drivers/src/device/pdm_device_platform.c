#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/of_gpio.h>
#include <linux/pwm.h>

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
static int pdm_device_platform_probe(struct platform_device *pdev)
{
	struct pdm_device *pdmdev;
	int status;

	pdmdev = pdm_device_alloc(&pdev->dev);
	if (IS_ERR(pdmdev)) {
		OSA_ERROR("Failed to allocate pdm_device\n");
		return PTR_ERR(pdmdev);
	}

	status = pdm_device_register(pdmdev);
	if (status) {
		OSA_ERROR("Failed to register pdm device, status=%d\n", status);
		goto err_pdmdev_free;
	}

	return 0;

err_pdmdev_free:
	pdm_device_free(pdmdev);
	return status;
}

/**
 * @brief Removes the PLATFORM device and unregisters the PDM device.
 *
 * This function is called when a PLATFORM device is removed, responsible for unregistering and freeing the PDM device.
 *
 * @param pdev Pointer to the PLATFORM device structure.
 * @return Returns 0 on success; negative error code on failure.
 */
static void pdm_device_platform_real_remove(struct platform_device *pdev)
{
	struct pdm_device *pdmdev = pdm_bus_find_device_by_parent(&pdev->dev);
	if (pdmdev) {
		pdm_device_unregister(pdmdev);
		pdm_device_free(pdmdev);
	}
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
static int pdm_device_platform_remove(struct platform_device *pdev)
{
	pdm_device_platform_real_remove(pdev);
	return 0;
}
#else
/**
 * @brief Removes the PLATFORM device and unregisters the PDM device.
 *
 * This function is called when a PLATFORM device is removed, responsible for unregistering and freeing the PDM device.
 *
 * @param pdev Pointer to the PLATFORM device structure.
 */
static void pdm_device_platform_remove(struct platform_device *pdev)
{
	pdm_device_platform_real_remove(pdev);
}
#endif

/**
 * @brief DEVICE_TREE match table.
 *
 * Defines the supported DEVICE_TREE compatibility strings.
 */
static const struct of_device_id pdm_device_platform_of_match[] = {
	{ .compatible = "pdm-device-gpio" },
	{ .compatible = "pdm-device-pwm" },
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
};

/**
 * @brief Initializes the PLATFORM driver.
 *
 * Registers the PLATFORM driver with the system.
 *
 * @return Returns 0 on success; negative error code on failure.
 */
int pdm_device_platform_driver_init(void)
{
	return platform_driver_register(&pdm_device_platform_driver);
}

/**
 * @brief Exits the PLATFORM driver.
 *
 * Unregisters the PLATFORM driver from the system.
 */
void pdm_device_platform_driver_exit(void)
{
	platform_driver_unregister(&pdm_device_platform_driver);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Device PLATFORM Driver");
