#include "pdm.h"
#include "pdm_adapter_priv.h"
#include "pdm_eeprom_ioctl.h"
#include "pdm_eeprom_priv.h"

static struct pdm_adapter *eeprom_adapter = NULL;

/**
 * @brief Sets the state of a specified PDM EEPROM device.
 *
 * @param client Pointer to the PDM client structure.
 * @param state State value (0 or 1).
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_eeprom_read_reg(struct pdm_client *client, unsigned char addr, unsigned char *value)
{
    struct pdm_eeprom_priv *eeprom_priv;
    int status = 0;

    if (!client) {
        OSA_ERROR("Invalid client\n");
        return -EINVAL;
    }

    eeprom_priv = pdm_client_get_private_data(client);
    if (!eeprom_priv) {
        OSA_ERROR("Get PDM Client Device Data Failed\n");
        return -ENOMEM;
    }

    if (!eeprom_priv->ops || !eeprom_priv->ops->read_reg) {
        OSA_ERROR("read_reg not supported\n");
        return -ENOTSUPP;
    }

    status = eeprom_priv->ops->read_reg(client, addr, value);
    if (status) {
        OSA_ERROR("PDM EEPROM read_reg failed, status: %d\n", status);
        return status;
    }

    return 0;
}

/**
 * @brief Gets the current state of a specified PDM EEPROM device.
 *
 * @param client Pointer to the PDM client structure.
 * @param state Pointer to store the current state.
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_eeprom_write_reg(struct pdm_client *client, unsigned char addr, unsigned char value)
{
    struct pdm_eeprom_priv *eeprom_priv;
    int status = 0;

    if (!client) {
        OSA_ERROR("Invalid argument\n");
        return -EINVAL;
    }

    eeprom_priv = pdm_client_get_private_data(client);
    if (!eeprom_priv) {
        OSA_ERROR("Get PDM Client Device Data Failed\n");
        return -ENOMEM;
    }

    if (!eeprom_priv->ops || !eeprom_priv->ops->write_reg) {
        OSA_ERROR("write_reg not supported\n");
        return -ENOTSUPP;
    }

    status = eeprom_priv->ops->write_reg(client, addr, value);
    if (status) {
        OSA_ERROR("PDM EEPROM write_reg failed, status: %d\n", status);
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
static long pdm_eeprom_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct pdm_client *client = filp->private_data;
    int status = 0;

    if (!client) {
        OSA_ERROR("Invalid client\n");
        return -EINVAL;
    }

    switch (cmd) {
        default:
        {
            OSA_ERROR("Unknown ioctl command\n");
            return -ENOTTY;
        }
    }

    if (status) {
        OSA_ERROR("pdm_eeprom_ioctl error\n");
        return status;
    }

    return 0;
}


/**
 * @brief Reads information about available commands or EEPROM state/brightness.
 *
 * @param filp File pointer.
 * @param buf User buffer to write data into.
 * @param count Number of bytes to read.
 * @param ppos Offset in the file.
 * @return Returns number of bytes read or negative error code on failure.
 */
static ssize_t pdm_eeprom_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
    const char help_info[] =
        "Available commands:\n"
        " > 1 <0|1>    - Set EEPROM state\n"
        " > 2          - Get current EEPROM state\n"
        " > 3 <0-255>  - Set EEPROM brightness\n"
        " > 4          - Get current EEPROM brightness\n";
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
 * @brief Writes commands to change EEPROM state or brightness.
 *
 * @param filp File pointer.
 * @param buf User buffer containing command data.
 * @param count Number of bytes to write.
 * @param ppos Offset in the file.
 * @return Returns number of bytes written or negative error code on failure.
 */
static ssize_t pdm_eeprom_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
    struct pdm_client *client = filp->private_data;
    char kernel_buf[64];
    ssize_t bytes_read;
    char buffer[32];
    unsigned char addr = 0x0;
    unsigned char value = 0x0;
    int cmd;

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

    switch (cmd)
    {
        case PDM_EEPROM_CMD_WRITE_REG:
        {
            if (sscanf(kernel_buf, "%d 0x%hhx 0x%hhx", &cmd, &addr, &value) != 3) {
                OSA_ERROR("Command %d requires one parameter.\n", cmd);
                return -EINVAL;
            }
            break;
        }
        case PDM_EEPROM_CMD_READ_REG:
        {
            if (sscanf(kernel_buf, "%d 0x%hhx", &cmd, &addr) != 2) {
                OSA_ERROR("Command %d should not have parameters.\n", cmd);
                return -EINVAL;
            }
            break;
        }
        default:
        {
            OSA_ERROR("Unknown command: %d\n", cmd);
            return -EINVAL;
        }
    }

    switch (cmd)
    {
        case PDM_EEPROM_CMD_WRITE_REG:
        {
            if (pdm_eeprom_write_reg(client, addr, value)) {
                OSA_ERROR("pdm_eeprom_set_state failed\n");
                return -EINVAL;
            }
            break;
        }
        case PDM_EEPROM_CMD_READ_REG:
        {
            if (pdm_eeprom_read_reg(client, addr, &value)) {
                OSA_ERROR("pdm_eeprom_get_state failed\n");
                return -EINVAL;
            }
            snprintf(buffer, sizeof(buffer), "%d\n", value);
            bytes_read = simple_read_from_buffer((void __user *)buf, strlen(buffer), ppos, buffer, strlen(buffer));
            return bytes_read;
        }
        default:
        {
            OSA_ERROR("Unknown command: %d\n", cmd);
            return -EINVAL;
        }
    }

    return count;
}

