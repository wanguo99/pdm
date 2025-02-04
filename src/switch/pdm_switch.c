#include "pdm.h"
#include "pdm_adapter_priv.h"
#include "pdm_switch_ioctl.h"
#include "pdm_switch_priv.h"

static struct pdm_adapter *switch_adapter = NULL;


/**
 * @brief Sets the state of a specified PDM SWITCH device.
 *
 * @param client Pointer to the PDM client structure.
 * @param state State value (0 or 1).
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_switch_set_state(struct pdm_client *client, int state)
{
	struct pdm_switch_priv *switch_priv;
	int status = 0;

	if (!client) {
		OSA_ERROR("Invalid client\n");
		return -EINVAL;
	}

	switch_priv = pdm_client_get_private_data(client);
	if (!switch_priv) {
		OSA_ERROR("Get PDM Client Device Data Failed\n");
		return -ENOMEM;
	}

	if (!switch_priv->set_state) {
		OSA_ERROR("set_state not supported\n");
		return -ENOTSUPP;
	}

	status = switch_priv->set_state(client, state);
	if (status) {
		OSA_ERROR("PDM Switch set_state failed, status: %d\n", status);
		return status;
	}

	return 0;
}

/**
 * @brief Gets the current state of a specified PDM SWITCH device.
 *
 * @param client Pointer to the PDM client structure.
 * @param state Pointer to store the current state.
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_switch_get_state(struct pdm_client *client, int *state)
{
	struct pdm_switch_priv *switch_priv;
	int status = 0;

	if (!client || !state) {
		OSA_ERROR("Invalid argument\n");
		return -EINVAL;
	}

	switch_priv = pdm_client_get_private_data(client);
	if (!switch_priv) {
		OSA_ERROR("Get PDM Client Device Data Failed\n");
		return -ENOMEM;
	}

	if (!switch_priv->get_state) {
		OSA_ERROR("get_state not supported\n");
		return -ENOTSUPP;
	}

	status = switch_priv->get_state(client, state);
	if (status) {
		OSA_ERROR("PDM Switch get_state failed, status: %d\n", status);
		return status;
	}

	OSA_DEBUG("Current state is %s\n", *state ? "ON" : "OFF");
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
static long pdm_switch_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct pdm_client *client = filp->private_data;
	int status = 0;

	if (!client) {
		OSA_ERROR("Invalid client\n");
		return -EINVAL;
	}

	switch (cmd) {
		case PDM_SWITCH_SET_STATE:
		{
			int state;
			if (copy_from_user(&state, (void __user *)arg, sizeof(state))) {
				OSA_ERROR("Failed to copy data from user space\n");
				return -EFAULT;
			}
			status = pdm_switch_set_state(client, state);
			break;
		}
		case PDM_SWITCH_GET_STATE:
		{
			int state;
			status = pdm_switch_get_state(client, &state);
			if (status) {
				OSA_ERROR("Failed to get SWITCH state, status: %d\n", status);
				return status;
			}
			if (copy_to_user((void __user *)arg, &state, sizeof(state))) {
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
		OSA_ERROR("pdm_switch_ioctl error\n");
		return status;
	}

	return 0;
}


/**
 * @brief Reads information about available commands or SWITCH state.
 *
 * @param filp File pointer.
 * @param buf User buffer to write data into.
 * @param count Number of bytes to read.
 * @param ppos Offset in the file.
 * @return Returns number of bytes read or negative error code on failure.
 */
static ssize_t pdm_switch_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
	const char help_info[] =
		"Available commands:\n"
		" > 1 <0|1>	- Set SWITCH state\n"
		" > 2		- Get current SWITCH state\n";
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


static void pdm_switch_write_help(void)
{
    OSA_print("PDM Switch Module Test Unit\n");
    OSA_print("Usage: pdm_test_switch [-h] [-s <state>] [-g]\n");
    OSA_print("\n");
    OSA_print("Options:\n");
    OSA_print("  -h, --help      Show this help message and exit.\n");
    OSA_print("  -s <state>      Set the switch state to the specified integer value.\n");
    OSA_print("                  Example: -s 1 (to set state to 1)\n");
    OSA_print("  -g              Get the current switch state.\n");
    OSA_print("\n");
    OSA_print("Examples:\n");
    OSA_print("  pdm_test_switch -s 1   # Set the switch state to 1\n");
    OSA_print("  pdm_test_switch -g     # Get the current switch state\n");
}

/**
 * @brief Writes commands to change SWITCH state.
 *
 * @param filp File pointer.
 * @param buf User buffer containing command data.
 * @param count Number of bytes to write.
 * @param ppos Offset in the file.
 * @return Returns number of bytes written or negative error code on failure.
 */
