#ifndef _PDM_DEVICE_PRIV_H_
#define _PDM_DEVICE_PRIV_H_


/**
 * @struct pdm_device_match_data
 * @brief Match data structure for initializing specific types of PDM Devices.
 *
 * This structure contains setup and cleanup function pointers for initializing
 * and cleaning up specific types of PDM Devices.
 */
struct pdm_device_match_data {
    int (*setup)(struct pdm_device *client);
    void (*cleanup)(struct pdm_device *client);
};

/**
 * @brief Initializes the PDM device drivers.
 *
 * Registers the device class and device drivers for the PDM devices.
 *
 * @return Returns 0 on success; negative error code on failure.
 */
int pdm_device_drivers_register(void);

/**
 * @brief Unregisters the PDM device drivers.
 *
 * Cleans up and unregisters the device drivers for the PDM devices.
 */
void pdm_device_drivers_unregister(void);

/* I2C Driver Initialization and Cleanup */
/**
 * @brief Initializes the I2C driver.
 *
 * Registers the I2C driver with the system.
 *
 * @return Returns 0 on success; negative error code on failure.
 */
int pdm_device_i2c_driver_init(void);

/**
 * @brief Exits the I2C driver.
 *
 * Unregisters the I2C driver from the system.
 */
void pdm_device_i2c_driver_exit(void);

/* PLATFORM Driver Initialization and Cleanup */
/**
 * @brief Initializes the PLATFORM driver.
 *
 * Registers the PLATFORM driver with the system.
 *
 * @return Returns 0 on success; negative error code on failure.
 */
int pdm_device_platform_driver_init(void);

/**
 * @brief Exits the PLATFORM driver.
 *
 * Unregisters the PLATFORM driver from the system.
 */
void pdm_device_platform_driver_exit(void);

/* SPI Driver Initialization and Cleanup */
/**
 * @brief Initializes the SPI driver.
 *
 * Registers the SPI driver with the system.
 *
 * @return Returns 0 on success; negative error code on failure.
 */
int pdm_device_spi_driver_init(void);

/**
 * @brief Exits the SPI driver.
 *
 * Unregisters the SPI driver from the system.
 */
void pdm_device_spi_driver_exit(void);

#endif /* _PDM_DEVICE_PRIV_H_ */
