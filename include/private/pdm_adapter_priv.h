#ifndef _PDM_ADAPTER_PRIV_H_
#define _PDM_ADAPTER_PRIV_H_

/**
 * @brief Registers all PDM adapter drivers.
 *
 * Initializes all PDM adapter drivers within the system.
 *
 * @return Returns 0 on success; negative error code on failure.
 */
int pdm_adapter_drivers_register(void);

/**
 * @brief Unregisters all PDM adapter drivers.
 *
 * Cleans up and unregisters all PDM adapter drivers from the system.
 */
void pdm_adapter_drivers_unregister(void);

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

#endif /* _PDM_ADAPTER_PRIV_H_ */
