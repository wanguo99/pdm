#include <linux/pwm.h>

#include "pdm.h"
#include "pdm_led_priv.h"

/**
 * @brief Sets the state of a PWM LED.
 *
 * This function sets the state (on/off) of the specified PDM device's PWM LED.
 *
 * @param client Pointer to the PDM client structure.
 * @param state LED state (0 for off, 1 for on).
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_led_pwm_set_state(struct pdm_client *client, int state)
{
    if (!client || !client->pdmdev) {
        OSA_ERROR("Invalid client\n");
        return -EINVAL;
    }

    OSA_INFO("PWM PDM Led: Set %s state to %d\n", dev_name(&client->dev), state);
    return 0;
}

/**
 * @struct pdm_led_operations
 * @brief PDM LED device operations structure (PWM version).
 *
 * This structure defines the operation functions for a PDM LED device using PWM.
 */
static const struct pdm_led_operations pdm_device_led_ops_pwm = {
    .set_state = pdm_led_pwm_set_state,
};

/**
 * @brief Initializes PWM settings for a PDM device.
 *
 * This function initializes the PWM settings for the specified PDM device and sets up the operation functions.
 *
 * @param client Pointer to the PDM client structure.
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_led_pwm_setup(struct pdm_client *client)
{
    struct pdm_led_priv *led_priv;
    struct device_node *np;
    unsigned int default_level;
    struct pwm_device *pwmdev;
    int status;

    if (!client) {
        OSA_ERROR("Invalid client\n");
    }

    led_priv = pdm_client_get_private_data(client);
    if (!led_priv) {
        OSA_ERROR("Get PDM Client DevData Failed\n");
        return -ENOMEM;
    }

    led_priv->ops = &pdm_device_led_ops_pwm;

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
    OSA_DEBUG("PWM LED Setup: %s\n", dev_name(&client->dev));
    return 0;

}

static void pdm_led_pwm_cleanup(struct pdm_client *client)
{
    if (client && !IS_ERR_OR_NULL(client->hardware.pwm.pwmdev)) {
        pwm_put(client->hardware.pwm.pwmdev);
    }
}

/**
 * @brief Match data structure for initializing PWM type LED devices.
 */
const struct pdm_client_match_data pdm_led_pwm_match_data = {
    .setup = pdm_led_pwm_setup,
    .cleanup = pdm_led_pwm_cleanup,
};
