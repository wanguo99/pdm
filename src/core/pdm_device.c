#include "pdm.h"
#include "pdm_device.h"
#include "pdm_component.h"
#include "pdm_device_priv.h"

/**
 * @brief List to store all registered PDM device drivers.
 */
static LIST_HEAD(pdm_device_driver_list);

/**
 * @brief Array of PDM device drivers.
 *
 * Each `pdm_component` structure contains the driver's name, init and exit functions.
 */
static struct pdm_component pdm_device_drivers[] = {
    { .name = "SPI PDM Device", .status = true, .ignore_failures = true,
      .init = pdm_device_spi_driver_init, .exit = pdm_device_spi_driver_exit },
    { .name = "I2C PDM Device", .status = true, .ignore_failures = true,
      .init = pdm_device_i2c_driver_init, .exit = pdm_device_i2c_driver_exit },
    { .name = "PLATFORM PDM Device", .status = true, .ignore_failures = true,
      .init = pdm_device_platform_driver_init, .exit = pdm_device_platform_driver_exit },
};

/**
 * @brief Registers PDM device drivers.
 *
 * Initializes the PDM device driver list and registers the drivers.
 *
 * @return 0 on success, negative error code on failure.
 */
int pdm_device_drivers_register(void)
{
    struct pdm_component_params params;
    int status;

    INIT_LIST_HEAD(&pdm_device_driver_list);

    params.components = pdm_device_drivers;
    params.count = ARRAY_SIZE(pdm_device_drivers);
    params.list = &pdm_device_driver_list;
    status = pdm_component_register(&params);
    if (status < 0) {
        OSA_ERROR("Failed to register PDM Device Drivers, error: %d.\n", status);
        return status;
    }

    OSA_DEBUG("PDM Device Drivers initialized successfully.\n");
    return 0;
}

/**
 * @brief Unregisters PDM device drivers.
 *
 * Cleans up and unregisters the PDM device drivers.
 */
void pdm_device_drivers_unregister(void)
{
    pdm_component_unregister(&pdm_device_driver_list);
    OSA_DEBUG("PDM Device Drivers exited successfully.\n");
}

/**
 * @brief Validates a PDM device.
 *
 * Checks if the provided PDM device pointer is valid.
 *
 * @param pdmdev Pointer to the PDM device structure.
 * @return 0 on success, -EINVAL on failure.
 */
