#include "pdm.h"

/**
 * @brief Matches a device based on its parent device.
 *
 * @param dev The device to check.
 * @param data The parent device to match against.
 * @return Returns 1 if the device's parent matches the given parent device; otherwise, returns 0.
 */
static int pdm_bus_device_match_parent(struct device *dev, const void *data)
{
	struct pdm_device *pdmdev = dev_to_pdm_device(dev);
	struct device *parent = (struct device *)data;
	return (pdmdev->dev.parent == parent) ? 1 : 0;
}

/**
 * @brief Finds a device on the `pdm_bus_type` bus that matches the specified parent device.
 *
 * This function iterates over all devices on the `pdm_bus_type` bus and finds one whose parent device
 * matches the provided one.
 *
 * @param parent The parent device to match.
 * @return A pointer to the matching PDM device or NULL if no match is found.
 */
struct pdm_device *pdm_bus_find_device_by_parent(struct device *parent)
{
	struct device *dev = bus_find_device(&pdm_bus_type, NULL, parent, pdm_bus_device_match_parent);
	if (!dev) {
		return NULL;
	}
	return dev_to_pdm_device(dev);
}

/**
 * @brief Registers a PDM driver with the kernel.
 *
 * @param owner Pointer to the module that owns this driver.
 * @param driver Pointer to the PDM driver structure.
 * @return Returns 0 on success or a negative error code on failure.
 */
int pdm_bus_register_driver(struct module *owner, struct pdm_driver *driver)
{
	int status;

	if (!driver) {
		return -EINVAL;
	}

	driver->driver.owner = owner;
	driver->driver.bus = &pdm_bus_type;
	status = driver_register(&driver->driver);
	if (status) {
		OSA_ERROR("Failed to register driver [%s], error %d\n", driver->driver.name, status);
		return status;
	}
	return 0;
}

/**
 * @brief Unregisters a PDM driver from the kernel.
 *
 * @param driver Pointer to the PDM driver structure.
 */
void pdm_bus_unregister_driver(struct pdm_driver *driver)
{
	if (!driver) {
		return;
	}
	driver_unregister(&driver->driver);
}

/**
 * @brief Probes a PDM device.
 *
 * This function handles the probing of a PDM device.
 *
 * @param dev Pointer to the device.
 * @return Returns 0 on success or a negative error code on failure.
 */
static int pdm_bus_device_probe(struct device *dev)
{
	struct pdm_device *pdmdev;
	struct pdm_driver *pdmdrv;

	if (!dev) {
		return -EINVAL;
	}

	pdmdev = dev_to_pdm_device(dev);
	pdmdrv = drv_to_pdm_driver(dev->driver);
	if (pdmdev && pdmdrv && pdmdrv->probe) {
		return pdmdrv->probe(pdmdev);
	}

	OSA_WARN("Driver or device not found or probe function not available\n");
	return -ENODEV;
}

/**
 * @brief Removes a PDM device.
 *
 * This function handles the removal of a PDM device.
 *
 * @param dev Pointer to the device.
 */
static void pdm_bus_device_remove(struct device *dev)
{
	struct pdm_device *pdmdev;
	struct pdm_driver *pdmdrv;

	if (!dev) {
		return;
	}

	pdmdev = dev_to_pdm_device(dev);
	pdmdrv = drv_to_pdm_driver(dev->driver);
	if (pdmdev && pdmdrv && pdmdrv->remove) {
		pdmdrv->remove(pdmdev);
	}
}

/**
 * @brief Matches a PDM device with a driver.
 *
 * This function attempts to match a PDM device with a driver using an OF-style match.
 *
 * @param dev Pointer to the device.
 * @param drv Pointer to the driver.
 * @return Returns 1 if a match is found; otherwise, returns 0.
 */
static int pdm_bus_device_real_match(struct device *dev, const struct device_driver *drv)
{
	/* Attempt an OF style match */
	if (of_driver_match_device(dev->parent, drv)) {
		return 1;
	}
	return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 10, 0)
static int pdm_bus_device_match(struct device *dev, struct device_driver *drv) {
	return pdm_bus_device_real_match(dev, (const struct device_driver *)drv);
}
#else
static int pdm_bus_device_match(struct device *dev, const struct device_driver *drv) {
	return pdm_bus_device_real_match(dev, drv);
}
#endif

/**
 * @brief Definition of the PDM bus type.
 *
 * This structure defines the basic information and operation functions of the PDM bus.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 2, 0)
struct bus_type pdm_bus_type = {
#else
const struct bus_type pdm_bus_type = {
#endif
	.name   = "pdm",
	.probe  = pdm_bus_device_probe,
	.remove = pdm_bus_device_remove,
	.match  = pdm_bus_device_match,
};

/**
 * @brief Initializes the PDM bus.
 *
 * This function registers the PDM bus type so it can be used within the kernel. It performs the
 * following actions:
 * - Registers the PDM bus type.
 * - Initializes relevant data structures for the PDM bus instance.
 *
 * @return Returns 0 on success or a negative error code on failure.
 */
int pdm_bus_init(void)
{
	int status;

	status = bus_register(&pdm_bus_type);
	if (status < 0) {
		OSA_ERROR("Failed to register PDM bus, error %d\n", status);
		return status;
	}

	OSA_DEBUG("PDM bus initialized\n");
	return 0;
}

/**
 * @brief Exits the PDM bus.
 *
 * This function unregisters the PDM bus type so it is no longer used within the kernel. It performs
 * the following actions:
 * - Unregisters the PDM bus type.
 *
 * @note Ensure all related devices have been unregistered before calling this function.
 */
void pdm_bus_exit(void)
{
	bus_unregister(&pdm_bus_type);
	OSA_DEBUG("PDM bus unregistered\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM BUS Module");