/**
 * @brief Initializes the EEPROM client using match data.
 *
 * @param client Pointer to the PDM client structure.
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_eeprom_match_setup(struct pdm_client *client)
{
    struct pdm_eeprom_priv *eeprom_priv;
    const void *match_data;
    int status;

    if (!client) {
        return -EINVAL;
    }

    eeprom_priv = pdm_client_get_private_data(client);
    if (!eeprom_priv) {
        OSA_ERROR("EEPROM Client get private data is NULL\n");
        return -ENOMEM;
    }

    match_data = pdm_client_get_match_data(client);
    if (!match_data) {
        OSA_ERROR("Failed to get match data for device\n");
        return -ENODEV;
    }

    eeprom_priv->match_data = match_data;
    if (eeprom_priv->match_data->setup) {
        status = eeprom_priv->match_data->setup(client);
        if (status) {
            OSA_ERROR("EEPROM Client Setup Failed, status=%d\n", status);
            return status;
        }
    }

    return 0;
}

/**
 * @brief Probes the EEPROM PDM device.
 *
 * This function is called when a PDM device is detected and adds the device to the main device.
 *
 * @param pdmdev Pointer to the PDM device.
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_eeprom_device_probe(struct pdm_device *pdmdev)
{
    struct pdm_client *client;
    int status;

    client = devm_pdm_client_alloc(pdmdev, sizeof(struct pdm_eeprom_priv));
    if (IS_ERR(client)) {
        OSA_ERROR("EEPROM Client Alloc Failed\n");
        return PTR_ERR(client);
    }

    status = devm_pdm_client_register(eeprom_adapter, client);
    if (status) {
        OSA_ERROR("EEPROM Adapter Add Device Failed, status=%d\n", status);
        return status;
    }

    status = pdm_eeprom_match_setup(client);
    if (status) {
        OSA_ERROR("EEPROM Client Setup Failed, status=%d\n", status);
        return status;
    }

    client->fops.read = pdm_eeprom_read;
    client->fops.write = pdm_eeprom_write;
    client->fops.unlocked_ioctl = pdm_eeprom_ioctl;

    return 0;
}

/**
 * @brief Removes the EEPROM PDM device.
 *
 * This function is called when a PDM device is removed and deletes the device from the main device.
 *
 * @param pdmdev Pointer to the PDM device.
 */
static void pdm_eeprom_device_remove(struct pdm_device *pdmdev)
{
    if (pdmdev && pdmdev->client) {
        pdm_eeprom_match_setup(pdmdev->client);
    }
}

/**
 * @brief Match data structure for initializing GPIO type EEPROM devices.
 */
static const struct pdm_eeprom_match_data pdm_eeprom_spi_match_data = {
    .setup = pdm_eeprom_spi_setup,
    .cleanup = NULL,
};

/**
 * @brief Match data structure for initializing PWM type EEPROM devices.
 */
static const struct pdm_eeprom_match_data pdm_eeprom_i2c_match_data = {
    .setup = NULL,
    .cleanup = NULL,
};

/**
 * @brief Device tree match table.
 *
 * Defines the supported device tree compatible properties.
 */
static const struct of_device_id of_pdm_eeprom_match[] = {
    { .compatible = "pdm,eeprom-spi",     .data = &pdm_eeprom_spi_match_data},
    { .compatible = "pdm,eeprom-i2c",     .data = &pdm_eeprom_i2c_match_data},
    {},
};
MODULE_DEVICE_TABLE(of, of_pdm_eeprom_match);

/**
 * @brief EEPROM PDM driver structure.
 *
 * Defines the basic information and operation functions of the EEPROM PDM driver.
 */
static struct pdm_driver pdm_eeprom_driver = {
    .probe = pdm_eeprom_device_probe,
    .remove = pdm_eeprom_device_remove,
    .driver = {
        .name = "pdm-eeprom",
        .of_match_table = of_pdm_eeprom_match,
    },
};

/**
 * @brief Initializes the EEPROM PDM adapter driver.
 *
 * Allocates and registers the adapter and driver.
 *
 * @return Returns 0 on success; negative error code on failure.
 */
int pdm_eeprom_driver_init(void)
{
    int status;

    eeprom_adapter = pdm_adapter_alloc(sizeof(void *));
    if (!eeprom_adapter) {
        OSA_ERROR("Failed to allocate pdm_adapter\n");
        return -ENOMEM;
    }

    status = pdm_adapter_register(eeprom_adapter, PDM_EEPROM_NAME);
    if (status) {
        OSA_ERROR("Failed to register EEPROM PDM Adapter, status=%d\n", status);
        return status;
    }

    status = pdm_bus_register_driver(THIS_MODULE, &pdm_eeprom_driver);
    if (status) {
        OSA_ERROR("Failed to register EEPROM PDM Driver, status=%d\n", status);
        goto err_adapter_unregister;
    }

    return 0;

err_adapter_unregister:
    pdm_adapter_unregister(eeprom_adapter);
    return status;
}

/**
 * @brief Exits the EEPROM PDM adapter driver.
 *
 * Unregisters the driver and adapter, releasing related resources.
 */
void pdm_eeprom_driver_exit(void)
{
    pdm_bus_unregister_driver(&pdm_eeprom_driver);
    pdm_adapter_unregister(eeprom_adapter);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("EEPROM PDM Adapter Driver");
