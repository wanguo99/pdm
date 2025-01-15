#ifndef _PDM_DEVICE_PRIV_H_
#define _PDM_DEVICE_PRIV_H_

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
