#include "pdm.h"
#include "pdm_component.h"
#include "pdm_adapter_drivers.h"

/**
 * @brief PDM Adapter 列表
 *
 * 该列表用于存储所有注册的 PDM Adapter 。
 */
static struct list_head pdm_adapter_list;

/**
 * @brief 保护 PDM Adapter 列表的互斥锁
 *
 * 该互斥锁用于同步对 PDM Adapter 列表的访问，防止并发访问导致的数据竞争。
 */
static struct mutex pdm_adapter_list_mutex_lock;

/**
 * name_show - 显示设备名称
 * @dev: 设备结构
 * @da: 设备属性结构
 * @buf: 输出缓冲区
 *
 * 返回值:
 * 实际写入的字节数
 */
static ssize_t name_show(struct device *dev, struct device_attribute *da, char *buf)
{
    struct pdm_adapter *adapter = dev_to_pdm_adapter(dev);
    ssize_t status;

    down_read(&adapter->rwlock);
    status = sysfs_emit(buf, "%s\n", dev_name(&adapter->dev));
    up_read(&adapter->rwlock);

    OSA_INFO("Device name: %s.\n", dev_name(&adapter->dev));
    return status;
}

static DEVICE_ATTR_RO(name);

static struct attribute *pdm_adapter_device_attrs[] = {
    &dev_attr_name.attr,
    NULL,
};
ATTRIBUTE_GROUPS(pdm_adapter_device);

/**
 * @brief PDM 主控制器设备类型
 */
static const struct device_type pdm_adapter_device_type = {
    .groups = pdm_adapter_device_groups,
};

/**
 * @brief 释放 PDM 主控制器设备
 * @dev: 设备结构
 */
static void pdm_adapter_dev_release(struct device *dev)
{
    struct pdm_adapter *master = dev_to_pdm_adapter(dev);
    kfree(master);
}

/**
 * @brief 获取PDM主控制器的私有数据
 * @master: PDM主控制器
 *
 * 返回值:
 * 指向私有数据的指针
 */
void *pdm_adapter_devdata_get(struct pdm_adapter *master)
{
    if (!master) {
        OSA_ERROR("Invalid input parameter (master: %p).\n", master);
        return NULL;
    }

    return dev_get_drvdata(&master->dev);
}

/**
 * @brief 设置PDM主控制器的私有数据
 * @master: PDM主控制器
 * @data: 私有数据指针
 */
void pdm_adapter_devdata_set(struct pdm_adapter *master, void *data)
{
    if (!master) {
        OSA_ERROR("Invalid input parameter (master: %p).\n", master);
        return;
    }

    dev_set_drvdata(&master->dev, data);
}


/**
 * @brief 分配PDM设备ID
 *
 * 该函数用于分配一个唯一的ID给PDM设备。
 *
 * @param adapter PDM Adapter
 * @param pdmdev PDM设备
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_adapter_id_alloc(struct pdm_adapter *adapter, struct pdm_client *client)
{
    int id;
    int client_index = 0;

    if (!client) {
        OSA_ERROR("Invalid input parameters.\n");
        return -EINVAL;
    }

    if (of_property_read_s32(client->pdmdev->dev.parent->of_node, "client-index", &client_index)) {
        if (client->force_dts_id) {
            OSA_ERROR("Cannot get index from dts, force_dts_id was set\n");
            return -EINVAL;
        }
        OSA_DEBUG("Cannot get index from dts\n");
    }

    if (client_index < 0) {
        OSA_ERROR("Invalid client index: %d.\n", client->index);
        return -EINVAL;
    }

    mutex_lock(&adapter->idr_mutex_lock);
    id = idr_alloc(&adapter->device_idr, NULL, client_index, PDM_ADAPTER_CLIENT_IDR_END, GFP_KERNEL);
    mutex_unlock(&adapter->idr_mutex_lock);

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
 * @brief 释放 PDM Client 的ID
 *
 * 该函数用于释放 PDM Client 的ID。
 *
 * @param adapter PDM Adapter
 * @param client PDM Client
 */
void pdm_adapter_id_free(struct pdm_adapter *adapter, struct pdm_client *client)
{
    mutex_lock(&adapter->idr_mutex_lock);
    idr_remove(&adapter->device_idr, client->index);
    mutex_unlock(&adapter->idr_mutex_lock);
}

/**
 * @brief 获取PDM主控制器的引用
 * @master: PDM主控制器
 *
 * 返回值:
 * 指向PDM主控制器的指针，或NULL（失败）
 */