static int pdm_device_verify(struct pdm_device *pdmdev)
{
    if (!pdmdev || !pdmdev->dev.parent) {
        OSA_ERROR("%s is null\n", !pdmdev ? "pdmdev" : "parent");
        return -EINVAL;
    }
    return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
static int pdm_device_uevent(struct device *dev, struct kobj_uevent_env *env)
#else
static int pdm_device_uevent(const struct device *dev, struct kobj_uevent_env *env)
#endif
{
    struct pdm_device *pdmdev = dev_to_pdm_device(dev);
    if (pdm_device_verify(pdmdev)) {
        return -EINVAL;
    }

    OSA_DEBUG("Generating MODALIAS for device %s\n", dev_name(dev));
    return add_uevent_var(env, "MODALIAS=pdm:%04X", pdmdev->id);
}

/**
 * @brief Shows the compatible string for the device.
 *
 * @param dev Pointer to the device structure.
 * @param da Pointer to the device attribute structure.
 * @param buf Output buffer.
 * @return Number of bytes written or -EINVAL on failure.
 */
static ssize_t compatible_show(struct device *dev, struct device_attribute *da, char *buf)
{
    struct pdm_device *pdmdev = dev_to_pdm_device(dev);
    if (pdm_device_verify(pdmdev)) {
        return -EINVAL;
    }

    OSA_INFO("Showing compatible string for device %s\n", dev_name(dev));
    return 0;

    // TODO: Implement compatible parsing logic.
    // return sysfs_emit(buf, "%s\n", pdmdev->physical_info.compatible);
}
static DEVICE_ATTR_RO(compatible);

/**
 * @brief Attribute array for PDM devices.
 */
static struct attribute *pdm_device_attrs[] = {
    &dev_attr_compatible.attr,
    NULL,
};
ATTRIBUTE_GROUPS(pdm_device);

const struct device_type pdm_device_type = {
    .name   = "pdm_device",
    .groups = pdm_device_groups,
    .uevent = pdm_device_uevent,
};

/**
 * @brief Releases resources associated with a PDM device.
 *
 * Frees the memory allocated for the PDM device structure.
 *
 * @param dev Pointer to the device structure.
 */
static void pdm_device_release(struct device *dev)
{
    struct pdm_device *pdmdev = dev_to_pdm_device(dev);
    if (pdmdev) {
        OSA_DEBUG("PDM Device Released: %s\n", dev_name(dev));
        kfree(pdmdev);
    }
}

/**
 * @brief Allocates a new PDM device structure.
 *
 * @return Pointer to the allocated PDM device structure, or NULL on failure.
 */
struct pdm_device *pdm_device_alloc(void)
{
    struct pdm_device *pdmdev;
    int status;

    pdmdev = kzalloc(sizeof(*pdmdev), GFP_KERNEL);
    if (!pdmdev) {
        OSA_ERROR("Failed to allocate memory for PDM device.\n");
        return NULL;
    }

    status = pdm_bus_device_id_alloc(pdmdev);
    if (status) {
        OSA_ERROR("ID allocation failed, status %d\n", status);
        kfree(pdmdev);
        return NULL;
    }

    device_initialize(&pdmdev->dev);
    pdmdev->dev.bus = &pdm_bus_type;
    pdmdev->dev.type = &pdm_device_type;
    pdmdev->dev.release = pdm_device_release;

    return pdmdev;
}

/**
 * @brief Frees a PDM device structure.
 *
 * Decrements the device reference count; when it reaches zero, the device is freed.
 *
 * @param pdmdev Pointer to the PDM device structure.
 */
void pdm_device_free(struct pdm_device *pdmdev)
{
    if (pdmdev) {
        pdm_bus_device_id_free(pdmdev);
        put_device(&pdmdev->dev);
    }
}
/**
 * @brief Registers a PDM device.
 *
 * Verifies the device, allocates an ID, checks for duplicates, sets the device name,
 * and adds the device to the system.
 *
 * @param pdmdev Pointer to the PDM device structure.
 * @return 0 on success, negative error code on failure.
 */
int pdm_device_register(struct pdm_device *pdmdev)
{
    int status;

    if (pdm_device_verify(pdmdev))
        return -EINVAL;

    if (pdm_bus_find_device_by_parent(pdmdev->dev.parent)) {
        OSA_ERROR("Device with parent %s already exists\n", dev_name(pdmdev->dev.parent));
        status = -EEXIST;
        goto err_out;
    }

    dev_set_name(&pdmdev->dev, "pdmdev-%d", pdmdev->id);
    status = device_add(&pdmdev->dev);
    if (status) {
        OSA_ERROR("Failed to add device %s, error: %d\n", dev_name(&pdmdev->dev), status);
        goto err_free_id;
    }

    OSA_DEBUG("Device %s registered successfully.\n", dev_name(&pdmdev->dev));
    return 0;

err_free_id:
    pdm_bus_device_id_free(pdmdev);
err_out:
    return status;
}

/**
 * @brief Unregisters a PDM device.
 *
 * Removes the device from the system and frees its ID.
 *
 * @param pdmdev Pointer to the PDM device structure.
 */
void pdm_device_unregister(struct pdm_device *pdmdev)
{
    if (!pdmdev) {
        OSA_ERROR("Invalid PDM device pointer.\n");
        return;
    }

    OSA_DEBUG("Unregistering device %s.\n", dev_name(&pdmdev->dev));
    device_unregister(&pdmdev->dev);
}

/**
 * @brief Initializes the PDM device module.
 *
 * Registers the device class and device drivers.
 *
 * @return 0 on success, negative error code on failure.
 */
int pdm_device_init(void)
{
    int status;

    status = pdm_device_drivers_register();
    if (status < 0) {
        OSA_ERROR("Failed to register PDM Device Drivers, error: %d\n", status);
        return status;
    }

    OSA_INFO("PDM Device module initialized successfully.\n");
    return 0;
}

/**
 * @brief Exits the PDM device module.
 *
 * Unregisters the device drivers and cleans up resources.
 */
void pdm_device_exit(void)
{
    pdm_device_drivers_unregister();
    OSA_INFO("PDM Device module exited successfully.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Device Module");
