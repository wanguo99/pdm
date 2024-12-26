#include "pdm.h"
#include "pdm_adapter_priv.h"
#include "pdm_led_ioctl.h"
#include "pdm_led_priv.h"

static struct pdm_adapter *led_adapter = NULL;

/**
 * @brief Sets the brightness of a specified PDM LED device.
 *
 * @param client Pointer to the PDM client structure.
 * @param brightness Brightness level (0-255).
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_led_set_brightness(struct pdm_client *client, int brightness)
{
    struct pdm_led_priv *led_priv;
    int status = 0;

    if (!client) {
        OSA_ERROR("Invalid client\n");
        return -EINVAL;
    }

    if (brightness < 0 || brightness > 255) {
        OSA_ERROR("Invalid brightness: %d\n", brightness);
        return -EINVAL;
    }

    led_priv = pdm_client_get_private_data(client);
    if (!led_priv) {
        OSA_ERROR("Get PDM Client Device Data Failed\n");
        return -ENOMEM;
    }

    if (!led_priv->ops || !led_priv->ops->set_brightness) {
        OSA_ERROR("set_brightness not supported\n");
        return -ENOTSUPP;
    }

    status = led_priv->ops->set_brightness(client, brightness);
    if (status) {
        OSA_ERROR("PDM Led set_brightness failed, status: %d\n", status);
        return status;
    }

    return 0;
}

/**
 * @brief Gets the current brightness of a specified PDM LED device.
 *
 * @param client Pointer to the PDM client structure.
 * @param brightness Pointer to store the current brightness.
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_led_get_brightness(struct pdm_client *client, int *brightness)
{
    struct pdm_led_priv *led_priv;
    int status = 0;

    if (!client || !brightness) {
        OSA_ERROR("Invalid argument\n");
        return -EINVAL;
    }

    led_priv = pdm_client_get_private_data(client);
    if (!led_priv) {
        OSA_ERROR("Get PDM Client Device Data Failed\n");
        return -ENOMEM;
    }

    if (!led_priv->ops || !led_priv->ops->get_brightness) {
        OSA_ERROR("get_brightness not supported\n");
        return -ENOTSUPP;
    }

    status = led_priv->ops->get_brightness(client, brightness);
    if (status) {
        OSA_ERROR("PDM Led get_brightness failed, status: %d\n", status);
        return status;
    }

    OSA_INFO("Current brightness is %d\n", *brightness);

    return 0;
}

/**
 * @brief Sets the state of a specified PDM LED device.
 *
 * @param client Pointer to the PDM client structure.
 * @param state State value (0 or 1).
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_led_set_state(struct pdm_client *client, int state)
{
    struct pdm_led_priv *led_priv;
    int status = 0;

    if (!client) {
        OSA_ERROR("Invalid client\n");
        return -EINVAL;
    }

    led_priv = pdm_client_get_private_data(client);
    if (!led_priv) {
        OSA_ERROR("Get PDM Client Device Data Failed\n");
        return -ENOMEM;
    }

    if (!led_priv->ops || !led_priv->ops->set_state) {
        OSA_ERROR("set_state not supported\n");
        return -ENOTSUPP;
    }

    status = led_priv->ops->set_state(client, state);
    if (status) {
        OSA_ERROR("PDM Led set_state failed, status: %d\n", status);
        return status;
    }

    return 0;
}

/**
 * @brief Gets the current state of a specified PDM LED device.
 *
 * @param client Pointer to the PDM client structure.
 * @param state Pointer to store the current state.
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_led_get_state(struct pdm_client *client, int *state)
{
    struct pdm_led_priv *led_priv;
    int status = 0;

    if (!client || !state) {
        OSA_ERROR("Invalid argument\n");
        return -EINVAL;
    }

    led_priv = pdm_client_get_private_data(client);
    if (!led_priv) {
        OSA_ERROR("Get PDM Client Device Data Failed\n");
        return -ENOMEM;
    }

    if (!led_priv->ops || !led_priv->ops->get_state) {
        OSA_ERROR("get_state not supported\n");
        return -ENOTSUPP;
    }

    status = led_priv->ops->get_state(client, state);
    if (status) {
        OSA_ERROR("PDM Led get_state failed, status: %d\n", status);
        return status;
    }

    OSA_INFO("Current state is %d\n", *state);

    return 0;
}

/**
 * @brief Handles IOCTL commands from user space.
 *
 * @param file File descriptor.
 * @param cmd IOCTL command.
 * @param arg Command argument.
 * @return Returns 0 on success; negative error code on failure.
 */
