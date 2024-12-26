#include <linux/of_gpio.h>
#include <linux/gpio.h>

#include "pdm.h"
#include "pdm_eeprom_priv.h"


static int pdm_eeprom_spi_read_reg(struct pdm_client *client, unsigned char addr, unsigned char *value)
{
    OSA_INFO("SPI PDM EEPROM Read: %s [0x%x]\n", dev_name(&client->dev), addr);
    return 0;
}

static int pdm_eeprom_spi_write_reg(struct pdm_client *client, unsigned char addr, unsigned char value)
{
    OSA_INFO("SPI PDM EEPROM Write %s [0x%x] to 0x%x\n", dev_name(&client->dev), addr, value);
    return 0;
}

/**
 * @struct pdm_eeprom_operations
 * @brief PDM EEPROM device operations structure (GPIO version).
 *
 * This structure defines the operation functions for a PDM EEPROM device using GPIO.
 */
static const struct pdm_eeprom_operations pdm_device_eeprom_ops_spi = {
    .read_reg = pdm_eeprom_spi_read_reg,
    .write_reg = pdm_eeprom_spi_write_reg,
};

/**
 * @brief Initializes GPIO settings for a PDM device.
 *
 * This function initializes the GPIO settings for the specified PDM device and sets up the operation functions.
 *
 * @param client Pointer to the PDM client structure.
 * @return Returns 0 on success; negative error code on failure.
 */
int pdm_eeprom_spi_setup(struct pdm_client *client)
{
    struct pdm_eeprom_priv *eeprom_priv;
    if (!client) {
        OSA_ERROR("Invalid client\n");
    }

    eeprom_priv = pdm_client_get_private_data(client);
    if (!eeprom_priv) {
        OSA_ERROR("Get PDM Client DevData Failed\n");
        return -ENOMEM;
    }

    eeprom_priv->ops = &pdm_device_eeprom_ops_spi;

    OSA_DEBUG("GPIO EEPROM Setup: %s\n", dev_name(&client->dev));
    return 0;
}
