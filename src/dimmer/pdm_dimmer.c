#include "pdm.h"
#include "pdm_adapter_priv.h"
#include "pdm_dimmer_ioctl.h"
#include "pdm_dimmer_priv.h"

static struct pdm_adapter *dimmer_adapter = NULL;

/**
 * @brief Sets the level of a specified PDM DIMMER device.
 *
 * @param client Pointer to the PDM client structure.
 * @param level (0-255).
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_dimmer_set_level(struct pdm_client *client, unsigned int level)
{
	struct pdm_dimmer_priv *dimmer_priv;
	int status = 0;

	if (!client) {
		OSA_ERROR("Invalid client\n");
		return -EINVAL;
	}

	if (level > PDM_DIMMER_MAX_LEVEL_VALUE) {
		OSA_ERROR("Invalid level: %u\n", level);
		return -EINVAL;
	}

	dimmer_priv = pdm_client_get_private_data(client);
	if (!dimmer_priv) {
		OSA_ERROR("Get PDM Client Device Data Failed\n");
		return -ENOMEM;
	}

	if (!dimmer_priv->set_level) {
		OSA_ERROR("set_level not supported\n");
		return -ENOTSUPP;
	}

	status = dimmer_priv->set_level(client, level);
	if (status) {
		OSA_ERROR("PDM Dimmer set_level failed, status: %d\n", status);
		return status;
	}

	return 0;
}

/**
 * @brief Gets the current level of a specified PDM DIMMER device.
 *
 * @param client Pointer to the PDM client structure.
 * @param level Pointer to store the current level.
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_dimmer_get_level(struct pdm_client *client, unsigned int *level)
{
	struct pdm_dimmer_priv *dimmer_priv;
	int status = 0;

	if (!client || !level) {
		OSA_ERROR("Invalid argument\n");
		return -EINVAL;
	}

	dimmer_priv = pdm_client_get_private_data(client);
	if (!dimmer_priv) {
		OSA_ERROR("Get PDM Client Device Data Failed\n");
		return -ENOMEM;
	}

	if (!dimmer_priv->get_level) {
		OSA_ERROR("get_level not supported\n");
		return -ENOTSUPP;
	}

	status = dimmer_priv->get_level(client, level);
	if (status) {
		OSA_ERROR("PDM Dimmer get_level failed, status: %d\n", status);
		return status;
	}

	OSA_INFO("Current level is %u\n", *level);

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
static long pdm_dimmer_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct pdm_client *client = filp->private_data;
	unsigned int level;
	int status = 0;

	if (!client) {
		OSA_ERROR("Invalid client\n");
		return -EINVAL;
	}

	switch (cmd) {
		case PDM_DIMMER_CMD_SET_LEVEL:
		{
			if (copy_from_user(&level, (void __user *)arg, sizeof(level))) {
				OSA_ERROR("Failed to copy data from user space\n");
				return -EFAULT;
			}
			OSA_INFO("PDM_DIMMER: Set %s's level to %u\n", dev_name(&client->dev), level);
			status = pdm_dimmer_set_level(client, level);
			break;
		}
		case PDM_DIMMER_CMD_GET_LEVEL:
		{
			status = pdm_dimmer_get_level(client, &level);
			if (status) {
				OSA_ERROR("Failed to get DIMMER level, status: %d\n", status);
				return status;
			}
			OSA_INFO("PDM_DIMMER: Current level is %u\n", level);
			if (copy_to_user((void __user *)arg, &level, sizeof(level))) {
				OSA_ERROR("Failed to copy data to user space\n");
				return -EFAULT;
			}
			break;
		}
		default:
		{
			OSA_ERROR("Unknown ioctl command\n");
			return -ENOTTY;
		}
	}

	if (status) {
		OSA_ERROR("pdm_dimmer_ioctl error\n");
		return status;
	}

	return 0;
}


/**
 * @brief Reads information about available commands or DIMMER level.
 *
 * @param filp File pointer.
 * @param buf User buffer to write data into.
 * @param count Number of bytes to read.
 * @param ppos Offset in the file.
 * @return Returns number of bytes read or negative error code on failure.
 */
static ssize_t pdm_dimmer_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
	const char help_info[] =
		"Available commands:\n"
		" > 1 <0-255>	- Set DIMMER level\n"
		" > 2		- Get current DIMMER level\n";
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
 * @brief Writes commands to change DIMMER level.
 *
 * @param filp File pointer.
 * @param buf User buffer containing command data.
 * @param count Number of bytes to write.
 * @param ppos Offset in the file.
 * @return Returns number of bytes written or negative error code on failure.
 */