static long pdm_led_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct pdm_client *client = filp->private_data;
    int status = 0;

    if (!client) {
        OSA_ERROR("Invalid client\n");
        return -EINVAL;
    }

    switch (cmd) {
        case PDM_LED_SET_STATE: {
            int state;
            if (copy_from_user(&state, (void __user *)arg, sizeof(state))) {
                OSA_ERROR("Failed to copy data from user space\n");
                return -EFAULT;
            }
            OSA_INFO("PDM_LED: Set %s's state to %d\n", dev_name(&client->dev), state);
            status = pdm_led_set_state(client, state);
            break;
        }
        case PDM_LED_GET_STATE:
        {
            int state;
            status = pdm_led_get_state(client, &state);
            if (status) {
                OSA_ERROR("Failed to get LED state, status: %d\n", status);
                return status;
            }
            OSA_INFO("PDM_LED: Current state is %d\n", state);
            if (copy_to_user((void __user *)arg, &state, sizeof(state))) {
                OSA_ERROR("Failed to copy data to user space\n");
                return -EFAULT;
            }
            break;
        }
        case PDM_LED_SET_BRIGHTNESS: {
            int brightness;
            if (copy_from_user(&brightness, (void __user *)arg, sizeof(brightness))) {
                OSA_ERROR("Failed to copy data from user space\n");
                return -EFAULT;
            }
            OSA_INFO("PDM_LED: Set %s's brightness to %d\n", dev_name(&client->dev), brightness);
            status = pdm_led_set_brightness(client, brightness);
            break;
        }
        case PDM_LED_GET_BRIGHTNESS: {
            int brightness;
            status = pdm_led_get_brightness(client, &brightness);
            if (status) {
                OSA_ERROR("Failed to get LED brightness, status: %d\n", status);
                return status;
            }
            OSA_INFO("PDM_LED: Current brightness is %d\n", brightness);
            if (copy_to_user((void __user *)arg, &brightness, sizeof(brightness))) {
                OSA_ERROR("Failed to copy data to user space\n");
                return -EFAULT;
            }
            break;
        }
        default: {
            OSA_ERROR("Unknown ioctl command\n");
            return -ENOTTY;
        }
    }

    if (status) {
        OSA_ERROR("pdm_led_ioctl error\n");
        return status;
    }

    return 0;
}


/**
 * @brief Reads information about available commands or LED state/brightness.
 *
 * @param filp File pointer.
 * @param buf User buffer to write data into.
 * @param count Number of bytes to read.
 * @param ppos Offset in the file.
 * @return Returns number of bytes read or negative error code on failure.
 */
static ssize_t pdm_led_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
    const char help_info[] =
        "Available commands:\n"
        " > 1 <0|1>    - Set LED state\n"
        " > 2          - Get current LED state\n"
        " > 3 <0-255>  - Set LED brightness\n"
        " > 4          - Get current LED brightness\n";
    size_t len = strlen(help_info);

    if (*ppos >= len)
        return 0;

    // 使用min_t宏指定类型为size_t
    size_t remaining = min_t(size_t, count, len - *ppos);

    if (copy_to_user(buf, help_info + *ppos, remaining))
        return -EFAULT;

    *ppos += remaining;
    return remaining;
}

/**
 * @brief Writes commands to change LED state or brightness.
 *
 * @param filp File pointer.
 * @param buf User buffer containing command data.
 * @param count Number of bytes to write.
 * @param ppos Offset in the file.
 * @return Returns number of bytes written or negative error code on failure.
 */
