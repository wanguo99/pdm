#include <linux/of_gpio.h>
#include <linux/gpio.h>

#include "pdm.h"
#include "pdm_eeprom_priv.h"

/**
 * @brief Sets the state of a GPIO EEPROM.
 *
 * This function sets the state (on/off) of the specified PDM device's GPIO EEPROM.
 *
 * @param client Pointer to the PDM client structure.
 * @param state EEPROM state (0 for off, 1 for on).
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_eeprom_gpio_set_state(struct pdm_client *client, int state)
{
    struct pdm_device_priv *pdmdev_priv;
    struct gpio_desc *gpiod;
    bool is_active_low;

    if (!client || !client->pdmdev) {
        OSA_ERROR("Invalid client\n");
        return -EINVAL;
    }

    pdmdev_priv = pdm_device_get_private_data(client->pdmdev);
    if (!pdmdev_priv) {
        OSA_ERROR("Get PDM Device drvdata Failed\n");
        return -ENOMEM;
    }

    gpiod = pdmdev_priv->hw_data.gpio.gpiod;
    is_active_low = gpiod_is_active_low(gpiod);
    if (is_active_low) {
        gpiod_set_value_cansleep(gpiod, !!state);
    } else {
        gpiod_set_value_cansleep(gpiod, !state);
    }

    OSA_INFO("SPI PDM EEPROM: Set %s state to %d\n", dev_name(&client->dev), state);
    return 0;
}

/**
 * @struct pdm_eeprom_operations
 * @brief PDM EEPROM device operations structure (GPIO version).
 *
 * This structure defines the operation functions for a PDM EEPROM device using GPIO.
 */
static const struct pdm_eeprom_operations pdm_device_eeprom_ops_gpio = {
    .set_state = pdm_eeprom_gpio_set_state,
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

    eeprom_priv->ops = &pdm_device_eeprom_ops_gpio;

    OSA_DEBUG("GPIO EEPROM Setup: %s\n", dev_name(&client->dev));
    return 0;
}
