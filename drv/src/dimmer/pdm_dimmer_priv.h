#ifndef _PDM_DIMMER_PRIV_H_
#define _PDM_DIMMER_PRIV_H_

/**
 * @file pdm_dimmer_priv.h
 * @brief PDM DIMMER Driver Private Interface
 *
 * This file defines the private structures and related functions for the PDM
  DIMMER driver, used to manage and operate PDM DIMMER devices.
 */

#include "pdm.h"

/**
 * @def PDM_DIMMER_NAME
 * @brief Controller name
 */
#define PDM_DIMMER_NAME "pdm_dimmer"

enum pdm_dimmer_command {
	PDM_DIMMER_CMD_NULL		= 0x00,
	PDM_DIMMER_CMD_SET_LEVEL	= 0x01,
	PDM_DIMMER_CMD_GET_LEVEL	= 0x02,
	PDM_DIMMER_CMD_INVALID		= 0xFF
};

#define PDM_DIMMER_MAX_LEVEL_VALUE	(0xFF)

/**
 * @struct pdm_dimmer_priv
 * @brief PDM DIMMER Device Private Data Structure
 *
 * This structure is used to store private data for a PDM DIMMER device, including pointers to
 * operation functions.
 */
struct pdm_dimmer_priv {
	unsigned int max_level;
	unsigned int *level_map;
	int (*set_level)(struct pdm_client *client, unsigned int level);
	int (*get_level)(struct pdm_client *client, unsigned int *level);
};

/**
 * @brief Match data structure for initializing PWM type DIMMER devices.
 */
extern const struct pdm_client_match_data pdm_dimmer_pwm_match_data;

#endif /* _PDM_DIMMER_PRIV_H_ */
