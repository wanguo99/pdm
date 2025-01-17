#include <linux/compat.h>
#include "pdm.h"

/**
 * @brief Constructs the device node path for PDM Client devices.
 *
 * This function generates the full path for the device node, prefixed with "pdm_client/".
 *
 * @param dev Pointer to the device structure.
 * @param mode Pointer to the file permissions mode (optional).
 *
 * @return Dynamically allocated string of the full device node path. Caller must free with kfree().
 */
static char *pdm_client_devnode(const struct device *dev, umode_t *mode)
{
	return kasprintf(GFP_KERNEL, "pdm_client/%s", dev_name(dev));
}

/**
 * @brief PDM Client class.
 */
static struct class pdm_client_class = {
	.name = "pdm_client",
	.devnode = pdm_client_devnode,
};

/**
 * @brief PDM Client major device number.
 */
static dev_t pdm_client_major;

/**
 * @brief Default open function.
 *
 * This function is called when the device file is opened.
 *
 * @param inode Pointer to the inode structure.
 * @param filp Pointer to the file structure.
 *
 * @return 0 on success, negative error code on failure.
 */
static int pdm_client_fops_default_open(struct inode *inode, struct file *filp)
{
	struct pdm_client *client = container_of(inode->i_cdev, struct pdm_client, cdev);

	if (!client) {
		OSA_ERROR("Invalid client\n");
		return -EINVAL;
	}

	filp->private_data = client;
	return 0;
}

/**
 * @brief Default release function.
 *
 * This function is called when the device file is closed.
 *
 * @param inode Pointer to the inode structure.
 * @param filp Pointer to the file structure.
 *
 * @return 0 on success.
 */
static int pdm_client_fops_default_release(struct inode *inode, struct file *filp)
{
	return 0;
}

/**
 * @brief Default read function.
 *
 * This function is called when data is read from the device file.
 *
 * @param filp Pointer to the file structure.
 * @param buf User-space buffer to read into.
 * @param count Number of bytes to read.
 * @param ppos Current file position.
 *
 * @return Number of bytes read, or negative error code on failure.
 */
static ssize_t pdm_client_fops_default_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
	return 0;
}

/**
 * @brief Default write function.
 *
 * This function is called when data is written to the device file.
 *
 * @param filp Pointer to the file structure.
 * @param buf User-space buffer to write from.
 * @param count Number of bytes to write.
 * @param ppos Current file position.
 *
 * @return Number of bytes written, or negative error code on failure.
 */
static ssize_t pdm_client_fops_default_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
	return count;
}

/**
 * @brief Default ioctl function.
 *
 * This function handles ioctl operations.
 *
 * @param filp Pointer to the file structure.
 * @param cmd Ioctl command.
 * @param arg Command argument.
 *
 * @return 0 on success, negative error code on failure.
 */
static long pdm_client_fops_default_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	OSA_INFO("This client does not support ioctl operations\n");
	return -ENOTSUPP;
}

/**
 * @brief Default compat ioctl function.
 *
 * This function handles compatibility layer ioctl operations.
 *
 * @param filp Pointer to the file structure.
 * @param cmd Ioctl command.
 * @param arg Command argument.
 *
 * @return Result of underlying unlocked_ioctl function.
 */
static long pdm_client_fops_default_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	OSA_INFO("pdm_client_fops_default_compat_ioctl for %s\n", dev_name(filp->private_data));

	if (_IOC_DIR(cmd) & (_IOC_READ | _IOC_WRITE)) {
		arg = (unsigned long)compat_ptr(arg);
	}

	return filp->f_op->unlocked_ioctl(filp, cmd, arg);
}


/**
 * @brief Registers a PDM Client character device.
 *
 * This function registers a new PDM Client character device with the system.
 *
 * @param client Pointer to the PDM Client.
 * @return 0 on success, negative error code on failure.
 */
static int pdm_client_device_register(struct pdm_client *client)
{
	int status;

	if (!client || !client->adapter) {
		OSA_ERROR("Invalid input parameter\n");
		return -EINVAL;
	}

	if (client->pdmdev->index >= PDM_CLIENT_MINORS) {
		OSA_ERROR("Out of pdm_client minors (%d)\n", client->pdmdev->index);
		return -ENODEV;
	}

	client->dev.devt = MKDEV(pdm_client_major, client->pdmdev->index);
	status = dev_set_name(&client->dev, "%s.%d", dev_name(&client->adapter->dev), client->index);
	if (status) {
		OSA_ERROR("Failed to set client name, error:%d\n", status);
		return status;
	}

	client->fops.open = pdm_client_fops_default_open;
	client->fops.release = pdm_client_fops_default_release;
	client->fops.read = pdm_client_fops_default_read;
	client->fops.write = pdm_client_fops_default_write;
	client->fops.unlocked_ioctl = pdm_client_fops_default_ioctl;
	client->fops.compat_ioctl = pdm_client_fops_default_compat_ioctl;
	cdev_init(&client->cdev, &client->fops);

	status = cdev_device_add(&client->cdev, &client->dev);
	if (status < 0) {
		OSA_ERROR("Failed to add char device for %s, error: %d\n", dev_name(&client->dev), status);
		return status;
	}

	return 0;
}

