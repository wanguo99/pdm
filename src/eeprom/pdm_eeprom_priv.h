#ifndef _PDM_EEPROM_PRIV_H_
#define _PDM_EEPROM_PRIV_H_

/**
 * @file pdm_eeprom_priv.h
 * @brief PDM EEPROM Driver Private Interface
 *
 * This file defines the private structures and related functions for the PDM EEPROM driver,
 * used to manage and operate PDM EEPROM devices.
 */

#include "pdm.h"

/**
 * @def PDM_EEPROM_NAME
 * @brief Controller name
 */
#define PDM_EEPROM_NAME "pdm_eeprom"

enum pdm_eeprom_command {
    PDM_EEPROM_CMD_NULL            = 0x0,
    PDM_EEPROM_CMD_READ_REG        = 0x1,
    PDM_EEPROM_CMD_WRITE_REG       = 0x2,
    PDM_EEPROM_CMD_INVALID         = 0xFF
};

/**
 * @struct pdm_eeprom_match_data
 * @brief Match data structure for initializing specific types of EEPROM devices.
 *
 * This structure contains setup and cleanup function pointers for initializing
 * and cleaning up specific types of EEPROM devices.
 */
struct pdm_eeprom_match_data {
    int (*setup)(struct pdm_client *client);
    void (*cleanup)(struct pdm_client *client);
};

/**
 * @struct pdm_eeprom_operations
 * @brief PDM EEPROM Device Operations Structure
 *
 * This structure defines the operation functions for a PDM EEPROM device, including setting
 * the EEPROM state (on/off).
 */
struct pdm_eeprom_operations {
    int (*read_reg)(struct pdm_client *client, unsigned char addr, unsigned char *value);
    int (*write_reg)(struct pdm_client *client, unsigned char addr, unsigned char value);
};

/**
 * @struct pdm_eeprom_priv
 * @brief PDM EEPROM Device Private Data Structure
 *
 * This structure is used to store private data for a PDM EEPROM device, including pointers to
 * operation functions.
 */
struct pdm_eeprom_priv {
    const struct pdm_eeprom_operations *ops;  ///< Pointer to operation function callbacks
    const struct pdm_eeprom_match_data *match_data;
    void *private_data;
};

/**
 * @brief Initializes GPIO settings for a PDM EEPROM device.
 *
 * This function initializes the GPIO settings for the specified PDM EEPROM device and sets up the operation functions.
 *
 * @param client Pointer to the PDM client structure representing the EEPROM device.
 * @return Returns 0 on success; negative error code on failure.
 */
int pdm_eeprom_spi_setup(struct pdm_client *client);

/**
 * @brief Cleans up GPIO settings for a PDM EEPROM device.
 *
 * This function cleans up resources associated with GPIO-controlled EEPROM devices.
 *
 * @param client Pointer to the PDM client structure representing the EEPROM device.
 * @return Returns 0 on success; negative error code on failure.
 */
int pdm_eeprom_spi_cleanup(struct pdm_client *client);

#endif /* _PDM_EEPROM_PRIV_H_ */
