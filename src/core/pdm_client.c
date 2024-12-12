#include <linux/compat.h>
#include "pdm.h"

/**
 * @brief PDM Client class.
 */
static struct class pdm_client_class = {
    .name = "pdm_client",
};

/**
 * @brief PDM Client major device number.
 */
static dev_t pdm_client_major;

/**
 * @brief IDA for allocating minor device numbers.
 */
static struct ida pdm_client_ida;

/**
 * @brief Allocates a new minor device number.
 *
 * @return The allocated minor number, or negative error code on failure.
 */
static int pdm_client_minor_alloc(void)
{
    return ida_alloc_range(&pdm_client_ida, PDM_CLIENT_MIN_MINOR, PDM_CLIENT_MAX_MINOR, GFP_KERNEL);
}

/**
 * @brief Frees a previously allocated minor device number.
 *
 * @param minor The minor number to free.
 */
static void pdm_client_minor_free(unsigned int minor)
{
    ida_free(&pdm_client_ida, minor);
}

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
    struct pdm_client *client;

    OSA_INFO("fops_default_open.\n");

    client = container_of(inode->i_cdev, struct pdm_client, cdev);
    if (!client) {
        OSA_ERROR("Invalid client.\n");
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
    OSA_INFO("fops_default_release.\n");
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
    OSA_INFO("fops_default_read.\n");
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
    OSA_INFO("fops_default_write.\n");
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
    OSA_INFO("pdm_client_fops_default_compat_ioctl.\n");

    if (_IOC_DIR(cmd) & (_IOC_READ | _IOC_WRITE)) {
        arg = (unsigned long)compat_ptr(arg);
    }

    return filp->f_op->unlocked_ioctl(filp, cmd, arg);
}

/**
 * @brief Decrements the reference count on the device.
 *
 * @param client Pointer to the PDM Client.
 */
static inline void pdm_client_put_device(struct pdm_client *client)
{
    if (client)
        put_device(&client->dev);
}

/**
 * @brief Releases the device structure.
 *
 * This function is called when the last reference to the device is dropped.
 *
 * @param dev Pointer to the device structure.
 */
static void pdm_client_device_release(struct device *dev)
{
    struct pdm_client *client;

    client = container_of(dev, struct pdm_client, dev);
    pdm_client_put_device(client);
    kfree(client);
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
    int minor;

    if (!client) {
        OSA_ERROR("Invalid input parameter.\n");
        return -EINVAL;
    }

    minor = pdm_client_minor_alloc();
    if (minor < 0) {
        OSA_ERROR("Failed to allocate new minor: %d\n", minor);
        return minor;
    }
    OSA_DEBUG("Client minor: %d\n", minor);

    dev_set_name(&client->dev, "%s.%d", dev_name(&client->adapter->dev), client->index);

    client->dev.devt = MKDEV(pdm_client_major, minor);
    client->dev.class = &pdm_client_class;
    client->dev.parent = &client->pdmdev->dev;
    client->dev.release = pdm_client_device_release;
    device_initialize(&client->dev);

    memset(&client->fops, 0, sizeof(client->fops));
    client->fops.open = pdm_client_fops_default_open;
    client->fops.release = pdm_client_fops_default_release;
    client->fops.read = pdm_client_fops_default_read;
    client->fops.write = pdm_client_fops_default_write;
    client->fops.unlocked_ioctl = pdm_client_fops_default_ioctl;
    client->fops.compat_ioctl = pdm_client_fops_default_compat_ioctl;
    cdev_init(&client->cdev, &client->fops);

    status = cdev_device_add(&client->cdev, &client->dev);
    if (status < 0) {
        OSA_ERROR("Failed to add char device for %s, error: %d.\n", dev_name(&client->dev), status);
        goto err_free_minor;
    }

    pdm_client_set_devdata(client, (void *)(client + sizeof(struct pdm_client)));

    OSA_DEBUG("PDM Client %s Device Registered.\n", dev_name(&client->dev));

    return 0;

err_free_minor:
    pdm_client_minor_free(minor);
    return status;
}

/**
 * @brief Unregisters a PDM Client character device.
 *
 * This function unregisters a PDM Client character device from the system.
 *
 * @param client Pointer to the PDM Client.
 */
