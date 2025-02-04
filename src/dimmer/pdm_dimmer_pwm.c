#include <linux/pwm.h>

#include "pdm.h"
#include "pdm_dimmer_priv.h"


static int pdm_dimmer_pwm_set_level(struct pdm_client *client, unsigned int level)
{
	struct pdm_dimmer_priv *dimmer_priv;
	struct pwm_device *pwmdev;
	struct pwm_state pwmstate;
	struct pwm_args pwmargs;
	unsigned int duty_cycle;
	int status;

	if (!client || !client->pdmdev) {
		OSA_ERROR("Invalid client\n");
		return -EINVAL;
	}

	pwmdev = client->hardware.pwm.pwmdev;
	if (!pwmdev) {
		return -EINVAL;
	}

	dimmer_priv = pdm_client_get_private_data(client);
	if (!dimmer_priv) {
		OSA_ERROR("Get PDM Client DevData Failed\n");
		return -ENOMEM;
	}

	if (level > dimmer_priv->max_level) {
		OSA_ERROR("Invalid level %u, max_level: %u\n", level, dimmer_priv->max_level);
		return -EINVAL;
	}

	OSA_DEBUG("PWM PDM Dimmer: Set %s-%d level to %u\n", dev_name(&client->adapter->dev), client->index, level);

	if (!level) {
		pwm_disable(pwmdev);
		return 0;
	}

	duty_cycle = dimmer_priv->level_map[level];
	if (duty_cycle > PDM_DIMMER_MAX_LEVEL_VALUE) {
		OSA_ERROR("Invalid real level: %u\n", duty_cycle);
		return -EINVAL;
	}

	memset(&pwmargs, 0, sizeof(pwmargs));
	pwm_get_args(pwmdev, &pwmargs);

	memset(&pwmstate, 0, sizeof(pwmstate));
	pwmstate.period = pwmargs.period;
	pwmstate.enabled = true;
	pwm_set_relative_duty_cycle(&pwmstate, duty_cycle, PDM_DIMMER_MAX_LEVEL_VALUE);
	status = pwm_apply_might_sleep(pwmdev, &pwmstate);
	if (status) {
		OSA_ERROR("pwm_apply_might_sleep failed: %d\n", status);
		return status;
	}

	return 0;
}

static int pdm_dimmer_pwm_get_level(struct pdm_client *client, unsigned int *level)
{
	struct pdm_dimmer_priv *dimmer_priv;
	struct pwm_device *pwmdev;
	struct pwm_state pwmstate;
	unsigned int duty_cycle;
	int index;
	int status = -ENOENT;

	if (!client || !client->pdmdev) {
		OSA_ERROR("Invalid client\n");
		return -EINVAL;
	}

	pwmdev = client->hardware.pwm.pwmdev;
	if (!pwmdev) {
		return -EINVAL;
	}

	dimmer_priv = pdm_client_get_private_data(client);
	if (!dimmer_priv) {
		OSA_ERROR("Get PDM Client DevData Failed\n");
		return -ENOMEM;
	}

	memset(&pwmstate, 0, sizeof(pwmstate));
	pwm_get_state(pwmdev, &pwmstate);

	*level = 0;
	if (pwmstate.enabled) {
		duty_cycle = pwm_get_relative_duty_cycle(&pwmstate, PDM_DIMMER_MAX_LEVEL_VALUE);
		for (index = 0; index <= dimmer_priv->max_level; index++)
		{
			if (duty_cycle == dimmer_priv->level_map[index]) {
				*level = index;
				status = 0;
				break;
			}
		}
	} else {
		*level = 0;
		status = 0;
	}

	if (status) {
		OSA_ERROR("PWM PDM Dimmer: Get level failed\n");
		return status;
	}

	OSA_INFO("PWM PDM Dimmer: Get %s-%d level: %u\n", dev_name(&client->adapter->dev), client->index, *level);
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
	struct pwm_device *pwmdev;
	struct pwm_state pwmstate;
	struct device_node *np;
	unsigned int default_level;
	unsigned int level_count;
	unsigned int *level_map;
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
		OSA_WARN("No default-state property found, using defaults as off\n");
		default_level = 0;
	} else if (default_level > PDM_DIMMER_MAX_LEVEL_VALUE) {
		OSA_WARN("Invalid default-level (0~255): %u\n", default_level);
		OSA_WARN("Set default-level to 0\n");
		default_level = 0;
	}

	level_count = of_property_count_elems_of_size(np, "level-map", sizeof(u32));
	if (level_count == 0) {
		OSA_ERROR("Failed to get max level\n");
		return -EINVAL;
	}
	dimmer_priv->max_level = level_count - 1;

	level_map = kmalloc(level_count * sizeof(u32), GFP_KERNEL);
	if (!level_map) {
		OSA_ERROR("Failed to allocate memory for level map\n");
		return -ENOMEM;
	}
	dimmer_priv->level_map = level_map;

	status = of_property_read_u32_array(np, "level-map", level_map, level_count);
	if (status) {
		OSA_ERROR("Failed to get levels\n");
		goto err_level_map_free;
	}

	pwmdev = pwm_get(client->pdmdev->dev.parent, NULL);
	if (IS_ERR(pwmdev)) {
		OSA_ERROR("Failed to get PWM\n");
		status = PTR_ERR(pwmdev);
		goto err_level_map_free;
	}
	client->hardware.pwm.pwmdev = pwmdev;

	pwm_init_state(pwmdev, &pwmstate);

	status = pdm_dimmer_pwm_set_level(client, default_level);
	if (status) {
		OSA_WARN("Failed to set default level: %d\n", status);
	}

	OSA_DEBUG("PWM DIMMER Setup: %s-%d\n", dev_name(&client->adapter->dev), client->index);
	return 0;

err_level_map_free:
	kfree(level_map);
	dimmer_priv->level_map = NULL;
	return status;
}

static void pdm_dimmer_pwm_cleanup(struct pdm_client *client)
{
	struct pdm_dimmer_priv *dimmer_priv;
	struct pwm_device *pwmdev;

	if (!client || IS_ERR_OR_NULL(client->hardware.pwm.pwmdev)) {
		return;
	}
	pwmdev = client->hardware.pwm.pwmdev;

	dimmer_priv = pdm_client_get_private_data(client);
	if (dimmer_priv) {
		pdm_dimmer_pwm_set_level(client, 0);
	}

	pwm_put(pwmdev);
	kfree(dimmer_priv->level_map);
	dimmer_priv->level_map = NULL;
}

/**
 * @brief Match data structure for initializing PWM type DIMMER devices.
 */
const struct pdm_client_match_data pdm_dimmer_pwm_match_data = {
	.setup = pdm_dimmer_pwm_setup,
	.cleanup = pdm_dimmer_pwm_cleanup,
};
