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
static int pdm_sensor_read_data(struct pdm_client *client, unsigned int type, unsigned int *val)
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

	if (!sensor_priv->read) {
		OSA_ERROR("read_reg not supported\n");
		return -ENOTSUPP;
	}

	status = sensor_priv->read(client, type, val);
	if (status) {
		OSA_ERROR("PDM SENSOR read_reg failed, status: %d\n", status);
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
	struct pdm_sensor_ioctl_data __user *user_data = (struct pdm_sensor_ioctl_data __user *)arg;
	struct pdm_sensor_ioctl_data data;

	if (!client) {
		OSA_ERROR("Invalid client\n");
		return -EINVAL;
	}

	switch (cmd) {
	case PDM_SENSOR_READ_REG:
	{
		// Copy the data structure from user space to kernel space
		if (copy_from_user(&data, user_data, sizeof(data))) {
			OSA_ERROR("Failed to copy data from user space\n");
			return -EFAULT;
		}

		// Perform the read operation
		status = pdm_sensor_read_data(client, data.type, &data.value);
		if (status) {
			OSA_ERROR("Failed to read sensor register: %d\n", status);
			return status;
		}

		// Copy the result back to user space
		if (copy_to_user(user_data, &data, sizeof(data))) {
			OSA_ERROR("Failed to copy data to user space\n");
			return -EFAULT;
		}
		break;
	}
	default:
	{
		OSA_ERROR("Unknown ioctl command: 0x%x\n", cmd);
		return -ENOTTY;
	}
	}

	if (status) {
		OSA_ERROR("pdm_sensor_ioctl error: %d\n", status);
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
		" > echo 1 type > /dev/pdm_sensor - Read SENSOR\n";
	size_t len = strlen(help_info);

	if (*ppos >= len)
		return 0;

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
static ssize_t pdm_sensor_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
	struct pdm_client *client = filp->private_data;
	char kernel_buf[64];
	ssize_t bytes_read;
	unsigned int type;
	unsigned int value;
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
	if (sscanf(kernel_buf, "%d %u", &cmd, &type) != 2) {
		OSA_ERROR("Command %d requires one parameter.\n", cmd);
		return -EINVAL;
	}

	switch (cmd) {
		case PDM_SENSOR_CMD_READ: {
			value = 0;
			if (pdm_sensor_read_data(client, type, &value)) {
				OSA_ERROR("pdm_dimmer_set_level failed\n");
				return -EINVAL;
			}
			break;
		}
		default: {
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
