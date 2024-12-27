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
    struct gpio_desc *gpiod;
    bool is_active_low;

    if (!client || !client->pdmdev) {
        OSA_ERROR("Invalid client\n");
        return -EINVAL;
    }

    gpiod = client->hardware.gpio.gpiod;
    is_active_low = gpiod_is_active_low(gpiod);
    if (is_active_low) {
        gpiod_set_value_cansleep(gpiod, !!state);
    } else {
        gpiod_set_value_cansleep(gpiod, !state);
    }

    OSA_INFO("GPIO PDM Led: Set %s state to %d\n", dev_name(&client->dev), state);
    return 0;
}

/**
 * @struct pdm_led_operations
 * @brief PDM LED device operations structure (GPIO version).
 *
 * This structure defines the operation functions for a PDM LED device using GPIO.
 */
static const struct pdm_led_operations pdm_led_ops_gpio = {
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
static int pdm_led_gpio_setup(struct pdm_client *client)
{
    struct pdm_led_priv *led_priv;
    struct device_node *np;
    struct gpio_desc *gpiod;
    const char *default_state;
    bool is_active_low;
    int status;

    if (!client) {
        OSA_ERROR("Invalid client\n");
    }

    led_priv = pdm_client_get_private_data(client);
    if (!led_priv) {
        OSA_ERROR("Get PDM Client DevData Failed\n");
        return -ENOMEM;
    }

    led_priv->ops = &pdm_led_ops_gpio;

    np = pdm_client_get_of_node(client);
    if (!np) {
        OSA_ERROR("No DT node found\n");
        return -EINVAL;
    }

    status = of_property_read_string(np, "default-state", &default_state);
    if (status) {
        OSA_INFO("No default-state property found, using defaults as off\n");
        default_state = "off";
    }

    gpiod = gpiod_get_index(client->pdmdev->dev.parent, NULL, 0, GPIOD_OUT_LOW);
    if (IS_ERR(gpiod)) {
        OSA_ERROR("Failed to get GPIO\n");
        return PTR_ERR(gpiod);
    }

    is_active_low = gpiod_is_active_low(gpiod);
    if (!strcmp(default_state, "on")) {
        gpiod_set_value_cansleep(gpiod, is_active_low ? 1 : 0);
    } else if (!strcmp(default_state, "off")) {
        gpiod_set_value_cansleep(gpiod, is_active_low ? 0 : 1);
    } else {
        OSA_INFO("Unknown default-state: %s, using off\n", default_state);
        gpiod_set_value_cansleep(gpiod, is_active_low ? 0 : 1);
    }

    client->hardware.gpio.gpiod = gpiod;

    OSA_DEBUG("GPIO LED Setup: %s\n", dev_name(&client->dev));

    return 0;
}

static void pdm_led_gpio_cleanup(struct pdm_client *client)
{
    if (client && !IS_ERR_OR_NULL(client->hardware.gpio.gpiod)) {
        OSA_DEBUG("GPIO LED Cleanup: %s\n", dev_name(&client->dev));
        gpiod_put(client->hardware.gpio.gpiod);
    }
}


/**
 * @brief Match data structure for initializing GPIO type LED devices.
 */
const struct pdm_client_match_data pdm_led_gpio_match_data = {
    .setup = pdm_led_gpio_setup,
    .cleanup = pdm_led_gpio_cleanup,
};