struct pdm_adapter *pdm_adapter_get(struct pdm_adapter *master)
{
    if (!master || !get_device(&master->dev)) {
        OSA_ERROR("Invalid input parameter or unable to get device reference (master: %p).\n", master);
        return NULL;
    }

    return master;
}

/**
 * @brief 释放PDM主控制器的引用
 * @master: PDM主控制器
 */
void pdm_adapter_put(struct pdm_adapter *master)
{
    if (master) {
        put_device(&master->dev);
    }
}

/**
 * @brief 注册 PDM  Adapter
 *
 * 该函数用于注册 PDM  Adapter，并将其添加到 Adapter列表中。
 *
 * @param adapter PDM  Adapter指针
 * @return 0 - 成功
 *         -EINVAL - 参数无效
 *         -EEXIST -  Adapter名称已存在
 *         其他负值 - 其他错误码
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

    adapter->dev.type = &pdm_adapter_device_type;
    dev_set_name(&adapter->dev, "pdm_adapter_%s", name);
    status = device_add(&adapter->dev);
    if (status) {
        OSA_ERROR("Failed to add device: %s, error: %d.\n", dev_name(&adapter->dev), status);
        goto err_device_put;
    }

    mutex_lock(&pdm_adapter_list_mutex_lock);
    list_add_tail(&adapter->entry, &pdm_adapter_list);
    mutex_unlock(&pdm_adapter_list_mutex_lock);

    mutex_init(&adapter->idr_mutex_lock);
    idr_init(&adapter->device_idr);

    OSA_DEBUG("PDM Adapter Registered: %s.\n", name);

    return 0;

err_device_put:
    pdm_adapter_put(adapter);
    return status;
}

/**
 * @brief 注销 PDM  Adapter
 *
 * 该函数用于注销 PDM  Adapter，并从 Adapter列表中移除。
 *
 * @param adapter PDM  Adapter指针
 */
void pdm_adapter_unregister(struct pdm_adapter *adapter)
{
    if (!adapter) {
        OSA_ERROR("Invalid input parameters (adapter: %p).\n", adapter);
        return;
    }

    OSA_INFO("PDM Adapter unregistered: %s.\n", dev_name(&adapter->dev));

    mutex_lock(&pdm_adapter_list_mutex_lock);
    list_del(&adapter->entry);
    mutex_unlock(&pdm_adapter_list_mutex_lock);

    device_unregister(&adapter->dev);
}

/**
 * @brief 分配PDM Adapter结构
 *
 * 该函数用于分配PDM Adapter结构及其私有数据。
 *
 * @param data_size 私有数据的大小
 * @return 指向分配的PDM Adapter的指针，或NULL（失败）
 */
struct pdm_adapter *pdm_adapter_alloc(unsigned int data_size)
{
    struct pdm_adapter *adapter;
    size_t adapter_size = sizeof(struct pdm_adapter);

    adapter = kzalloc(adapter_size + data_size, GFP_KERNEL);
    if (!adapter) {
        OSA_ERROR("Failed to allocate memory for pdm_adapter.\n");
        return NULL;
    }

    device_initialize(&adapter->dev);
    adapter->dev.release = pdm_adapter_dev_release;
    pdm_adapter_devdata_set(adapter, (void *)adapter + adapter_size);

    INIT_LIST_HEAD(&adapter->client_list);
    mutex_init(&adapter->client_list_mutex_lock);

    return adapter;
}

/**
 * @brief 释放PDM Adapter结构
 *
 * 该函数用于释放PDM Adapter结构。
 *
 * @param adapter PDM Adapter
 */
void pdm_adapter_free(struct pdm_adapter *adapter)
{
    if (adapter) {
        kfree(adapter);
    }
}

/**
 * @brief 初始化 PDM  Adapter模块
 *
 * 该函数用于初始化 PDM  Adapter模块，包括注册类和驱动。
 *
 * @return 0 - 成功
 *         负值 - 失败
 */
int pdm_adapter_init(void)
{
    int status;

    INIT_LIST_HEAD(&pdm_adapter_list);
    mutex_init(&pdm_adapter_list_mutex_lock);

    status = pdm_adapter_drivers_register();
    if (status < 0) {
        OSA_ERROR("Failed to register PDM Adapter Drivers, error: %d.\n", status);
        return status;
    }

    OSA_DEBUG("Initialize PDM Adapter OK.\n");
    return 0;
}

/**
 * @brief 卸载 PDM  Adapter模块
 *
 * 该函数用于卸载 PDM  Adapter模块，包括注销类和驱动。
 */
void pdm_adapter_exit(void)
{
    pdm_adapter_drivers_unregister();
    OSA_DEBUG("PDM Adapter unregistered.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Adapter Module");
