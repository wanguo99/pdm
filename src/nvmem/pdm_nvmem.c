#include "pdm.h"
#include "pdm_adapter_priv.h"
#include "pdm_nvmem_ioctl.h"
#include "pdm_nvmem_priv.h"

static struct pdm_adapter *nvmem_adapter = NULL;

/**
 * @brief Sets the state of a specified PDM NVMEM device.
 *
 * @param client Pointer to the PDM client structure.
 * @param state State value (0 or 1).
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_nvmem_read_reg(struct pdm_client *client, unsigned int offset, void *val, size_t bytes)
{
	struct pdm_nvmem_priv *nvmem_priv;
	int status = 0;

	if (!client) {
		OSA_ERROR("Invalid client\n");
		return -EINVAL;
	}

	nvmem_priv = pdm_client_get_private_data(client);
	if (!nvmem_priv) {
		OSA_ERROR("Get PDM Client Device Data Failed\n");
		return -ENOMEM;
	}

	if (!nvmem_priv->ops || !nvmem_priv->ops->read_reg) {
		OSA_ERROR("read_reg not supported\n");
		return -ENOTSUPP;
	}

	status = nvmem_priv->ops->read_reg(client, offset, val, bytes);
	if (status) {
		OSA_ERROR("PDM NVMEM read_reg failed, status: %d\n", status);
		return status;
	}

	return 0;
}

/**
 * @brief Gets the current state of a specified PDM NVMEM device.
 *
 * @param client Pointer to the PDM client structure.
 * @param state Pointer to store the current state.
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_nvmem_write_reg(struct pdm_client *client, unsigned int offset, void *val, size_t bytes)
{
	struct pdm_nvmem_priv *nvmem_priv;
	int status = 0;

	if (!client) {
		OSA_ERROR("Invalid argument\n");
		return -EINVAL;
	}

	nvmem_priv = pdm_client_get_private_data(client);
	if (!nvmem_priv) {
		OSA_ERROR("Get PDM Client Device Data Failed\n");
		return -ENOMEM;
	}

	if (!nvmem_priv->ops || !nvmem_priv->ops->write_reg) {
		OSA_ERROR("write_reg not supported\n");
		return -ENOTSUPP;
	}

	status = nvmem_priv->ops->write_reg(client, offset, val, bytes);
	if (status) {
		OSA_ERROR("PDM NVMEM write_reg failed, status: %d\n", status);
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
static long pdm_nvmem_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
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
		OSA_ERROR("pdm_nvmem_ioctl error\n");
		return status;
	}

	return 0;
}


/**
 * @brief Reads information about available commands or NVMEM state/brightness.
 *
 * @param filp File pointer.
 * @param buf User buffer to write data into.
 * @param count Number of bytes to read.
 * @param ppos Offset in the file.
 * @return Returns number of bytes read or negative error code on failure.
 */
static ssize_t pdm_nvmem_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
	const char help_info[] =
		"Available commands:\n"
		" > 1 <0|1>	- Set NVMEM state\n"
		" > 2		  - Get current NVMEM state\n"
		" > 3 <0-255>  - Set NVMEM brightness\n"
		" > 4		  - Get current NVMEM brightness\n";
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
 * @brief Writes commands to change NVMEM state or brightness.
 *
 * @param filp File pointer.
 * @param buf User buffer containing command data.
 * @param count Number of bytes to write.
 * @param ppos Offset in the file.
 * @return Returns number of bytes written or negative error code on failure.
 */
