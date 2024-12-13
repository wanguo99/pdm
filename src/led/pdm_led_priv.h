#ifndef _PDM_LED_PRIV_H_
#define _PDM_LED_PRIV_H_

/**
 * @file pdm_led_priv.h
 * @brief PDM LED Driver Private Interface
 *
 * This file defines the private structures and related functions for the PDM LED driver,
 * used to manage and operate PDM LED devices.
 */

#include "pdm.h"

/**
 * @def PDM_LED_NAME
 * @brief Controller name
 */
#define PDM_LED_NAME "pdm_led"

/**
 * @def PDM_LED_COMPATIBLE_GPIO
 * @def PDM_LED_COMPATIBLE_PWM
 *
 * @brief Compatibility strings for different types of PDM LED devices.
 */
#define PDM_LED_COMPATIBLE_GPIO     "led,pdm-device-gpio"
#define PDM_LED_COMPATIBLE_PWM      "led,pdm-device-pwm"


/**
 * @struct pdm_led_operations
 * @brief PDM LED Device Operations Structure
 *
 * This structure defines the operation functions for a PDM LED device, including setting the LED state (on/off).
 */
struct pdm_led_operations {
    int (*set_state)(struct pdm_client *client, int state);  ///< Function to set the LED state
};

/**
 * @struct pdm_led_priv
 * @brief PDM LED Device Private Data Structure
 *
 * This structure is used to store private data for a PDM LED device, including pointers to operation functions.
 */
struct pdm_led_priv {
    const struct pdm_led_operations *ops;  ///< Pointer to operation function callbacks
};

/**
 * @brief Initializes GPIO settings for a PDM LED device.
 *
 * This function initializes the GPIO settings for the specified PDM LED device and sets up the operation functions.
 *
 * @param client Pointer to the PDM client structure.
 * @return Returns 0 on success; negative error code on failure.
 */
int pdm_led_gpio_setup(struct pdm_client *client);

/**
 * @brief Initializes PWM settings for a PDM LED device.
 *
 * This function initializes the PWM settings for the specified PDM LED device and sets up the operation functions.
 *
 * @param client Pointer to the PDM client structure.
 * @return Returns 0 on success; negative error code on failure.
 */
int pdm_led_pwm_setup(struct pdm_client *client);

#endif /* _PDM_LED_PRIV_H_ */
