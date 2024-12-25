#ifndef _PDM_BUS_H_
#define _PDM_BUS_H_

/**
 * @file pdm.h
 * @brief Public interface definitions for the PDM module.
 *
 * This file provides public interface declarations for the PDM module,
 * including data structures and function prototypes.
 */

#include <linux/debugfs.h>
#include <linux/proc_fs.h>
#include <linux/of_device.h>

/**
 * @struct pdm_device_id
 * @brief Device ID information for PDM devices.
 *
 * Stores metadata related to matching PDM devices, such as compatibility
 * strings and driver private data.
 */
struct pdm_device_id {
    char compatible[PDM_DEVICE_NAME_SIZE];     /**< Compatibility string: Used for driver matching */
    kernel_ulong_t driver_data;                /**< Driver private data: Extra info passed to the driver */
};

/**
 * @struct pdm_driver
 * @brief Definition of a PDM driver.
 *
 * Describes basic properties of a PDM driver, including the table of supported
 * device IDs and probe/remove callback functions.
 */
struct pdm_driver {
    struct device_driver driver;               /**< Kernel device driver structure */
    const struct pdm_device_id *id_table;      /**< Device ID table: Lists all supported devices */
    int (*probe)(struct pdm_device *dev);      /**< Probe callback: Called when a matching device is found */
    void (*remove)(struct pdm_device *dev);    /**< Remove callback: Called when a device is unloaded */
};

/**
 * @fn struct pdm_device *pdm_bus_find_device_by_parent(struct device *parent)
 * @brief Finds a PDM device by its parent device.
 *
 * Searches for a device instance on the PDM bus with the specified parent device.
 *
 * @param[in] parent Pointer to the parent device.
 * @return Pointer to the matching PDM device on success, NULL if not found.
 */
struct pdm_device *pdm_bus_find_device_by_parent(struct device *parent);

/**
 * @fn int pdm_bus_for_each_dev(void *data, int (*fn)(struct device *dev, void *data))
 * @brief Iterates over all devices on the PDM bus.
 *
 * Traverses all devices on the PDM bus and calls the specified callback function
 * for each device.
 *
 * @param[in] data Data to pass to the callback function.
 * @param[in] fn Callback function pointer to handle each device.
 * @return Result of traversal, 0 on success, non-zero on failure.
 */
int pdm_bus_for_each_dev(void *data, int (*fn)(struct device *dev, void *data));

/**
 * @fn int pdm_bus_register_driver(struct module *owner, struct pdm_driver *driver)
 * @brief Registers a new PDM driver.
 *
 * Adds the given PDM driver to the kernel and enables it to respond to relevant
 * device events.
 *
 * @param[in] owner Module owning this driver.
 * @param[in] driver Pointer to the PDM driver structure to register.
 * @return 0 on success, negative error code on failure.
 */
int pdm_bus_register_driver(struct module *owner, struct pdm_driver *driver);

/**
 * @fn void pdm_bus_unregister_driver(struct pdm_driver *driver)
 * @brief Unregisters a PDM driver from the kernel.
 *
 * Removes a registered PDM driver, preventing it from handling any new device
 * events.
 *
 * @param[in] driver Pointer to the PDM driver structure to unregister.
 */
void pdm_bus_unregister_driver(struct pdm_driver *driver);

/**
 * @def drv_to_pdm_driver(__drv)
 * @brief Converts a `device_driver` structure to a `pdm_driver` structure.
 *
 * Macro to convert a `device_driver` structure to a `pdm_driver` structure.
 *
 * @param __drv Pointer to a `device_driver` structure.
 * @return Pointer to a `pdm_driver` structure.
 */
#define drv_to_pdm_driver(__drv) container_of(__drv, struct pdm_driver, driver)

/**
 * @fn int pdm_bus_init(void)
 * @brief Initializes the PDM bus.
 *
 * Registers the PDM bus type so it can be used within the kernel. It initializes
 * related data structures of the PDM bus instance.
 *
 * @return 0 on success, negative error code on failure.
 */
int pdm_bus_init(void);

/**
 * @fn void pdm_bus_exit(void)
 * @brief Unregisters the PDM bus.
 *
 * Unregisters the PDM bus type, making it unavailable in the kernel.
 *
 * @note Ensure all related devices have been unregistered before calling this.
 */
void pdm_bus_exit(void);

/**
 * @var pdm_bus_type
 * @brief Defines the basic information and operation functions of the PDM bus.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 2, 0)
extern struct bus_type pdm_bus_type;
#else
extern const struct bus_type pdm_bus_type;
#endif

#endif /* _PDM_BUS_H_ */
