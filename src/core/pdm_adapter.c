#include "pdm.h"
#include "pdm_component.h"
#include "pdm_adapter_priv.h"

/**
 * @brief List of registered PDM Adapter drivers.
 */
static struct list_head pdm_adapter_driver_list;

/**
 * @brief Array of PDM Adapter drivers to be registered.
 */
static struct pdm_component pdm_adapter_drivers[] = {
    {
        .name = "LED Adapter",
        .status = true,
        .ignore_failures = true,
        .init = pdm_led_driver_init,
        .exit = pdm_led_driver_exit,
    },
    { }
};

/**
 * @brief Registers PDM Adapter drivers.
 *
 * This function initializes the PDM Adapter driver list and registers all PDM Adapter drivers.
 *
 * @return 0 on success, negative error code on failure.
 */
int pdm_adapter_drivers_register(void)
{
    int status;
    struct pdm_component_params params = {
        .components = pdm_adapter_drivers,
        .count = ARRAY_SIZE(pdm_adapter_drivers),
        .list = &pdm_adapter_driver_list,
    };

    INIT_LIST_HEAD(&pdm_adapter_driver_list);
    status = pdm_component_register(&params);
    if (status < 0) {
        OSA_ERROR("Failed to register PDM Adapter Drivers, error: %d.\n", status);
        return status;
    }

    OSA_DEBUG("PDM Adapter Drivers Registered OK.\n");
    return 0;
}

/**
 * @brief Unregisters PDM Adapter drivers.
 */
void pdm_adapter_drivers_unregister(void)
{
    pdm_component_unregister(&pdm_adapter_driver_list);
    OSA_DEBUG("PDM Adapter Drivers Unregistered.\n");
}

/**
 * @brief List of registered PDM Adapters.
 */
static struct list_head pdm_adapter_list = LIST_HEAD_INIT(pdm_adapter_list);

/**
 * @brief Mutex to protect the PDM Adapter list.
 */
static struct mutex pdm_adapter_list_mutex_lock = __MUTEX_INITIALIZER(pdm_adapter_list_mutex_lock);

/**
 * @brief Sysfs attribute for showing device name.
 */
static ssize_t client_list_show(struct device *dev, struct device_attribute *da, char *buf)
{
    struct pdm_adapter *adapter = dev_to_pdm_adapter(dev);
    struct pdm_client *client;

    down_read(&adapter->rwlock);
    sysfs_emit(buf, "PDM Adapter %s's client list:\n", dev_name(&adapter->dev));
    list_for_each_entry(client, &adapter->client_list, entry) {
        if (sysfs_emit(buf, "%s\n", dev_name(&client->dev))) {
            OSA_WARN("sysfs_emit error\n");
        }
    }
    up_read(&adapter->rwlock);

    return 0;
}
static DEVICE_ATTR_RO(client_list);

/**
 * @brief Sysfs attribute for showing device name.
 */
static ssize_t name_show(struct device *dev, struct device_attribute *da, char *buf)
{
    struct pdm_adapter *adapter = dev_to_pdm_adapter(dev);

    down_read(&adapter->rwlock);
    if (sysfs_emit(buf, "%s\n", dev_name(&adapter->dev))) {
        OSA_WARN("sysfs_emit error\n");
    }
    up_read(&adapter->rwlock);

    OSA_INFO("PDM Adapter name: %s.\n", dev_name(&adapter->dev));
    return 0;
}
static DEVICE_ATTR_RO(name);

static struct attribute *pdm_adapter_device_attrs[] = {
    &dev_attr_name.attr,
    &dev_attr_client_list.attr,
    NULL,
};
ATTRIBUTE_GROUPS(pdm_adapter_device);

static const struct device_type pdm_adapter_device_type = {
    .name   = "pdm_adapter",
    .groups = pdm_adapter_device_groups,
};

/**
 * @brief Releases a PDM Adapter device.
 */
static void pdm_adapter_dev_release(struct device *dev)
{
    struct pdm_adapter *adapter = dev_to_pdm_adapter(dev);
    WARN(!list_empty(&adapter->client_list), "Client list is not empty!");
    kfree(adapter);
}

