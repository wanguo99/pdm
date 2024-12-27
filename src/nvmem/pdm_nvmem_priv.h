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
    PDM_NVMEM_CMD_NULL            = 0x0,
    PDM_NVMEM_CMD_READ_REG        = 0x1,
    PDM_NVMEM_CMD_WRITE_REG       = 0x2,
    PDM_NVMEM_CMD_INVALID         = 0xFF
};

/**
 * @struct pdm_nvmem_match_data
 * @brief Match data structure for initializing specific types of NVMEM devices.
 *
 * This structure contains setup and cleanup function pointers for initializing
 * and cleaning up specific types of NVMEM devices.
 */
struct pdm_nvmem_match_data {
    int (*setup)(struct pdm_client *client);
    void (*cleanup)(struct pdm_client *client);
};

/**
 * @struct pdm_nvmem_operations
 * @brief PDM NVMEM Device Operations Structure
 *
 * This structure defines the operation functions for a PDM NVMEM device, including setting
 * the NVMEM state (on/off).
 */
struct pdm_nvmem_operations {
    int (*read_reg)(struct pdm_client *client, unsigned char addr, unsigned char *value);
    int (*write_reg)(struct pdm_client *client, unsigned char addr, unsigned char value);
};

/**
 * @struct pdm_nvmem_priv
 * @brief PDM NVMEM Device Private Data Structure
 *
 * This structure is used to store private data for a PDM NVMEM device, including pointers to
 * operation functions.
 */
struct pdm_nvmem_priv {
    const struct pdm_nvmem_operations *ops;  ///< Pointer to operation function callbacks
    const struct pdm_nvmem_match_data *match_data;
    void *private_data;
};

/**
 * @brief Initializes GPIO settings for a PDM NVMEM device.
 *
 * This function initializes the GPIO settings for the specified PDM NVMEM device and sets up the operation functions.
 *
 * @param client Pointer to the PDM client structure representing the NVMEM device.
 * @return Returns 0 on success; negative error code on failure.
 */
int pdm_nvmem_spi_setup(struct pdm_client *client);

/**
 * @brief Cleans up GPIO settings for a PDM NVMEM device.
 *
 * This function cleans up resources associated with GPIO-controlled NVMEM devices.
 *
 * @param client Pointer to the PDM client structure representing the NVMEM device.
 * @return Returns 0 on success; negative error code on failure.
 */
int pdm_nvmem_spi_cleanup(struct pdm_client *client);

#endif /* _PDM_NVMEM_PRIV_H_ */
