#include "pdm.h"
#include "pdm_adapter_priv.h"
#include "pdm_sensor_ioctl.h"
#include "pdm_sensor_priv.h"

static struct pdm_adapter *sensor_adapter = NULL;

/**
 * @brief Sets the state of a specified PDM SENSOR device.
 *
 * @param client Pointer to the PDM client structure.
 * @param state State value (0 or 1).
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_sensor_read_reg(struct pdm_client *client, unsigned int offset, void *val, size_t bytes)
{
	struct pdm_sensor_priv *sensor_priv;
	int status = 0;

	if (!client) {
		OSA_ERROR("Invalid client\n");
		return -EINVAL;
	}

	sensor_priv = pdm_client_get_private_data(client);
	if (!sensor_priv) {
		OSA_ERROR("Get PDM Client Device Data Failed\n");
		return -ENOMEM;
	}

	if (!sensor_priv->read_reg) {
		OSA_ERROR("read_reg not supported\n");
		return -ENOTSUPP;
	}

	status = sensor_priv->read_reg(client, offset, val, bytes);
	if (status) {
		OSA_ERROR("PDM SENSOR read_reg failed, status: %d\n", status);
		return status;
	}

	return 0;
}

/**
 * @brief Gets the current state of a specified PDM SENSOR device.
 *
 * @param client Pointer to the PDM client structure.
 * @param state Pointer to store the current state.
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_sensor_write_reg(struct pdm_client *client, unsigned int offset, void *val, size_t bytes)
{
	struct pdm_sensor_priv *sensor_priv;
	int status = 0;

	if (!client) {
		OSA_ERROR("Invalid argument\n");
		return -EINVAL;
	}

	sensor_priv = pdm_client_get_private_data(client);
	if (!sensor_priv) {
		OSA_ERROR("Get PDM Client Device Data Failed\n");
		return -ENOMEM;
	}

	if (!sensor_priv->write_reg) {
		OSA_ERROR("write_reg not supported\n");
		return -ENOTSUPP;
	}

	status = sensor_priv->write_reg(client, offset, val, bytes);
	if (status) {
		OSA_ERROR("PDM SENSOR write_reg failed, status: %d\n", status);
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
static long pdm_sensor_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct pdm_client *client = filp->private_data;
	int status = 0;

	if (!client) {
		OSA_ERROR("Invalid client\n");
		return -EINVAL;
	}

	switch (cmd) {
		case PDM_SENSOR_CMD_WRITE_REG:
		{
			int level;
			if (copy_from_user(&level, (void __user *)arg, sizeof(level))) {
				OSA_ERROR("Failed to copy data from user space\n");
				return -EFAULT;
			}
			OSA_INFO("PDM_DIMMER: Set %s's level to %d\n", dev_name(&client->dev), level);
			status = pdm_sensor_write_reg(client, 0, 0, 0);
			break;
		}
		case PDM_SENSOR_CMD_READ_REG:
		{
			int level;
			status = pdm_sensor_read_reg(client, 0, &level, 0);
			if (status) {
				OSA_ERROR("Failed to get DIMMER level, status: %d\n", status);
				return status;
			}
			OSA_INFO("PDM_DIMMER: Current level is %d\n", level);
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
		OSA_ERROR("pdm_sensor_ioctl error\n");
		return status;
	}

	return 0;
}


/**
 * @brief Reads information about available commands or SENSOR state/brightness.
 *
 * @param filp File pointer.
 * @param buf User buffer to write data into.
 * @param count Number of bytes to read.
 * @param ppos Offset in the file.
 * @return Returns number of bytes read or negative error code on failure.
 */
static ssize_t pdm_sensor_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
	const char help_info[] =
		"Available commands:\n"
		" > 1		- Read SENSOR Reg\n"
		" > 2		- Write SENSOR Reg\n";
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
 * @brief Writes commands to change SENSOR state or brightness.
 *
 * @param filp File pointer.
 * @param buf User buffer containing command data.
 * @param count Number of bytes to write.
 * @param ppos Offset in the file.
 * @return Returns number of bytes written or negative error code on failure.
 */