static ssize_t pdm_led_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
    struct pdm_client *client = filp->private_data;
    char kernel_buf[64];
    ssize_t bytes_read;
    int cmd;
    int param;

    if (!client || count >= sizeof(kernel_buf)) {
        OSA_ERROR("Invalid client or input too long.\n");
        return -EINVAL;
    }

    if ((bytes_read = copy_from_user(kernel_buf, buf, count)) != 0) {
        OSA_ERROR("Failed to copy data from user space: %zd\n", bytes_read);
        return -EFAULT;
    }

    kernel_buf[count] = '\0';
    if (sscanf(kernel_buf, "%d", &cmd) != 1) {
        OSA_ERROR("Invalid command format: %s\n", kernel_buf);
        return -EINVAL;
    }

    switch (cmd) {
        case PDM_LED_CMD_SET_STATE:
        case PDM_LED_CMD_SET_BRIGHTNESS:
        {
            if (sscanf(kernel_buf, "%d %d", &cmd, &param) != 2) {
                OSA_ERROR("Command %d requires one parameter.\n", cmd);
                return -EINVAL;
            }
            break;
        }
        case PDM_LED_CMD_GET_STATE:
        case PDM_LED_CMD_GET_BRIGHTNESS:
        {
            if (sscanf(kernel_buf, "%d", &cmd) != 1) {
                OSA_ERROR("Command %d should not have parameters.\n", cmd);
                return -EINVAL;
            }
            param = 0;
            break;
        }
        default: {
            OSA_ERROR("Unknown command: %d\n", cmd);
            return -EINVAL;
        }
    }

    switch (cmd) {
        case PDM_LED_CMD_SET_STATE: {
            if (param != 0 && param != 1) {
                OSA_ERROR("Invalid state: %d\n", param);
                return -EINVAL;
            }
            if (pdm_led_set_state(client, param)) {
                OSA_ERROR("pdm_led_set_state failed\n");
                return -EINVAL;
            }
            break;
        }
        case PDM_LED_CMD_GET_STATE: {
            int state;
            if (pdm_led_get_state(client, &state)) {
                OSA_ERROR("pdm_led_get_state failed\n");
                return -EINVAL;
            }
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "%d\n", state);
            bytes_read = simple_read_from_buffer((void __user *)buf, strlen(buffer), ppos, buffer, strlen(buffer));
            return bytes_read;
        }
        case PDM_LED_CMD_SET_BRIGHTNESS: {
            if (pdm_led_set_brightness(client, param)) {
                OSA_ERROR("pdm_led_set_brightness failed\n");
                return -EINVAL;
            }
            break;
        }
        case PDM_LED_CMD_GET_BRIGHTNESS: {
            int brightness;
            if (pdm_led_get_brightness(client, &brightness)) {
                OSA_ERROR("pdm_led_get_brightness failed\n");
                return -EINVAL;
            }
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "%d\n", brightness);
            bytes_read = simple_read_from_buffer((void __user *)buf, strlen(buffer), ppos, buffer, strlen(buffer));
            return bytes_read;
        }
        default: {
            OSA_ERROR("Unknown command: %d\n", cmd);
            return -EINVAL;
        }
    }

    return count;
}

