#include <linux/of_gpio.h>
#include <linux/gpio.h>

#include "pdm.h"
#include "pdm_led_priv.h"


static bool pdm_led_gpio_level_to_state(struct gpio_desc *gpiod, int level)
{
	return gpiod_is_active_low(gpiod) ? 1 : 0;
}

static bool pdm_led_gpio_state_to_level(struct gpio_desc *gpiod, int state)
{
	return gpiod_is_active_low(gpiod) ? 1 : 0;
}

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
	int gpio_level;

	if (!client || !client->pdmdev) {
		OSA_ERROR("Invalid client\n");
		return -EINVAL;
	}

	gpiod = client->hardware.gpio.gpiod;
	gpio_level = pdm_led_gpio_state_to_level(gpiod, state);
	gpiod_set_value_cansleep(gpiod, gpio_level);

	OSA_INFO("GPIO PDM Led: Set %s state to %d\n", dev_name(&client->dev), state);
	return 0;
}

static int pdm_led_gpio_get_state(struct pdm_client *client, int *state)
{
	struct gpio_desc *gpiod;
	int gpio_level;

	if (!client || !client->pdmdev) {
		OSA_ERROR("Invalid client\n");
		return -EINVAL;
	}

	gpiod = client->hardware.gpio.gpiod;
	gpio_level = gpiod_get_value_cansleep(gpiod);
	*state = pdm_led_gpio_level_to_state(gpiod, gpio_level);

	OSA_INFO("GPIO PDM Led: Get %s state: %d\n", dev_name(&client->dev), *state);
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
	.get_state = pdm_led_gpio_get_state,
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
	int origin_level;
	int status;

	if (!client) {
		OSA_ERROR("Invalid client\n");
		return -EINVAL;
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

	origin_level = gpiod_get_value_cansleep(gpiod);
	led_priv->origin_state = pdm_led_gpio_level_to_state(gpiod, origin_level);

	client->hardware.gpio.gpiod = gpiod;

	OSA_DEBUG("GPIO LED Setup: %s\n", dev_name(&client->dev));
	return 0;
}

static void pdm_led_gpio_cleanup(struct pdm_client *client)
{
	struct pdm_led_priv *led_priv;
	struct gpio_desc *gpiod;
	int origin_level;

	if (!client && IS_ERR_OR_NULL(client->hardware.gpio.gpiod)) {
		return;
	}

	led_priv = pdm_client_get_private_data(client);
	if (!led_priv) {
		OSA_ERROR("Get PDM Client DevData Failed\n");
		return;
	}
	gpiod = client->hardware.gpio.gpiod;

	origin_level = pdm_led_gpio_state_to_level(gpiod, led_priv->origin_state);
	gpiod_set_value_cansleep(gpiod, origin_level);
	gpiod_put(client->hardware.gpio.gpiod);

	OSA_DEBUG("GPIO LED Cleanup: %s\n", dev_name(&client->dev));
}


/**
 * @brief Match data structure for initializing GPIO type LED devices.
 */
const struct pdm_client_match_data pdm_led_gpio_match_data = {
	.setup = pdm_led_gpio_setup,
	.cleanup = pdm_led_gpio_cleanup,
};

