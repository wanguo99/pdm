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
    struct pdm_led_priv *led_priv;

    if (!client) {
        OSA_ERROR("Invalid client\n");
        return -EINVAL;
    }

    if (state != 0 && state != 1) {
        OSA_ERROR("Invalid state: %d\n", state);
        return -EINVAL;
    }

    led_priv = client->priv_data;
    if (!led_priv) {
        OSA_ERROR("Get PDM Client DevData Failed\n");
        return -ENOMEM;
    }

    gpio_set_value(led_priv->hw_data.gpio.gpio_num, state);
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
    struct device_node *np;
    struct pdm_led_priv *led_priv;
    unsigned int gpio_num;
    int status;

    if (!client) {
        OSA_ERROR("Invalid client\n");
    }

    led_priv = client->priv_data;
    if (!led_priv) {
        OSA_ERROR("Get PDM Client DevData Failed\n");
        return -ENOMEM;
    }

    np = pdm_device_get_of_node(client->pdmdev);
    if (!np) {
        OSA_ERROR("No DT node found\n");
        return -EINVAL;
    }

    gpio_num = of_get_named_gpio(np, "gpios", 0);
    if (!gpio_is_valid(gpio_num)) {
        OSA_ERROR("Invalid GPIO specified in DT\n");
        return gpio_num;
    }

    status = devm_gpio_request_one(client->pdmdev->dev.parent, gpio_num,
                                    GPIOF_OUT_INIT_LOW, dev_name(&client->dev));
    if (status) {
        OSA_ERROR("Failed to request GPIO %d\n", gpio_num);
        return status;
    }

    led_priv->hw_data.gpio.gpio_num = gpio_num;
    led_priv->ops = &pdm_device_led_ops_gpio;

    OSA_DEBUG("GPIO LED Setup: %s\n", dev_name(&client->dev));
    return 0;
}