/**
 * @brief Initializes the LED client using match data.
 *
 * @param client Pointer to the PDM client structure.
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_led_match_setup(struct pdm_client *client)
{
    struct pdm_led_priv *led_priv;
    const void *match_data;
    int status;

    if (!client) {
        return -EINVAL;
    }

    led_priv = pdm_client_get_private_data(client);
    if (!led_priv) {
        OSA_ERROR("LED Client get private data is NULL\n");
        return -ENOMEM;
    }

    match_data = pdm_client_get_match_data(client);
    if (!match_data) {
        OSA_ERROR("Failed to get match data for device\n");
        return -ENODEV;
    }

    led_priv->match_data = match_data;
    if (led_priv->match_data->setup) {
        status = led_priv->match_data->setup(client);
        if (status) {
            OSA_ERROR("LED Client Setup Failed, status=%d\n", status);
            return status;
        }
    }

    return 0;
}

/**
 * @brief Probes the LED PDM device.
 *
 * This function is called when a PDM device is detected and adds the device to the main device.
 *
 * @param pdmdev Pointer to the PDM device.
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_led_device_probe(struct pdm_device *pdmdev)
{
    struct pdm_client *client;
    int status;

    client = devm_pdm_client_alloc(pdmdev, sizeof(struct pdm_led_priv));
    if (IS_ERR(client)) {
        OSA_ERROR("LED Client Alloc Failed\n");
        return PTR_ERR(client);
    }

    status = devm_pdm_client_register(led_adapter, client);
    if (status) {
        OSA_ERROR("LED Adapter Add Device Failed, status=%d\n", status);
        return status;
    }

    status = pdm_led_match_setup(client);
    if (status) {
        OSA_ERROR("LED Client Setup Failed, status=%d\n", status);
        return status;
    }

    client->fops.read = pdm_led_read;
    client->fops.write = pdm_led_write;
    client->fops.unlocked_ioctl = pdm_led_ioctl;

    return 0;
}

/**
 * @brief Removes the LED PDM device.
 *
 * This function is called when a PDM device is removed and deletes the device from the main device.
 *
 * @param pdmdev Pointer to the PDM device.
 */
static void pdm_led_device_remove(struct pdm_device *pdmdev)
{
    if (pdmdev && pdmdev->client) {
        pdm_led_match_setup(pdmdev->client);
    }
}

/**
 * @brief Match data structure for initializing GPIO type LED devices.
 */
static const struct pdm_led_match_data pdm_led_gpio_match_data = {
    .setup = pdm_led_gpio_setup,
    .cleanup = NULL,
};

/**
 * @brief Match data structure for initializing PWM type LED devices.
 */
static const struct pdm_led_match_data pdm_led_pwm_match_data = {
    .setup = pdm_led_pwm_setup,
    .cleanup = NULL,
};

/**
 * @brief Device tree match table.
 *
 * Defines the supported device tree compatible properties.
 */
static const struct of_device_id of_pdm_led_match[] = {
    { .compatible = "led,pdm-device-gpio", .data = &pdm_led_gpio_match_data},
    { .compatible = "led,pdm-device-pwm", .data = &pdm_led_pwm_match_data},
    {},
};
MODULE_DEVICE_TABLE(of, of_pdm_led_match);

/**
 * @brief LED PDM driver structure.
 *
 * Defines the basic information and operation functions of the LED PDM driver.
 */
static struct pdm_driver pdm_led_driver = {
    .probe = pdm_led_device_probe,
    .remove = pdm_led_device_remove,
    .driver = {
        .name = "pdm-led",
        .of_match_table = of_pdm_led_match,
    },
};

/**
 * @brief Initializes the LED PDM adapter driver.
 *
 * Allocates and registers the adapter and driver.
 *
 * @return Returns 0 on success; negative error code on failure.
 */
int pdm_led_driver_init(void)
{
    int status;

    led_adapter = pdm_adapter_alloc(sizeof(void *));
    if (!led_adapter) {
        OSA_ERROR("Failed to allocate pdm_adapter\n");
        return -ENOMEM;
    }

    status = pdm_adapter_register(led_adapter, PDM_LED_NAME);
    if (status) {
        OSA_ERROR("Failed to register LED PDM Adapter, status=%d\n", status);
        return status;
    }

    status = pdm_bus_register_driver(THIS_MODULE, &pdm_led_driver);
    if (status) {
        OSA_ERROR("Failed to register LED PDM Driver, status=%d\n", status);
        goto err_adapter_unregister;
    }

    return 0;

err_adapter_unregister:
    pdm_adapter_unregister(led_adapter);
    return status;
}

/**
 * @brief Exits the LED PDM adapter driver.
 *
 * Unregisters the driver and adapter, releasing related resources.
 */
void pdm_led_driver_exit(void)
{
    pdm_bus_unregister_driver(&pdm_led_driver);
    pdm_adapter_unregister(led_adapter);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("LED PDM Adapter Driver");
