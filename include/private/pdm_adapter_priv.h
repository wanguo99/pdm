#ifndef _PDM_ADAPTER_PRIV_H_
#define _PDM_ADAPTER_PRIV_H_

/**
 * @brief Initializes the LED main device driver.
 *
 * Sets up and registers the LED main device driver.
 *
 * @return Returns 0 on success; negative error code on failure.
 */
int pdm_led_driver_init(void);

/**
 * @brief Exits the LED main device driver.
 *
 * Cleans up and unregisters the LED main device driver, releasing associated resources.
 */
void pdm_led_driver_exit(void);

/**
 * @brief Initializes the NVMEM PDM adapter driver.
 *
 * Allocates and registers the adapter and driver.
 *
 * @return Returns 0 on success; negative error code on failure.
 */
int pdm_nvmem_driver_init(void);

/**
 * @brief Exits the NVMEM PDM adapter driver.
 *
 * Unregisters the driver and adapter, releasing related resources.
 */
void pdm_nvmem_driver_exit(void);

#endif /* _PDM_ADAPTER_PRIV_H_ */
