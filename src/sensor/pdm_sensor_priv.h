#ifndef _PDM_SENSOR_PRIV_H_
#define _PDM_SENSOR_PRIV_H_

/**
 * @file pdm_sensor_priv.h
 * @brief PDM SENSOR Driver Private Interface
 *
 * This file defines the private structures and related functions for the PDM SENSOR driver,
 * used to manage and operate PDM SENSOR devices.
 */

#include "pdm.h"

/**
 * @def PDM_SENSOR_NAME
 * @brief Controller name
 */
#define PDM_SENSOR_NAME "pdm_sensor"

enum pdm_sensor_command {
	PDM_SENSOR_CMD_NULL	= 0x00,
	PDM_SENSOR_CMD_READ	= 0x01,
	PDM_SENSOR_CMD_WRITE	= 0x02,
	PDM_SENSOR_CMD_INVALID	= 0xFF
};

/**
 * @struct pdm_sensor_priv
 * @brief PDM SENSOR Device Private Data Structure
 *
 * This structure is used to store private data for a PDM SENSOR device, including pointers to
 * operation functions.
 */
struct pdm_sensor_priv {
	int (*read)(struct pdm_client *client, unsigned int type, unsigned int *val);
	int (*write)(struct pdm_client *client, unsigned int type, unsigned int val);
};

/**
 * @brief Match data structure for initializing PWM type DIMMER devices.
 */
extern const struct pdm_client_match_data pdm_sensor_ap3216c_match_data;

#endif /* _PDM_SENSOR_PRIV_H_ */
