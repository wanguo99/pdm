#ifndef _PDM_NVMEM_PRIV_H_
#define _PDM_NVMEM_PRIV_H_

/**
 * @file pdm_nvmem_priv.h
 * @brief PDM NVMEM Driver Private Interface
 *
 * This file defines the private structures and related functions for the PDM NVMEM driver,
 * used to manage and operate PDM NVMEM devices.
 */

#include "pdm.h"

/**
 * @def PDM_NVMEM_NAME
 * @brief Controller name
 */
#define PDM_NVMEM_NAME "pdm_nvmem"

enum pdm_nvmem_command {
	PDM_NVMEM_CMD_NULL	= 0x00,
	PDM_NVMEM_CMD_READ_REG	= 0x01,
	PDM_NVMEM_CMD_WRITE_REG	= 0x02,
	PDM_NVMEM_CMD_INVALID	= 0xFF
};

/**
 * @struct pdm_nvmem_priv
 * @brief PDM NVMEM Device Private Data Structure
 *
 * This structure is used to store private data for a PDM NVMEM device, including pointers to
 * operation functions.
 */
struct pdm_nvmem_priv {
	int (*read_reg)(struct pdm_client *client, unsigned int offset, void *val, size_t bytes);
	int (*write_reg)(struct pdm_client *client, unsigned int offset, void *val, size_t bytes);
};


/**
 * @brief Match data structure for initializing PWM type DIMMER devices.
 */
extern const struct pdm_client_match_data pdm_nvmem_spi_match_data;

#endif /* _PDM_NVMEM_PRIV_H_ */
