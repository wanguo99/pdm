#include <linux/pwm.h>

#include "pdm.h"
#include "pdm_dimmer_priv.h"


static int pdm_dimmer_pwm_set_level(struct pdm_client *client, int level)
{
	if (!client || !client->pdmdev) {
		OSA_ERROR("Invalid client\n");
		return -EINVAL;
	}

	OSA_INFO("PWM PDM Dimmer: Set %s level to %d\n", dev_name(&client->dev), level);
	return 0;
}

static int pdm_dimmer_pwm_get_level(struct pdm_client *client, int *level)
{
	int pwm_value = 0;

	if (!client || !client->pdmdev) {
		OSA_ERROR("Invalid client\n");
		return -EINVAL;
	}

	*level = pwm_value;

	OSA_INFO("PWM PDM Dimmer: Get %s level: %d\n", dev_name(&client->dev), *level);
	return 0;
}

/**
 * @brief Initializes PWM settings for a PDM device.
 *
 * This function initializes the PWM settings for the specified PDM device and sets up the operation functions.
 *
 * @param client Pointer to the PDM client structure.
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_dimmer_pwm_setup(struct pdm_client *client)
{
	struct pdm_dimmer_priv *dimmer_priv;
	struct device_node *np;
	unsigned int default_level;
	struct pwm_device *pwmdev;
	int status;

	if (!client) {
		OSA_ERROR("Invalid client\n");
		return -EINVAL;
	}

	dimmer_priv = pdm_client_get_private_data(client);
	if (!dimmer_priv) {
		OSA_ERROR("Get PDM Client DevData Failed\n");
		return -ENOMEM;
	}

	dimmer_priv->set_level = pdm_dimmer_pwm_set_level;
	dimmer_priv->get_level = pdm_dimmer_pwm_get_level;

	np = pdm_client_get_of_node(client);
	if (!np) {
		OSA_ERROR("No DT node found\n");
		return -EINVAL;
	}

	status = of_property_read_u32(np, "default-level", &default_level);
	if (status) {
		OSA_INFO("No default-state property found, using defaults as off\n");
		default_level = 0;
	}

	pwmdev = pwm_get(client->pdmdev->dev.parent, NULL);
	if (IS_ERR(pwmdev)) {
		OSA_ERROR("Failed to get PWM\n");
		return PTR_ERR(pwmdev);
	}

	client->hardware.pwm.pwmdev = pwmdev;
	OSA_DEBUG("PWM DIMMER Setup: %s\n", dev_name(&client->dev));
	return 0;

}

static void pdm_dimmer_pwm_cleanup(struct pdm_client *client)
{
	if (client && !IS_ERR_OR_NULL(client->hardware.pwm.pwmdev)) {
		pwm_put(client->hardware.pwm.pwmdev);
	}
}

/**
 * @brief Match data structure for initializing PWM type DIMMER devices.
 */
const struct pdm_client_match_data pdm_dimmer_pwm_match_data = {
	.setup = pdm_dimmer_pwm_setup,
	.cleanup = pdm_dimmer_pwm_cleanup,
};