static ssize_t pdm_nvmem_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
	struct pdm_client *client = filp->private_data;
	char kernel_buf[64];
	ssize_t bytes_read;
	char buffer[32];
	unsigned int offset = 0;
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
		case PDM_NVMEM_CMD_WRITE_REG:
		{
			if (sscanf(kernel_buf, "%d 0x%x 0x%hhx", &cmd, &offset, &value) != 3) {
				OSA_ERROR("Command %d requires one parameter.\n", cmd);
				return -EINVAL;
			}
			break;
		}
		case PDM_NVMEM_CMD_READ_REG:
		{
			if (sscanf(kernel_buf, "%d 0x%x", &cmd, &offset) != 2) {
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
		case PDM_NVMEM_CMD_WRITE_REG:
		{
			if (pdm_nvmem_write_reg(client, offset, &value, 1)) {
				OSA_ERROR("pdm_nvmem_set_state failed\n");
				return -EINVAL;
			}
			break;
		}
		case PDM_NVMEM_CMD_READ_REG:
		{
			if (pdm_nvmem_read_reg(client, offset, &value, 1)) {
				OSA_ERROR("pdm_nvmem_get_state failed\n");
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
 * @brief Initializes the NVMEM client using match data.
 *
 * @param client Pointer to the PDM client structure.
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_nvmem_match_setup(struct pdm_client *client)
{
	struct pdm_nvmem_priv *nvmem_priv;
	const void *match_data;
	int status;

	if (!client) {
		return -EINVAL;
	}

	nvmem_priv = pdm_client_get_private_data(client);
	if (!nvmem_priv) {
		OSA_ERROR("NVMEM Client get private data is NULL\n");
		return -ENOMEM;
	}

	match_data = pdm_client_get_match_data(client);
	if (!match_data) {
		OSA_ERROR("Failed to get match data for device\n");
		return -ENODEV;
	}

	nvmem_priv->match_data = match_data;
	if (nvmem_priv->match_data->setup) {
		status = nvmem_priv->match_data->setup(client);
		if (status) {
			OSA_ERROR("NVMEM Client Setup Failed, status=%d\n", status);
			return status;
		}
	}

	return 0;
}

/**
 * @brief Probes the NVMEM PDM device.
 *
 * This function is called when a PDM device is detected and adds the device to the main device.
 *
 * @param pdmdev Pointer to the PDM device.
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_nvmem_device_probe(struct pdm_device *pdmdev)
{
	struct pdm_client *client;
	int status;

	client = devm_pdm_client_alloc(pdmdev, sizeof(struct pdm_nvmem_priv));
	if (IS_ERR(client)) {
		OSA_ERROR("NVMEM Client Alloc Failed\n");
		return PTR_ERR(client);
	}

	status = devm_pdm_client_register(nvmem_adapter, client);
	if (status) {
		OSA_ERROR("NVMEM Adapter Add Device Failed, status=%d\n", status);
		return status;
	}

	status = pdm_nvmem_match_setup(client);
	if (status) {
		OSA_ERROR("NVMEM Client Setup Failed, status=%d\n", status);
		return status;
	}

	client->fops.read = pdm_nvmem_read;
	client->fops.write = pdm_nvmem_write;
	client->fops.unlocked_ioctl = pdm_nvmem_ioctl;

	return 0;
}

/**
 * @brief Removes the NVMEM PDM device.
 *
 * This function is called when a PDM device is removed and deletes the device from the main device.
 *
 * @param pdmdev Pointer to the PDM device.
 */
static void pdm_nvmem_device_remove(struct pdm_device *pdmdev)
{
	if (pdmdev && pdmdev->client) {
		pdm_nvmem_match_setup(pdmdev->client);
	}
}

/**
 * @brief Match data structure for initializing GPIO type NVMEM devices.
 */
static const struct pdm_nvmem_match_data pdm_nvmem_spi_match_data = {
	.setup = pdm_nvmem_spi_setup,
	.cleanup = NULL,
};

/**
 * @brief Match data structure for initializing PWM type NVMEM devices.
 */
static const struct pdm_nvmem_match_data pdm_nvmem_i2c_match_data = {
	.setup = NULL,
	.cleanup = NULL,
};

/**
 * @brief Device tree match table.
 *
 * Defines the supported device tree compatible properties.
 */
static const struct of_device_id of_pdm_nvmem_match[] = {
	{ .compatible = "pdm-nvmem-spi",	 .data = &pdm_nvmem_spi_match_data},
	{ .compatible = "pdm-nvmem-i2c",	 .data = &pdm_nvmem_i2c_match_data},
	{},
};
MODULE_DEVICE_TABLE(of, of_pdm_nvmem_match);

/**
 * @brief NVMEM PDM driver structure.
 *
 * Defines the basic information and operation functions of the NVMEM PDM driver.
 */
static struct pdm_driver pdm_nvmem_driver = {
	.probe = pdm_nvmem_device_probe,
	.remove = pdm_nvmem_device_remove,
	.driver = {
		.name = "pdm-nvmem",
		.of_match_table = of_pdm_nvmem_match,
	},
};

/**
 * @brief Initializes the NVMEM PDM adapter driver.
 *
 * Allocates and registers the adapter and driver.
 *
 * @return Returns 0 on success; negative error code on failure.
 */
int pdm_nvmem_driver_init(void)
{
	int status;

	nvmem_adapter = pdm_adapter_alloc(sizeof(void *));
	if (!nvmem_adapter) {
		OSA_ERROR("Failed to allocate pdm_adapter\n");
		return -ENOMEM;
	}

	status = pdm_adapter_register(nvmem_adapter, PDM_NVMEM_NAME);
	if (status) {
		OSA_ERROR("Failed to register NVMEM PDM Adapter, status=%d\n", status);
		return status;
	}

	status = pdm_bus_register_driver(THIS_MODULE, &pdm_nvmem_driver);
	if (status) {
		OSA_ERROR("Failed to register NVMEM PDM Driver, status=%d\n", status);
		goto err_adapter_unregister;
	}

	return 0;

err_adapter_unregister:
	pdm_adapter_unregister(nvmem_adapter);
	return status;
}

/**
 * @brief Exits the NVMEM PDM adapter driver.
 *
 * Unregisters the driver and adapter, releasing related resources.
 */
void pdm_nvmem_driver_exit(void)
{
	pdm_bus_unregister_driver(&pdm_nvmem_driver);
	pdm_adapter_unregister(nvmem_adapter);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("NVMEM PDM Adapter Driver");