/**
 * @brief Gets private data from a PDM Adapter.
 *
 * @param adapter Pointer to the PDM Adapter structure.
 * @return Pointer to the private data.
 */
void *pdm_adapter_devdata_get(struct pdm_adapter *adapter)
{
    if (!adapter) {
        OSA_ERROR("Invalid input parameter (adapter: %p).\n", adapter);
        return NULL;
    }

    return dev_get_drvdata(&adapter->dev);
}

/**
 * @brief Sets private data for a PDM Adapter.
 *
 * @param adapter Pointer to the PDM Adapter structure.
 * @param data Pointer to the private data.
 */
void pdm_adapter_devdata_set(struct pdm_adapter *adapter, void *data)
{
    if (!adapter) {
        OSA_ERROR("Invalid input parameter (adapter: %p).\n", adapter);
        return;
    }

    dev_set_drvdata(&adapter->dev, data);
}

/**
 * @brief Allocates a unique ID for a PDM Client.
 *
 * @param adapter Pointer to the PDM Adapter structure.
 * @param client Pointer to the PDM Client structure.
 * @return 0 on success, negative error code on failure.
 */
int pdm_adapter_id_alloc(struct pdm_adapter *adapter, struct pdm_client *client)
{
    int id;
    int index = 0;

    if (!client || !adapter) {
        OSA_ERROR("Invalid input parameters.\n");
        return -EINVAL;
    }

    if (!of_property_read_s32(client->pdmdev->dev.parent->of_node, "index", &index)) {
        if (client->force_dts_id && index < 0) {
            OSA_ERROR("Cannot get valid index from dts, force_dts_id was set\n");
            return -EINVAL;
        }
    } else {
        OSA_DEBUG("Cannot get index from dts\n");
    }

    mutex_lock(&adapter->ida_mutex_lock);
    id = ida_alloc_range(&adapter->client_ida, index, PDM_CLIENT_MINORS, GFP_KERNEL);
    mutex_unlock(&adapter->ida_mutex_lock);

    if (id < 0) {
        if (id == -ENOSPC) {
            OSA_ERROR("No available IDs in the range.\n");
            return -EBUSY;
        } else {
            OSA_ERROR("Failed to allocate ID: %d.\n", id);
            return id;
        }
    }

    client->index = id;
    return 0;
}

/**
 * @brief Frees the ID allocated for a PDM Client.
 *
 * @param adapter Pointer to the PDM Adapter structure.
 * @param client Pointer to the PDM Client structure.
 */
void pdm_adapter_id_free(struct pdm_adapter *adapter, struct pdm_client *client)
{
    if (!adapter || !client) {
        OSA_ERROR("Invalid input parameters.\n");
        return;
    }

    mutex_lock(&adapter->ida_mutex_lock);
    ida_free(&adapter->client_ida, client->index);
    mutex_unlock(&adapter->ida_mutex_lock);
}

/**
 * @brief Gets a reference to a PDM Adapter.
 *
 * @param adapter Pointer to the PDM Adapter structure.
 * @return Pointer to the PDM Adapter structure, or NULL on failure.
 */
struct pdm_adapter *pdm_adapter_get(struct pdm_adapter *adapter)
{
    if (!adapter || !get_device(&adapter->dev)) {
        OSA_ERROR("Invalid input parameter or unable to get device reference (adapter: %p).\n", adapter);
        return NULL;
    }

    return adapter;
}

/**
 * @brief Releases a reference to a PDM Adapter.
 *
 * @param adapter Pointer to the PDM Adapter structure.
 */
void pdm_adapter_put(struct pdm_adapter *adapter)
{
    if (adapter) {
        put_device(&adapter->dev);
    }
}

/**
 * @brief Registers a PDM Adapter.
 *
 * @param adapter Pointer to the PDM Adapter structure.
 * @param name Name of the adapter.
 * @return 0 on success, negative error code on failure.
 */
