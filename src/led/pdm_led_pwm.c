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
int pdm_led_pwm_setup(struct pdm_client *client)
{
    struct pdm_led_priv *led_priv;
    if (!client) {
        OSA_ERROR("Invalid client\n");
    }

    led_priv = pdm_client_get_private_data(client);
    if (!led_priv) {
        OSA_ERROR("Get PDM Client DevData Failed\n");
        return -ENOMEM;
    }

    led_priv->ops = &pdm_device_led_ops_pwm;

    OSA_DEBUG("PWM LED Setup: %s\n", dev_name(&client->dev));
    return 0;
}

#if 0
static int pdm_device_pwm_setup(struct pdm_device *pdmdev)
{
    struct pdm_device_priv *pdmdev_priv;
    struct device_node *np;
    unsigned int default_level;
    struct pwm_device *pwmdev;
    int status;

    if (!pdmdev) {
        OSA_ERROR("Invalid pdmdev\n");
        return -EINVAL;
    }

    pdmdev_priv = pdm_device_get_private_data(pdmdev);
    if (!pdmdev_priv) {
        OSA_ERROR("Get PDM Device DrvData Failed\n");
        return -ENOMEM;
    }

    np = pdm_device_get_of_node(pdmdev);
    if (!np) {
        OSA_ERROR("No DT node found\n");
        return -EINVAL;
    }

    status = of_property_read_u32(np, "default-level", &default_level);
    if (status) {
        OSA_INFO("No default-state property found, using defaults as off\n");
        default_level = 0;
    }

    pwmdev = pwm_get(pdmdev->dev.parent, NULL);
    if (IS_ERR(pwmdev)) {
        OSA_ERROR("Failed to get PWM\n");
        return PTR_ERR(pwmdev);
    }

    pdmdev_priv->hardware.pwm.pwmdev = pwmdev;
    return 0;

}

static void pdm_device_pwm_cleanup(struct pdm_device *pdmdev)
{
    struct pdm_device_priv *pdmdev_priv;

    if (!pdmdev) {
        return;
    }

    pdmdev_priv = pdm_device_get_private_data(pdmdev);
    if (pdmdev_priv && !IS_ERR_OR_NULL(pdmdev_priv->hardware.pwm.pwmdev)) {
        pwm_put(pdmdev_priv->hardware.pwm.pwmdev);
    }
}
#endif