static ssize_t pdm_sensor_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
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
		case PDM_SENSOR_CMD_WRITE_REG:
		{
			if (sscanf(kernel_buf, "%d 0x%x 0x%hhx", &cmd, &offset, &value) != 3) {
				OSA_ERROR("Command %d requires one parameter.\n", cmd);
				return -EINVAL;
			}

			if (pdm_sensor_write_reg(client, offset, &value, 1)) {
				OSA_ERROR("pdm_sensor_set_state failed\n");
				return -EINVAL;
			}
			break;
		}
		case PDM_SENSOR_CMD_READ_REG:
		{
			if (sscanf(kernel_buf, "%d 0x%x", &cmd, &offset) != 2) {
				OSA_ERROR("Command %d should not have parameters.\n", cmd);
				return -EINVAL;
			}

			if (pdm_sensor_read_reg(client, offset, &value, 1)) {
				OSA_ERROR("pdm_sensor_get_state failed\n");
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
 * @brief Probes the SENSOR PDM device.
 *
 * This function is called when a PDM device is detected and adds the device to the main device.
 *
 * @param pdmdev Pointer to the PDM device.
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_sensor_device_probe(struct pdm_device *pdmdev)
{
	struct pdm_client *client;
	int status;

	client = devm_pdm_client_alloc(pdmdev, sizeof(struct pdm_sensor_priv));
	if (IS_ERR(client)) {
		OSA_ERROR("SENSOR Client Alloc Failed\n");
		return PTR_ERR(client);
	}

	status = devm_pdm_client_register(sensor_adapter, client);
	if (status) {
		OSA_ERROR("SENSOR Adapter Add Device Failed, status=%d\n", status);
		return status;
	}

	status = pdm_client_setup(client);
	if (status) {
		OSA_ERROR("DIMMER Client Setup Failed, status=%d\n", status);
		return status;
	}

	client->fops.read = pdm_sensor_read;
	client->fops.write = pdm_sensor_write;
	client->fops.unlocked_ioctl = pdm_sensor_ioctl;

	return 0;
}

/**
 * @brief Removes the SENSOR PDM device.
 *
 * This function is called when a PDM device is removed and deletes the device from the main device.
 *
 * @param pdmdev Pointer to the PDM device.
 */
static void pdm_sensor_device_remove(struct pdm_device *pdmdev)
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
static const struct of_device_id of_pdm_sensor_match[] = {
	{ .compatible = "pdm-sensor-ap3216c",	 .data = &pdm_sensor_ap3216c_match_data},
	{},
};
MODULE_DEVICE_TABLE(of, of_pdm_sensor_match);

/**
 * @brief SENSOR PDM driver structure.
 *
 * Defines the basic information and operation functions of the SENSOR PDM driver.
 */
static struct pdm_driver pdm_sensor_driver = {
	.probe = pdm_sensor_device_probe,
	.remove = pdm_sensor_device_remove,
	.driver = {
		.name = "pdm-sensor",
		.of_match_table = of_pdm_sensor_match,
	},
};

/**
 * @brief Initializes the SENSOR PDM adapter driver.
 *
 * Allocates and registers the adapter and driver.
 *
 * @return Returns 0 on success; negative error code on failure.
 */
int pdm_sensor_driver_init(void)
{
	int status;

	sensor_adapter = pdm_adapter_alloc(sizeof(void *));
	if (!sensor_adapter) {
		OSA_ERROR("Failed to allocate pdm_adapter\n");
		return -ENOMEM;
	}

	status = pdm_adapter_register(sensor_adapter, PDM_SENSOR_NAME);
	if (status) {
		OSA_ERROR("Failed to register SENSOR PDM Adapter, status=%d\n", status);
		return status;
	}

	status = pdm_bus_register_driver(THIS_MODULE, &pdm_sensor_driver);
	if (status) {
		OSA_ERROR("Failed to register SENSOR PDM Driver, status=%d\n", status);
		goto err_adapter_unregister;
	}

	return 0;

err_adapter_unregister:
	pdm_adapter_unregister(sensor_adapter);
	return status;
}

/**
 * @brief Exits the SENSOR PDM adapter driver.
 *
 * Unregisters the driver and adapter, releasing related resources.
 */
void pdm_sensor_driver_exit(void)
{
	pdm_bus_unregister_driver(&pdm_sensor_driver);
	pdm_adapter_unregister(sensor_adapter);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("SENSOR PDM Adapter Driver");
