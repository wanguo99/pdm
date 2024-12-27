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

enum pdm_led_command {
    PDM_LED_CMD_NULL            = 0x0,
    PDM_LED_CMD_SET_STATE       = 0x1,
    PDM_LED_CMD_GET_STATE       = 0x2,
    PDM_LED_CMD_SET_BRIGHTNESS  = 0x3,
    PDM_LED_CMD_GET_BRIGHTNESS  = 0x4,
    PDM_LED_CMD_INVALID         = 0xFF
};

/**
 * @struct pdm_led_operations
 * @brief PDM LED Device Operations Structure
 *
 * This structure defines the operation functions for a PDM LED device, including setting
 * the LED state (on/off).
 */
struct pdm_led_operations {
    int (*set_state)(struct pdm_client *client, int state);
    int (*get_state)(struct pdm_client *client, int *state);
    int (*set_brightness)(struct pdm_client *client, int brightness);
    int (*get_brightness)(struct pdm_client *client, int *brightness);
};

/**
 * @struct pdm_led_priv
 * @brief PDM LED Device Private Data Structure
 *
 * This structure is used to store private data for a PDM LED device, including pointers to
 * operation functions.
 */
struct pdm_led_priv {
    const struct pdm_led_operations *ops;  ///< Pointer to operation function callbacks
};

/**
 * @brief Match data structure for initializing GPIO type LED devices.
 */
extern const struct pdm_client_match_data pdm_led_gpio_match_data;

/**
 * @brief Match data structure for initializing PWM type LED devices.
 */
extern const struct pdm_client_match_data pdm_led_pwm_match_data;



#endif /* _PDM_LED_PRIV_H_ */
