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
        OSA_ERROR("Invalid client.\n");
        return -EINVAL;
    }

    filp->private_data = client;
    OSA_INFO("fops_default_open for %s\n", dev_name(&client->dev));

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
    struct pdm_client *client = filp->private_data;
    OSA_INFO("fops_default_release for %s\n", dev_name(&client->dev));
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
    struct pdm_client *client = filp->private_data;
    OSA_INFO("fops_default_read for %s\n", dev_name(&client->dev));
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
    struct pdm_client *client = filp->private_data;
    OSA_INFO("fops_default_write for %s\n", dev_name(&client->dev));
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
    OSA_INFO("This client does not support ioctl operations.\n");
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
static int __pdm_client_device_register(struct pdm_client *client)
{
    int status;

    if (!client || !client->adapter) {
        OSA_ERROR("Invalid input parameter.\n");
        return -EINVAL;
    }

    if (client->index >= PDM_CLIENT_MINORS) {
        OSA_ERROR("Out of pdm_client minors (%d)\n", client->index);
        return -ENODEV;
    }

    client->fops.open = pdm_client_fops_default_open;
    client->fops.release = pdm_client_fops_default_release;
    client->fops.read = pdm_client_fops_default_read;
    client->fops.write = pdm_client_fops_default_write;
    client->fops.unlocked_ioctl = pdm_client_fops_default_ioctl;
    client->fops.compat_ioctl = pdm_client_fops_default_compat_ioctl;
    cdev_init(&client->cdev, &client->fops);

    client->dev.devt = MKDEV(pdm_client_major, client->index);
    status = dev_set_name(&client->dev, "%s.%d", dev_name(&client->adapter->dev), client->index);
    if (status) {
        OSA_ERROR("Failed to set client name, error:%d.\n", status);
        return status;
    }

    status = cdev_device_add(&client->cdev, &client->dev);
    if (status < 0) {
        OSA_ERROR("Failed to add char device for %s, error: %d.\n", dev_name(&client->dev), status);
        return status;
    }

    OSA_DEBUG("PDM Client %s Device Registered.\n", dev_name(&client->dev));
    return 0;
}

/**
 * @brief Unregisters a PDM Client device and releases associated resources.
 *
 * This function removes the character device from the system and decreases the reference count of the client.
 *
 * @param client Pointer to the PDM Client structure.
 */
static void __pdm_client_device_unregister(struct pdm_client *client)
{
    cdev_device_del(&client->cdev, &client->dev);
    pdm_client_put_device(client);
}

/**
 * @brief Unregisters a PDM Client managed by devres.
 *
 * This function safely unregisters a PDM Client device using devres management, ensuring all resources are properly released.
 *
 * @param dev Pointer to the parent device structure.
 * @param res Pointer to the resource data.
 */
static void __devm_pdm_client_unregister(struct device *dev, void *res)
{
    struct pdm_client_devres *devres = res;
    struct pdm_client *client = devres->client;

    if ((!client) || (!client->adapter)) {
        OSA_ERROR("Invalid input parameters (adapter: %p, client: %p).\n", client->adapter, client);
        return;
    }
    OSA_DEBUG("Device %s unregistered.\n", dev_name(&client->dev));

    mutex_lock(&client->adapter->client_list_mutex_lock);
    list_del(&client->entry);
    mutex_unlock(&client->adapter->client_list_mutex_lock);

    __pdm_client_device_unregister(client);

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
    struct pdm_client_devres *devres = NULL;
    int status;

    if (!adapter || !client) {
        OSA_ERROR("Invalid input parameters (adapter: %p, client: %p).\n", adapter, client);
        return -EINVAL;
    }

    devres = devres_alloc(__devm_pdm_client_unregister, sizeof(*devres), GFP_KERNEL);
    if (!devres) {
        OSA_ERROR("Failed to allocate devres for pdm_client.\n");
        return -ENOMEM;
    }
    devres->client = client;

    if (!pdm_adapter_get(adapter)) {
        OSA_ERROR("Failed to get adapter.\n");
        status = -EBUSY;
        goto err_devres_free;
    }

    status = pdm_adapter_id_alloc(adapter, client);
    if (status) {
        OSA_ERROR("Alloc id for client failed: %d\n", status);
        goto err_put_adapter;
    }

    client->adapter = adapter;
    status = __pdm_client_device_register(client);
    if (status) {
        OSA_ERROR("Failed to register device, error: %d.\n", status);
        goto err_free_id;
    }

    mutex_lock(&adapter->client_list_mutex_lock);
    list_add_tail(&client->entry, &adapter->client_list);
    mutex_unlock(&adapter->client_list_mutex_lock);

    OSA_DEBUG("PDM Client %s registered.\n", dev_name(&client->dev));
    devres_add(client->dev.parent, devres);

    return 0;

err_free_id:
    pdm_adapter_id_free(adapter, client);
err_put_adapter:
    pdm_adapter_put(adapter);
err_devres_free:
    devres_free(devres);
    return status;
}

/**
 * @brief Releases the device structure when the last reference is dropped.
 *
 * This function is called by the kernel when the last reference to the device is dropped, freeing the allocated memory.
 *
 * @param dev Pointer to the device structure.
 */
static void __pdm_client_device_release(struct device *dev)
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
static void __devm_pdm_client_free(struct device *dev, void *res)
{
    struct pdm_client_devres *devres = res;
    struct pdm_client *client = devres->client;

    OSA_INFO("Dropping reference to %s\n", dev_name(&client->dev));
    pdm_client_put_device(client);
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
    struct pdm_client_devres *devres;
    unsigned int client_size = sizeof(struct pdm_client);
    unsigned int total_size = ALIGN(client_size + data_size, 8);

    if (!pdmdev) {
        OSA_ERROR("Invalid pdm_device pointer.\n");
        return ERR_PTR(-EINVAL);
    }

    devres = devres_alloc(__devm_pdm_client_free, sizeof(*devres), GFP_KERNEL);
    if (!devres) {
        return ERR_PTR(-ENOMEM);
    }

    client = kzalloc(total_size, GFP_KERNEL);
    if (!client) {
        OSA_ERROR("Failed to allocate memory for pdm_client.\n");
        devres_free(devres);
        return ERR_PTR(-ENOMEM);
    }

    client->dev.class = &pdm_client_class;
    client->dev.release = __pdm_client_device_release;
    client->dev.parent = &pdmdev->dev;
    device_initialize(&client->dev);

    pdm_client_set_drvdata(client, (void *)(client + client_size));

    devres->client = client;
    devres_add(&pdmdev->dev, devres);

    return client;
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
        OSA_ERROR("Failed to register PDM Client Class, error: %d.\n", status);
        return status;
    }
    OSA_DEBUG("PDM Client Class registered.\n");

    status = alloc_chrdev_region(&dev, 0, PDM_CLIENT_MINORS, PDM_CLIENT_DEVICE_NAME);
    if (status < 0) {
        OSA_ERROR("Failed to allocate device region for %s, error: %d.\n",
                  PDM_CLIENT_DEVICE_NAME, status);
        class_unregister(&pdm_client_class);
        return status;
    }

    pdm_client_major = MAJOR(dev);
    OSA_DEBUG("PDM Client major: %d\n", pdm_client_major);

    OSA_INFO("PDM Client initialized successfully.\n");
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
    OSA_INFO("PDM Client cleaned up successfully.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Client Module");