static void pdm_client_device_unregister(struct pdm_client *client)
{
    if (!client) {
        OSA_ERROR("Invalid input parameter.\n");
        return;
    }
    cdev_device_del(&client->cdev, &client->dev);
    pdm_client_minor_free(MINOR(client->dev.devt));

    OSA_DEBUG("PDM Client Device Unregistered.\n");
}

/**
 * @brief Adds a device to the PDM Adapter.
 *
 * This function adds a new PDM device to the specified PDM Adapter.
 *
 * @param adapter Pointer to the PDM Adapter.
 * @param client Pointer to the PDM Client to add.
 * @return 0 on success, negative error code on failure.
 */
int pdm_client_register(struct pdm_adapter *adapter, struct pdm_client *client)
{
    int status;

    if (!adapter || !client) {
        OSA_ERROR("Invalid input parameters (adapter: %p, client: %p).\n", adapter, client);
        return -EINVAL;
    }

    status = pdm_adapter_id_alloc(adapter, client);
    if (status) {
        OSA_ERROR("Alloc id for client failed: %d\n", status);
        return status;
    }

    client->adapter = adapter;
    status = pdm_client_device_register(client);
    if (status) {
        OSA_ERROR("Failed to register device, error: %d.\n", status);
        pdm_adapter_id_free(adapter, client);
        return status;
    }

    mutex_lock(&adapter->client_list_mutex_lock);
    list_add_tail(&client->entry, &adapter->client_list);
    mutex_unlock(&adapter->client_list_mutex_lock);

    return 0;
}

/**
 * @brief Removes a device from the PDM Adapter.
 *
 * This function removes a PDM device from the specified PDM Adapter.
 *
 * @param adapter Pointer to the PDM Adapter.
 * @param client Pointer to the PDM Client to remove.
 */
void pdm_client_unregister(struct pdm_adapter *adapter, struct pdm_client *client)
{
    if (!adapter || !client) {
        OSA_ERROR("Invalid input parameters (adapter: %p, client: %p).\n", adapter, client);
        return;
    }
    OSA_DEBUG("Device %s unregistered.\n", dev_name(&client->dev));

    mutex_lock(&adapter->client_list_mutex_lock);
    list_del(&client->entry);
    mutex_unlock(&adapter->client_list_mutex_lock);

    pdm_client_device_unregister(client);
    pdm_adapter_id_free(adapter, client);
}

/**
 * @brief Allocates a PDM Client structure.
 *
 * This function allocates and initializes a new PDM Client structure along with its private data.
 *
 * @param data_size Size of the private data to allocate.
 * @return Pointer to the allocated PDM Client structure, or NULL on failure.
 */
struct pdm_client *pdm_client_alloc(unsigned int data_size)
{
    struct pdm_client *client;

    client = kzalloc(sizeof(struct pdm_client) + data_size, GFP_KERNEL);
    if (!client) {
        OSA_ERROR("Failed to allocate memory for pdm_client.\n");
        return NULL;
    }

    return client;
}

/**
 * @brief Frees a PDM Client structure.
 *
 * This function frees an allocated PDM Client structure and any associated resources.
 *
 * @param client Pointer to the PDM Client structure.
 */
void pdm_client_free(struct pdm_client *client)
{
    if (client) {
        kfree(client);
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
        OSA_ERROR("Failed to register PDM Client Class, error: %d.\n", status);
        return status;
    }
    OSA_DEBUG("PDM Client Class registered.\n");

    status = alloc_chrdev_region(&dev, PDM_CLIENT_MIN_MINOR,
                                 PDM_CLIENT_MAX_MINOR - PDM_CLIENT_MIN_MINOR + 1,
                                 PDM_CLIENT_DEVICE_NAME);
    if (status < 0) {
        OSA_ERROR("Failed to allocate device region for %s, error: %d.\n",
                  PDM_CLIENT_DEVICE_NAME, status);
        class_unregister(&pdm_client_class);
        return status;
    }

    pdm_client_major = MAJOR(dev);
    OSA_DEBUG("Client major: %d\n", pdm_client_major);

    ida_init(&pdm_client_ida);

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
    ida_destroy(&pdm_client_ida);
    unregister_chrdev_region(MKDEV(pdm_client_major, PDM_CLIENT_MIN_MINOR),
                             PDM_CLIENT_MAX_MINOR - PDM_CLIENT_MIN_MINOR + 1);

    class_unregister(&pdm_client_class);

    OSA_INFO("PDM Client cleaned up successfully.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Client Module");