int pdm_adapter_register(struct pdm_adapter *adapter, const char *name)
{
    struct pdm_adapter *existing_adapter;
    int status;

    if (!adapter || !name) {
        OSA_ERROR("Invalid input parameters (adapter: %p, name: %s).\n", adapter, name);
        return -EINVAL;
    }

    mutex_lock(&pdm_adapter_list_mutex_lock);
    list_for_each_entry(existing_adapter, &pdm_adapter_list, entry) {
        if (!strcmp(dev_name(&existing_adapter->dev), name)) {
            OSA_ERROR("Adapter already exists: %s.\n", name);
            mutex_unlock(&pdm_adapter_list_mutex_lock);
            return -EEXIST;
        }
    }
    mutex_unlock(&pdm_adapter_list_mutex_lock);

    dev_set_name(&adapter->dev, "%s", name);
    status = device_add(&adapter->dev);
    if (status) {
        OSA_ERROR("Failed to add device: %s, error: %d.\n", dev_name(&adapter->dev), status);
        goto err_device_put;
    }

    mutex_init(&adapter->ida_mutex_lock);
    ida_init(&adapter->client_ida);

    mutex_lock(&pdm_adapter_list_mutex_lock);
    list_add_tail(&adapter->entry, &pdm_adapter_list);
    mutex_unlock(&pdm_adapter_list_mutex_lock);

    OSA_DEBUG("PDM Adapter %s Registered\n", name);

    return 0;

err_device_put:
    pdm_adapter_put(adapter);
    return status;
}

/**
 * @brief Unregisters a PDM Adapter.
 *
 * @param adapter Pointer to the PDM Adapter structure.
 */
void pdm_adapter_unregister(struct pdm_adapter *adapter)
{
    if (!adapter) {
        OSA_ERROR("Invalid input parameters (adapter: %p).\n", adapter);
        return;
    }
    if (!list_empty(&adapter->client_list)) {
        OSA_ERROR("Client list is not empty.\n");
        return;
    }

    OSA_INFO("PDM Adapter unregistered: %s.\n", dev_name(&adapter->dev));

    mutex_lock(&adapter->ida_mutex_lock);
    ida_destroy(&adapter->client_ida);
    mutex_unlock(&adapter->ida_mutex_lock);

    mutex_lock(&pdm_adapter_list_mutex_lock);
    list_del(&adapter->entry);
    mutex_unlock(&pdm_adapter_list_mutex_lock);

    device_unregister(&adapter->dev);
}

/**
 * @brief Allocates and initializes a PDM Adapter structure.
 *
 * @param data_size Size in bytes of the private data to allocate.
 * @return Pointer to the allocated PDM Adapter, or NULL on failure.
 */
struct pdm_adapter *pdm_adapter_alloc(unsigned int data_size)
{
    struct pdm_adapter *adapter;

    adapter = kzalloc(sizeof(struct pdm_adapter) + data_size, GFP_KERNEL);
    if (!adapter) {
        OSA_ERROR("Failed to allocate memory for pdm_adapter.\n");
        return NULL;
    }

    device_initialize(&adapter->dev);
    adapter->dev.release = pdm_adapter_dev_release;
    adapter->dev.type = &pdm_adapter_device_type;
    pdm_adapter_devdata_set(adapter, (void *)adapter + sizeof(struct pdm_adapter));

    INIT_LIST_HEAD(&adapter->client_list);
    mutex_init(&adapter->client_list_mutex_lock);
    init_rwsem(&adapter->rwlock);

    return adapter;
}

/**
 * @brief Frees a PDM Adapter structure.
 *
 * @param adapter Pointer to the PDM Adapter structure.
 */
void pdm_adapter_free(struct pdm_adapter *adapter)
{
    if (adapter) {
        pdm_adapter_put(adapter);
    }
}

/**
 * @brief Initializes the PDM Adapter module.
 *
 * @return 0 on success, negative error code on failure.
 */
int pdm_adapter_init(void)
{
    int status;

    status = pdm_adapter_drivers_register();
    if (status < 0) {
        OSA_ERROR("Failed to register PDM Adapter Drivers, error: %d.\n", status);
        return status;
    }

    OSA_INFO("Initialize PDM Adapter OK.\n");
    return 0;
}

/**
 * @brief Cleans up the PDM Adapter module.
 */
void pdm_adapter_exit(void)
{
    pdm_adapter_drivers_unregister();
    OSA_INFO("PDM Adapter unregistered.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Adapter Module");
