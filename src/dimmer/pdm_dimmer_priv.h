#ifndef _PDM_DIMMER_PRIV_H_
#define _PDM_DIMMER_PRIV_H_

/**
 * @file pdm_dimmer_priv.h
 * @brief PDM DIMMER Driver Private Interface
 *
 * This file defines the private structures and related functions for the PDM DIMMER driver,
 * used to manage and operate PDM DIMMER devices.
 */

#include "pdm.h"

/**
 * @def PDM_DIMMER_NAME
 * @brief Controller name
 */
#define PDM_DIMMER_NAME "pdm_dimmer"

enum pdm_dimmer_command {
	PDM_DIMMER_CMD_NULL		= 0x0,
	PDM_DIMMER_CMD_SET_STATE		= 0x1,
	PDM_DIMMER_CMD_GET_STATE		= 0x2,
	PDM_DIMMER_CMD_SET_BRIGHTNESS	= 0x3,
	PDM_DIMMER_CMD_GET_BRIGHTNESS	= 0x4,
	PDM_DIMMER_CMD_INVALID		= 0xFF
};

/**
 * @struct pdm_dimmer_operations
 * @brief PDM DIMMER Device Operations Structure
 *
 * This structure defines the operation functions for a PDM DIMMER device, including setting
 * the DIMMER state (on/off).
 */
struct pdm_dimmer_operations {
	int (*set_state)(struct pdm_client *client, int state);
	int (*get_state)(struct pdm_client *client, int *state);
	int (*set_brightness)(struct pdm_client *client, int brightness);
	int (*get_brightness)(struct pdm_client *client, int *brightness);
};

/**
 * @struct pdm_dimmer_priv
 * @brief PDM DIMMER Device Private Data Structure
 *
 * This structure is used to store private data for a PDM DIMMER device, including pointers to
 * operation functions.
 */
struct pdm_dimmer_priv {
	bool origin_state;			///< Dimmer origin state
	const struct pdm_dimmer_operations *ops;	///< Pointer to operation function callbacks
};

/**
 * @brief Match data structure for initializing GPIO type DIMMER devices.
 */
extern const struct pdm_client_match_data pdm_dimmer_gpio_match_data;

/**
 * @brief Match data structure for initializing PWM type DIMMER devices.
 */
extern const struct pdm_client_match_data pdm_dimmer_pwm_match_data;



#endif /* _PDM_DIMMER_PRIV_H_ */
