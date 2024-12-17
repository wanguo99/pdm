#include <linux/i2c.h>

#include "pdm.h"
#include "pdm_device_priv.h"

/**
 * @brief Compatibility definition for i2c_device_id structure.
 *
 * This structure is used for compatibility with Linux kernel versions below 2.6.25.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 25)
struct i2c_device_id {
    char name[I2C_NAME_SIZE];
    kernel_ulong_t driver_data;
};
#endif

/**
 * @brief Probes the I2C device and initializes the PDM device.
 *
 * This function handles the probing of an I2C device, allocating a PDM device and registering it.
 *
 * @param client Pointer to the I2C client structure.
 * @param id Pointer to the I2C device ID structure.
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_device_i2c_real_probe(struct i2c_client *client, const struct i2c_device_id *id) {
    struct pdm_device *pdmdev;
    int status;

    pdmdev = pdm_device_alloc(&client->dev);
    if (IS_ERR(pdmdev)) {
        OSA_ERROR("Failed to allocate pdm_device.\n");
        return PTR_ERR(pdmdev);
    }

    status = pdm_device_register(pdmdev);
    if (status) {
        OSA_ERROR("Failed to register pdm device, status=%d.\n", status);
        return status;
    }

    OSA_DEBUG("PDM I2C Device Probed.\n");
    return 0;
}

/**
 * @brief Removes the I2C device and unregisters the PDM device.
 *
 * This function handles the removal of an I2C device, unregistering and freeing the PDM device.
 *
 * @param client Pointer to the I2C client structure.
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_device_i2c_real_remove(struct i2c_client *client) {
    struct pdm_device *pdmdev = pdm_bus_find_device_by_parent(&client->dev);

    if (!pdmdev) {
        OSA_ERROR("Failed to find pdm device from bus.\n");
        return -ENODEV;
    }

    OSA_DEBUG("Found I2C PDM Device: %s\n", dev_name(&pdmdev->dev));

    pdm_device_unregister(pdmdev);

    OSA_DEBUG("PDM I2C Device Removed.\n");
    return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 3, 0)
/**
 * @brief Compatibility probe function for older kernel versions.
 *
 * This function is used for Linux kernel versions below 6.3.0.
 *
 * @param client Pointer to the I2C client structure.
 * @param id Pointer to the I2C device ID structure.
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_device_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id) {
    return pdm_device_i2c_real_probe(client, id);
}
#else
/**
 * @brief Probes the I2C device and initializes the PDM device.
 *
 * This function handles the probing of an I2C device, allocating a PDM device and registering it.
 *
 * @param client Pointer to the I2C client structure.
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_device_i2c_probe(struct i2c_client *client) {
    return pdm_device_i2c_real_probe(client, NULL);
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
/**
 * @brief Compatibility remove function for older kernel versions.
 *
 * This function is used for Linux kernel versions below 6.0.0.
 *
 * @param client Pointer to the I2C client structure.
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_device_i2c_remove(struct i2c_client *client) {
    return pdm_device_i2c_real_remove(client);
}
#else
/**
 * @brief Removes the I2C device and unregisters the PDM device.
 *
 * This function handles the removal of an I2C device, unregistering and freeing the PDM device.
 *
 * @param client Pointer to the I2C client structure.
 */
static void pdm_device_i2c_remove(struct i2c_client *client) {
    int status;

    status = pdm_device_i2c_real_remove(client);
    if (status) {
        OSA_ERROR("pdm_device_i2c_real_remove failed.\n");
    }
}
#endif

/**
 * @brief I2C device ID table.
 *
 * Defines the supported I2C device IDs.
 */
static const struct i2c_device_id pdm_device_i2c_id[] = {
    { "pdm-device-i2c", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, pdm_device_i2c_id);

/**
 * @brief DEVICE_TREE match table.
 *
 * Defines the supported DEVICE_TREE compatibility strings.
 */
static const struct of_device_id pdm_device_i2c_of_match[] = {
    { .compatible = "pdm-device-i2c" },
    { }
};
MODULE_DEVICE_TABLE(of, pdm_device_i2c_of_match);

/**
 * @brief I2C driver structure.
 *
 * Defines the basic information and operation functions of the I2C driver.
 */
static struct i2c_driver pdm_device_i2c_driver = {
    .driver = {
        .name = "pdm-device-i2c",
        .owner = THIS_MODULE,
        .of_match_table = pdm_device_i2c_of_match,
    },
    .probe = pdm_device_i2c_probe,
    .remove = pdm_device_i2c_remove,
    .id_table = pdm_device_i2c_id,
};

/**
 * @brief Initializes the I2C driver.
 *
 * Registers the I2C driver with the system.
 *
 * @return Returns 0 on success; negative error code on failure.
 */
int pdm_device_i2c_driver_init(void) {
    int status;

    status = i2c_add_driver(&pdm_device_i2c_driver);
    if (status) {
        OSA_ERROR("Failed to register PDM Device I2C Driver, status=%d.\n", status);
        return status;
    }
    OSA_DEBUG("PDM Device I2C Driver Initialized.\n");
    return 0;
}

/**
 * @brief Exits the I2C driver.
 *
 * Unregisters the I2C driver from the system.
 */
void pdm_device_i2c_driver_exit(void) {
    i2c_del_driver(&pdm_device_i2c_driver);
    OSA_DEBUG("PDM Device I2C Driver Exited.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Device I2C Driver.");
