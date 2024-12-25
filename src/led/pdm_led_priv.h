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
 * @struct pdm_led_gpio_data
 * @brief Data structure for GPIO-controlled LED devices.
 *
 * This structure holds the necessary data for controlling an LED via GPIO.
 */
struct pdm_led_gpio_data {
    int gpio;
};

/**
 * @struct pdm_led_pwm_data
 * @brief Data structure for PWM-controlled LED devices.
 *
 * This structure holds the necessary data for controlling an LED via PWM.
 */
struct pdm_led_pwm_data {
    struct pwm_device *pwm;
};

/**
* @union pdm_led_hw_data
 * @brief Union to hold hardware-specific data for different types of LED controls.
 *
 * This union allows the same structure to accommodate different types of LED control
 * mechanisms (e.g., GPIO or PWM).
 */
union pdm_led_hw_data {
    struct pdm_led_gpio_data gpio;
    struct pdm_led_pwm_data pwm;
};

/**
 * @struct pdm_led_match_data
 * @brief Match data structure for initializing specific types of LED devices.
 *
 * This structure contains setup and cleanup function pointers for initializing
 * and cleaning up specific types of LED devices.
 */
struct pdm_led_match_data {
    int (*setup)(struct pdm_client *client);
    void (*cleanup)(struct pdm_client *client);
};

/**
 * @struct pdm_led_operations
 * @brief PDM LED Device Operations Structure
 *
 * This structure defines the operation functions for a PDM LED device, including setting
 * the LED state (on/off).
 */
struct pdm_led_operations {
    int (*set_state)(struct pdm_client *client, int state);  ///< Function to set the LED state
};

/**
 * @struct pdm_led_priv
 * @brief PDM LED Device Private Data Structure
 *
 * This structure is used to store private data for a PDM LED device, including pointers to
 * operation functions.
 */
struct pdm_led_priv {
    union pdm_led_hw_data hw_data;
    const struct pdm_led_operations *ops;  ///< Pointer to operation function callbacks
    const struct pdm_led_match_data *match_data;
};

/**
 * @brief Initializes GPIO settings for a PDM LED device.
 *
 * This function initializes the GPIO settings for the specified PDM LED device and sets up the operation functions.
 *
 * @param client Pointer to the PDM client structure representing the LED device.
 * @return Returns 0 on success; negative error code on failure.
 */
int pdm_led_gpio_setup(struct pdm_client *client);

/**
 * @brief Cleans up GPIO settings for a PDM LED device.
 *
 * This function cleans up resources associated with GPIO-controlled LED devices.
 *
 * @param client Pointer to the PDM client structure representing the LED device.
 * @return Returns 0 on success; negative error code on failure.
 */
int pdm_led_gpio_cleanup(struct pdm_client *client);

/**
 * @brief Initializes PWM settings for a PDM LED device.
 *
 * This function initializes the PWM settings for the specified PDM LED device and sets up the operation functions.
 *
 * @param client Pointer to the PDM client structure.
 * @return Returns 0 on success; negative error code on failure.
 */
int pdm_led_pwm_setup(struct pdm_client *client);

/**
 * @brief Cleans up PWM settings for a PDM LED device.
 *
 * This function cleans up resources associated with PWM-controlled LED devices.
 *
 * @param client Pointer to the PDM client structure representing the LED device.
 * @return Returns 0 on success; negative error code on failure.
 */
int pdm_led_pwm_cleanup(struct pdm_client *client);

#endif /* _PDM_LED_PRIV_H_ */
