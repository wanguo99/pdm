#ifndef _PDM_SWITCH_PRIV_H_
#define _PDM_SWITCH_PRIV_H_

/**
 * @file pdm_switch_priv.h
 * @brief PDM SWITCH Driver Private Interface
 *
 * This file defines the private structures and related functions for the PDM SWITCH driver,
 * used to manage and operate PDM SWITCH devices.
 */

#include "pdm.h"

/**
 * @def PDM_SWITCH_NAME
 * @brief Controller name
 */
#define PDM_SWITCH_NAME "pdm_switch"

enum pdm_switch_command {
	PDM_SWITCH_CMD_NULL		= 0x0,
	PDM_SWITCH_CMD_SET_STATE		= 0x1,
	PDM_SWITCH_CMD_GET_STATE		= 0x2,
	PDM_SWITCH_CMD_SET_BRIGHTNESS	= 0x3,
	PDM_SWITCH_CMD_GET_BRIGHTNESS	= 0x4,
	PDM_SWITCH_CMD_INVALID		= 0xFF
};

/**
 * @struct pdm_switch_operations
 * @brief PDM SWITCH Device Operations Structure
 *
 * This structure defines the operation functions for a PDM SWITCH device, including setting
 * the SWITCH state (on/off).
 */
struct pdm_switch_operations {
	int (*set_state)(struct pdm_client *client, int state);
	int (*get_state)(struct pdm_client *client, int *state);
	int (*set_brightness)(struct pdm_client *client, int brightness);
	int (*get_brightness)(struct pdm_client *client, int *brightness);
};

/**
 * @struct pdm_switch_priv
 * @brief PDM SWITCH Device Private Data Structure
 *
 * This structure is used to store private data for a PDM SWITCH device, including pointers to
 * operation functions.
 */
struct pdm_switch_priv {
	bool origin_state;			///< Switch origin state
	const struct pdm_switch_operations *ops;	///< Pointer to operation function callbacks
};

/**
 * @brief Match data structure for initializing GPIO type SWITCH devices.
 */
extern const struct pdm_client_match_data pdm_switch_gpio_match_data;

/**
 * @brief Match data structure for initializing PWM type SWITCH devices.
 */
extern const struct pdm_client_match_data pdm_switch_pwm_match_data;



#endif /* _PDM_SWITCH_PRIV_H_ */