static ssize_t pdm_switch_write(struct file *filp, const char __user *buf,
size_t count, loff_t *ppos)
{
	struct pdm_client *client = filp->private_data;
	char kernel_buf[64];
	ssize_t bytes_read;
	char cmd[4];
	int state;

	if (!client || count >= sizeof(kernel_buf)) {
		OSA_ERROR("Invalid client or input too long.\n");
		return -EINVAL;
	}

	if ((bytes_read = copy_from_user(kernel_buf, buf, count)) != 0) {
		OSA_ERROR("Failed to copy data from user space: %zd\n", bytes_read);
		return -EFAULT;
	}

	kernel_buf[count] = '\0';

	if (sscanf(kernel_buf, "%2s", cmd) != 1) {
		OSA_ERROR("Invalid command format: %s\n", kernel_buf);
		return -EINVAL;
	}

	if (!strcmp(cmd, "-s")) {
		if (sscanf(kernel_buf, "%2s %d", cmd, &state) != 2) {
			OSA_ERROR("Command %s requires one parameter.\n", cmd);
			return -EINVAL;
		}
		if (state != 0 && state != 1) {
			OSA_ERROR("Invalid state: %d\n", state);
			return -EINVAL;
		}
		if (pdm_switch_set_state(client, state)) {
			OSA_ERROR("pdm_switch_set_state failed\n");
			return -EINVAL;
		}
		return count;
	}
	else if (!strcmp(cmd, "-g")) {
		if (sscanf(kernel_buf, "%2s", cmd) != 1) {
			OSA_ERROR("Command %s should not have parameters.\n", cmd);
			return -EINVAL;
		}
		if (pdm_switch_get_state(client, &state)) {
			OSA_ERROR("pdm_switch_get_state failed\n");
			return -EINVAL;
		}
		OSA_INFO("Current state is %s\n", state ? "ON" : "OFF");
		return count;
	}
	else {
		pdm_switch_write_help();
		return count;
	}

	return count;
}

/**
 * @brief Probes the SWITCH PDM device.
 *
 * This function is called when a PDM device is detected and adds the device to the main device.
 *
 * @param pdmdev Pointer to the PDM device.
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_switch_device_probe(struct pdm_device *pdmdev)
{
	struct pdm_client *client;
	int status;

	client = devm_pdm_client_alloc(pdmdev, sizeof(struct pdm_switch_priv));
	if (IS_ERR(client)) {
		OSA_ERROR("SWITCH Client Alloc Failed\n");
		return PTR_ERR(client);
	}

	status = devm_pdm_client_register(switch_adapter, client);
	if (status) {
		OSA_ERROR("SWITCH Adapter Add Device Failed, status=%d\n", status);
		return status;
	}

	status = pdm_client_setup(client);
	if (status) {
		OSA_ERROR("SWITCH Client Setup Failed, status=%d\n", status);
		return status;
	}

	return 0;
}

/**
 * @brief Removes the SWITCH PDM device.
 *
 * This function is called when a PDM device is removed and deletes the device from the main device.
 *
 * @param pdmdev Pointer to the PDM device.
 */
static void pdm_switch_device_remove(struct pdm_device *pdmdev)
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
static const struct of_device_id of_pdm_switch_match[] = {
	{ .compatible = "pdm-switch-gpio",	 .data = &pdm_switch_gpio_match_data},
	{},
};
MODULE_DEVICE_TABLE(of, of_pdm_switch_match);

/**
 * @brief SWITCH PDM driver structure.
 *
 * Defines the basic information and operation functions of the SWITCH PDM driver.
 */
static struct pdm_driver pdm_switch_driver = {
	.probe = pdm_switch_device_probe,
	.remove = pdm_switch_device_remove,
	.driver = {
		.name = "pdm-switch",
		.of_match_table = of_pdm_switch_match,
	},
};

/**
 * @brief Initializes the SWITCH PDM adapter driver.
 *
 * Allocates and registers the adapter and driver.
 *
 * @return Returns 0 on success; negative error code on failure.
 */
int pdm_switch_driver_init(void)
{
	int status;

	switch_adapter = pdm_adapter_alloc(sizeof(void *));
	if (!switch_adapter) {
		OSA_ERROR("Failed to allocate pdm_adapter\n");
		return -ENOMEM;
	}

	status = pdm_adapter_register(switch_adapter, PDM_SWITCH_NAME);
	if (status) {
		OSA_ERROR("Failed to register SWITCH PDM Adapter, status=%d\n", status);
		return status;
	}

	status = pdm_bus_register_driver(THIS_MODULE, &pdm_switch_driver);
	if (status) {
		OSA_ERROR("Failed to register SWITCH PDM Driver, status=%d\n", status);
		goto err_adapter_unregister;
	}

	switch_adapter->fops.read = pdm_switch_read;
	switch_adapter->fops.write = pdm_switch_write;
	switch_adapter->fops.unlocked_ioctl = pdm_switch_ioctl;

	return 0;

err_adapter_unregister:
	pdm_adapter_unregister(switch_adapter);
	return status;
}

/**
 * @brief Exits the SWITCH PDM adapter driver.
 *
 * Unregisters the driver and adapter, releasing related resources.
 */
void pdm_switch_driver_exit(void)
{
	pdm_bus_unregister_driver(&pdm_switch_driver);
	pdm_adapter_unregister(switch_adapter);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("SWITCH PDM Adapter Driver");
