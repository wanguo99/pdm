#include "pdm.h"
#include "pdm_adapter_priv.h"
#include "pdm_led_ioctl.h"
#include "pdm_led_priv.h"

static struct pdm_adapter *led_adapter = NULL;

/**
 * @brief Sets the state of a specified PDM LED device.
 *
 * @param args Structure.
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_led_set_state(struct pdm_client *client, struct pdm_led_ioctl_args *args)
{
    struct pdm_led_priv *led_priv;
    int status = 0;

    led_priv = pdm_client_get_drvdata(client);
    if (!led_priv) {
        OSA_ERROR("Get PDM Client Device Data Failed\n");
        return -ENOMEM;
    }

    status = led_priv->ops->set_state(client, args->state);
    if (status) {
        OSA_ERROR("PDM Led set_state failed, status: %d\n", status);
        return status;
    }

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
    struct pdm_led_ioctl_args args;
    int status = 0;

    if (!client) {
        OSA_ERROR("invalid argument\n");
        return -EINVAL;
    }

    OSA_DEBUG("ioctl, cmd=0x%02x, arg=0x%02lx\n", cmd, arg);

    memset(&args, 0, sizeof(args));
    switch (cmd) {
        case PDM_LED_SET_STATE: {
            if (copy_from_user(&args, (void __user *)arg, sizeof(args))) {
                return -EFAULT;
            }
            OSA_INFO("PDM_LED: Set %s's state to %d\n", dev_name(&client->dev), args.state);
            status = pdm_led_set_state(client, &args);
            break;
        }
        default:
            return -ENOTTY;
    }

    if (status) {
        OSA_ERROR("pdm_led_ioctl error\n");
    }

    return status;
}

/**
 * @brief Handles write operations from user space.
 *
 * @param filp File descriptor.
 * @param buf User space buffer.
 * @param count Buffer size.
 * @param ppos File offset.
 * @return Number of bytes written, or negative error code on failure.
 */
static ssize_t pdm_led_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
    struct pdm_client *client = filp->private_data;
    struct pdm_led_ioctl_args args;
    char kernel_buf[5];
    ssize_t bytes_read;

    if (!client) {
        return -EINVAL;
    }

    if (count > sizeof(kernel_buf) - 1) {
        count = sizeof(kernel_buf) - 1;
    }
    if ((bytes_read = copy_from_user(kernel_buf, buf, count)) != 0) {
        OSA_ERROR("Failed to copy data from user space: %zd\n", bytes_read);
        return -EFAULT;
    }

    if (sscanf(kernel_buf, "%d", &args.state) != 1) {
        OSA_ERROR("Invalid data: %s\n", kernel_buf);
        return -EINVAL;
    }

    if (pdm_led_set_state(client, &args)) {
        OSA_ERROR("pdm_led_set_state failed\n");
        return -EINVAL;
    }

    return count;
}

/**
 * @brief Initializes the LED client using match data.
 *
 * @param client Pointer to the PDM client structure.
 * @param pdmdev Pointer to the PDM device structure.
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

    led_priv = pdm_client_get_drvdata(client);
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
    struct pdm_led_priv *led_priv = pdm_client_get_drvdata(pdmdev->client);
    if (led_priv && led_priv->match_data && led_priv->match_data->cleanup) {
        led_priv->match_data->cleanup(pdmdev->client);
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
    .setup = NULL,
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