/**
 * @brief Unregisters a PDM Client device and releases associated resources.
 *
 * This function removes the character device from the system and decreases the reference count of the client.
 *
 * @param client Pointer to the PDM Client structure.
 */
static void pdm_client_device_unregister(struct pdm_client *client)
{
	cdev_device_del(&client->cdev, &client->dev);
}

/**
 * @brief Unregisters a PDM Client managed by devres.
 *
 * This function safely unregisters a PDM Client device using devres management, ensuring all resources are properly released.
 *
 * @param dev Pointer to the parent device structure.
 * @param res Pointer to the resource data.
 */
static void devm_pdm_client_unregister(void *data)
{
	struct pdm_client *client = data;

	if ((!client) || (!client->adapter)) {
		OSA_ERROR("Invalid input parameters (adapter: %p, client: %p)\n", client->adapter, client);
		return;
	}

	OSA_INFO("PDM Client Unregistered: %s\n", dev_name(&client->dev));

	mutex_lock(&client->adapter->client_list_mutex_lock);
	list_del(&client->entry);
	mutex_unlock(&client->adapter->client_list_mutex_lock);

	pdm_client_device_unregister(client);
	pdm_adapter_id_free(client->adapter, client);
	pdm_adapter_put(client->adapter);
}

/**
 * @brief Registers a PDM client with the associated PDM adapter.
 *
 * This function registers the PDM client with the PDM adapter, setting up the necessary
 * resources and file operations, and linking the client to the adapter's client list.
 *
 * @param adapter Pointer to the PDM adapter structure.
 * @param client Pointer to the PDM client structure.
 * @return 0 on success, negative error code on failure.
 */
int devm_pdm_client_register(struct pdm_adapter *adapter, struct pdm_client *client)
{
	int status;

	if (!adapter || !client) {
		OSA_ERROR("Invalid input parameters (adapter: %p, client: %p)\n", adapter, client);
		return -EINVAL;
	}

	if (!pdm_adapter_get(adapter)) {
		OSA_ERROR("Failed to get adapter\n");
		return -EBUSY;
	}

	status = pdm_adapter_id_alloc(adapter, client);
	if (status) {
		OSA_ERROR("Alloc id for client failed: %d\n", status);
		goto err_put_adapter;
	}

	client->adapter = adapter;
	status = pdm_client_device_register(client);
	if (status) {
		OSA_ERROR("Failed to register device, error: %d\n", status);
		goto err_free_id;
	}

	mutex_lock(&adapter->client_list_mutex_lock);
	list_add_tail(&client->entry, &adapter->client_list);
	mutex_unlock(&adapter->client_list_mutex_lock);

	status = devm_add_action_or_reset(&client->pdmdev->dev, devm_pdm_client_unregister, client);
	if (status) {
		OSA_ERROR("Failed to add devres, error: %d\n", status);
		return status;
	}

	OSA_INFO("PDM Client Registered: %s\n", dev_name(&client->dev));
	return 0;

err_free_id:
	pdm_adapter_id_free(adapter, client);
err_put_adapter:
	pdm_adapter_put(adapter);
	return status;
}

/**
 * @brief Releases the device structure when the last reference is dropped.
 *
 * This function is called by the kernel when the last reference to the device is dropped, freeing the allocated memory.
 *
 * @param dev Pointer to the device structure.
 */
static void pdm_client_device_release(struct device *dev)
{
	struct pdm_client *client = container_of(dev, struct pdm_client, dev);
	kfree(client);
}

/**
 * @brief Frees a PDM Client managed by devres.
 *
 * This function drops the reference to the PDM Client and ensures it is properly freed when no longer needed.
 *
 * @param dev Pointer to the parent device structure.
 * @param res Pointer to the resource data.
 */
static void devm_pdm_client_free(void *data)
{
	pdm_client_put_device((struct pdm_client *)data);
}

/**
 * @brief Allocates and initializes a pdm_client structure, along with its associated resources.
 *
 * This function allocates memory for a new PDM client, initializes the structure, and returns
 * a pointer to the newly allocated pdm_client.
 *
 * @param pdmdev Pointer to the PDM device structure to which the client is associated.
 * @param data_size Size of additional data to allocate for the client.
 * @return Pointer to the newly allocated pdm_client structure, or ERR_PTR on failure.
 */
struct pdm_client *devm_pdm_client_alloc(struct pdm_device *pdmdev, unsigned int data_size)
{
	struct pdm_client *client;
	unsigned int client_size = sizeof(struct pdm_client);
	unsigned int total_size = ALIGN(client_size + data_size, 8);