static ssize_t pdm_dimmer_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
	struct pdm_client *client = filp->private_data;
	char kernel_buf[64];
	ssize_t bytes_read;
	unsigned int level;
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
		case PDM_DIMMER_CMD_SET_LEVEL:
		{
			if (sscanf(kernel_buf, "%d %u", &cmd, &level) != 2) {
				OSA_ERROR("Command %d requires one parameter.\n", cmd);
				return -EINVAL;
			}

			if (pdm_dimmer_set_level(client, level)) {
				OSA_ERROR("pdm_dimmer_set_level failed\n");
				return -EINVAL;
			}
			break;
		}
		case PDM_DIMMER_CMD_GET_LEVEL:
		{
			if (pdm_dimmer_get_level(client, &level)) {
				OSA_ERROR("pdm_dimmer_get_level failed\n");
				return -EINVAL;
			}
			char buffer[32];
			snprintf(buffer, sizeof(buffer), "%u\n", level);
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
 * @brief Probes the DIMMER PDM device.
 *
 * This function is called when a PDM device is detected and adds the device to the main device.
 *
 * @param pdmdev Pointer to the PDM device.
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_dimmer_device_probe(struct pdm_device *pdmdev)
{
	struct pdm_client *client;
	int status;

	client = devm_pdm_client_alloc(pdmdev, sizeof(struct pdm_dimmer_priv));
	if (IS_ERR(client)) {
		OSA_ERROR("DIMMER Client Alloc Failed\n");
		return PTR_ERR(client);
	}

	status = devm_pdm_client_register(dimmer_adapter, client);
	if (status) {
		OSA_ERROR("DIMMER Adapter Add Device Failed, status=%d\n", status);
		return status;
	}

	status = pdm_client_setup(client);
	if (status) {
		OSA_ERROR("DIMMER Client Setup Failed, status=%d\n", status);
		return status;
	}

	client->fops.read = pdm_dimmer_read;
	client->fops.write = pdm_dimmer_write;
	client->fops.unlocked_ioctl = pdm_dimmer_ioctl;

	return 0;
}

/**
 * @brief Removes the DIMMER PDM device.
 *
 * This function is called when a PDM device is removed and deletes the device from the main device.
 *
 * @param pdmdev Pointer to the PDM device.
 */
static void pdm_dimmer_device_remove(struct pdm_device *pdmdev)
{
	if (pdmdev && pdmdev->client) {
		pdm_client_cleanup(pdmdev->client);
	}
}

/**
 * @brief Device tree match table.
 *
 * Defines the supported device tree compatible properties.
 */
static const struct of_device_id of_pdm_dimmer_match[] = {
	{ .compatible = "pdm-dimmer-pwm",	  .data = &pdm_dimmer_pwm_match_data},
	{},
};
MODULE_DEVICE_TABLE(of, of_pdm_dimmer_match);

/**
 * @brief DIMMER PDM driver structure.
 *
 * Defines the basic information and operation functions of the DIMMER PDM driver.
 */
static struct pdm_driver pdm_dimmer_driver = {
	.probe = pdm_dimmer_device_probe,
	.remove = pdm_dimmer_device_remove,
	.driver = {
		.name = "pdm-dimmer",
		.of_match_table = of_pdm_dimmer_match,
	},
};

/**
 * @brief Initializes the DIMMER PDM adapter driver.
 *
 * Allocates and registers the adapter and driver.
 *
 * @return Returns 0 on success; negative error code on failure.
 */
int pdm_dimmer_driver_init(void)
{
	int status;

	dimmer_adapter = pdm_adapter_alloc(sizeof(void *));
	if (!dimmer_adapter) {
		OSA_ERROR("Failed to allocate pdm_adapter\n");
		return -ENOMEM;
	}

	status = pdm_adapter_register(dimmer_adapter, PDM_DIMMER_NAME);
	if (status) {
		OSA_ERROR("Failed to register DIMMER PDM Adapter, status=%d\n", status);
		return status;
	}

	status = pdm_bus_register_driver(THIS_MODULE, &pdm_dimmer_driver);
	if (status) {
		OSA_ERROR("Failed to register DIMMER PDM Driver, status=%d\n", status);
		goto err_adapter_unregister;
	}

	return 0;

err_adapter_unregister:
	pdm_adapter_unregister(dimmer_adapter);
	return status;
}

/**
 * @brief Exits the DIMMER PDM adapter driver.
 *
 * Unregisters the driver and adapter, releasing related resources.
 */
void pdm_dimmer_driver_exit(void)
{
	pdm_bus_unregister_driver(&pdm_dimmer_driver);
	pdm_adapter_unregister(dimmer_adapter);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("DIMMER PDM Adapter Driver");
