#include <linux/of_gpio.h>
#include <linux/gpio.h>

#include "pdm.h"
#include "pdm_led_priv.h"

/**
 * @brief Sets the state of a GPIO LED.
 *
 * This function sets the state (on/off) of the specified PDM device's GPIO LED.
 *
 * @param client Pointer to the PDM client structure.
 * @param state LED state (0 for off, 1 for on).
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_led_gpio_set_state(struct pdm_client *client, int state)
{
    struct pdm_device_priv *pdmdev_priv;

    if (!client || !client->pdmdev) {
        OSA_ERROR("Invalid client\n");
        return -EINVAL;
    }

    if (state != 0 && state != 1) {
        OSA_ERROR("Invalid state: %d\n", state);
        return -EINVAL;
    }

    pdmdev_priv = pdm_device_get_drvdata(client->pdmdev);
    if (!pdmdev_priv) {
        OSA_ERROR("Get PDM Device drvdata Failed\n");
        return -ENOMEM;
    }
    gpiod_set_value(pdmdev_priv->hw_data.gpio.gpiod, state);

    OSA_INFO("GPIO PDM Led: Set %s state to %d\n", dev_name(&client->dev), state);
    return 0;
}

/**
 * @struct pdm_led_operations
 * @brief PDM LED device operations structure (GPIO version).
 *
 * This structure defines the operation functions for a PDM LED device using GPIO.
 */
static const struct pdm_led_operations pdm_device_led_ops_gpio = {
    .set_state = pdm_led_gpio_set_state,
};

/**
 * @brief Initializes GPIO settings for a PDM device.
 *
 * This function initializes the GPIO settings for the specified PDM device and sets up the operation functions.
 *
 * @param client Pointer to the PDM client structure.
 * @return Returns 0 on success; negative error code on failure.
 */
int pdm_led_gpio_setup(struct pdm_client *client)
{
    struct pdm_led_priv *led_priv;
    if (!client) {
        OSA_ERROR("Invalid client\n");
    }

    led_priv = pdm_client_get_drvdata(client);
    if (!led_priv) {
        OSA_ERROR("Get PDM Client DevData Failed\n");
        return -ENOMEM;
    }

    led_priv->ops = &pdm_device_led_ops_gpio;

    OSA_DEBUG("GPIO LED Setup: %s\n", dev_name(&client->dev));
    return 0;
}