	if (!pdmdev) {
		OSA_ERROR("Invalid pdm_device pointer\n");
		return ERR_PTR(-EINVAL);
	}

	client = kzalloc(total_size, GFP_KERNEL);
	if (!client) {
		OSA_ERROR("Failed to allocate memory for pdm_client\n");
		return ERR_PTR(-ENOMEM);
	}

	client->dev.class = &pdm_client_class;
	client->dev.release = pdm_client_device_release;
	client->dev.parent = &pdmdev->dev;
	device_initialize(&client->dev);

	pdmdev->client = client;
	client->pdmdev = pdmdev;
	if (data_size) {
		pdm_client_set_private_data(client, (void *)(client + client_size));
	}

	if (devm_add_action_or_reset(&pdmdev->dev, devm_pdm_client_free, client)) {
		return ERR_PTR(-ENOMEM);
	}

	return client;
}

/**
 * @brief Retrieves match data for a PDM device from the device tree.
 *
 * This function looks up the device tree to find matching data for the given PDM device,
 * which can be used for initialization or configuration.
 *
 * @param pdmdev Pointer to the PDM device structure.
 * @return Pointer to the match data if found; NULL otherwise.
 */
const void *pdm_client_get_match_data(struct pdm_client *client)
{
	const struct of_device_id *match;

	if (!client || !client->pdmdev || !client->pdmdev->dev.driver) {
		return NULL;
	}
	if (!client->pdmdev->dev.driver->of_match_table || !client->pdmdev->dev.parent) {
		return NULL;
	}

	match = of_match_device(client->pdmdev->dev.driver->of_match_table, client->pdmdev->dev.parent);
	if (!match) {
		return NULL;
	}
	return match->data;
}

/**
 * @brief Retrieves the device tree node for a PDM device's parent device.
 *
 * This function retrieves the device tree node associated with the parent device of the given PDM device.
 * It can be used to access properties or subnodes defined in the device tree for the parent device.
 *
 * @param pdmdev Pointer to the PDM device structure.
 * @return Pointer to the device_node structure if found; NULL otherwise.
 */
struct device_node *pdm_client_get_of_node(struct pdm_client *client)
{
	if (!client || !client->pdmdev || !client->pdmdev->dev.parent) {
		return NULL;
	}
	return dev_of_node(client->pdmdev->dev.parent);
}

/**
 * @brief Setup a PDM device.
 *
 * @param pdmdev Pointer to the PDM device structure.
 * @return 0 on success, negative error code on failure.
 */
int pdm_client_setup(struct pdm_client *client)
{
	const struct pdm_client_match_data *match_data;
	int status;

	match_data = pdm_client_get_match_data(client);
	if (!match_data) {
		OSA_DEBUG("Failed to get match data for device: %s\n", dev_name(&client->dev));
		return 0;
	}

	if (match_data->setup) {
		status = match_data->setup(client);
		if (status) {
			OSA_ERROR("PDM Device Setup Failed, status=%d\n", status);
			return status;
		}
	}

	return status;
}

/**
 * @brief Cleanup a PDM device.
 *
 * @param pdmdev Pointer to the PDM device structure.
 */
void pdm_client_cleanup(struct pdm_client *client)
{
	const struct pdm_client_match_data *match_data;

	match_data = pdm_client_get_match_data(client);
	if (!match_data) {
		OSA_ERROR("Failed to get match data for device\n");
		return;
	}

	if (match_data->cleanup) {
		match_data->cleanup(client);
	}
}

/**
 * @brief Initializes the PDM Client module.
 *
 * This function initializes the PDM Client module by registering necessary drivers and setting initial states.
 *
 * @return 0 on success, negative error code on failure.
 */
int pdm_client_init(void)
{
	int status;
	dev_t dev;

	status = class_register(&pdm_client_class);
	if (status < 0) {
		OSA_ERROR("Failed to register PDM Client Class, error: %d\n", status);
		return status;
	}

	status = alloc_chrdev_region(&dev, 0, PDM_CLIENT_MINORS, PDM_CLIENT_DEVICE_NAME);
	if (status < 0) {
		OSA_ERROR("Failed to allocate device region for %s, error: %d\n",
				  PDM_CLIENT_DEVICE_NAME, status);
		class_unregister(&pdm_client_class);
		return status;
	}

	pdm_client_major = MAJOR(dev);
	OSA_DEBUG("PDM Client Initialized, Major is %d\n", pdm_client_major);
	return 0;
}

/**
 * @brief Cleans up the PDM Client module.
 *
 * This function unregisters the PDM Client module, including unregistering drivers and cleaning up all resources.
 */
void pdm_client_exit(void)
{
	unregister_chrdev_region(MKDEV(pdm_client_major, 0), PDM_CLIENT_MINORS);
	class_unregister(&pdm_client_class);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Client Module");
