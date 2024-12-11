#include "linux/compat.h"

#include "pdm.h"

#if 0
/**
 * @brief 默认打开函数
 *
 * 该函数是默认的文件打开操作。
 *
 * @param inode inode 结构
 * @param filp 文件结构
 * @return 成功返回 0
 */
static int pdm_client_fops_default_open(struct inode *inode, struct file *filp)
{
    OSA_INFO("fops_default_open.\n");
    return 0;
}

/**
 * @brief 默认释放函数
 *
 * 该函数是默认的文件释放操作。
 *
 * @param inode inode 结构
 * @param filp 文件结构
 * @return 成功返回 0
 */
static int pdm_client_fops_default_release(struct inode *inode, struct file *filp)
{
    OSA_INFO("fops_default_release.\n");
    return 0;
}

/**
 * @brief 默认读取函数
 *
 * 该函数是默认的文件读取操作。
 *
 * @param filp 文件结构
 * @param buf 用户空间缓冲区
 * @param count 要读取的字节数
 * @param ppos 当前文件位置
 * @return 成功返回 0
 */
static ssize_t pdm_client_fops_default_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
    OSA_INFO("fops_default_read.\n");
    return 0;
}

/**
 * @brief 默认写入函数
 *
 * 该函数是默认的文件写入操作。
 *
 * @param filp 文件结构
 * @param buf 用户空间缓冲区
 * @param count 要写入的字节数
 * @param ppos 当前文件位置
 * @return 成功返回写入的字节数
 */
static ssize_t pdm_client_fops_default_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
    OSA_INFO("fops_default_write.\n");
    return count;
}

/**
 * @brief 默认ioctl函数
 *
 * 该函数是默认的ioctl操作。
 *
 * @param filp 文件结构
 * @param cmd ioctl命令
 * @param arg 命令参数
 * @return -ENOTSUPP - 不支持的ioctl操作
 */
static long pdm_client_fops_default_unlocked_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    OSA_INFO("Called pdm_client_fops_default_unlocked_ioctl\n");

    return -ENOTSUPP;
}

/**
 * @brief 兼容层ioctl函数
 *
 * 该函数处理兼容层的ioctl操作。
 *
 * @param filp 文件结构
 * @param cmd ioctl命令
 * @param arg 命令参数
 * @return 返回底层unlocked_ioctl的结果
 */
static long pdm_client_fops_default_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    OSA_INFO("pdm_client_fops_default_compat_ioctl.\n");

    if (_IOC_DIR(cmd) & (_IOC_READ | _IOC_WRITE)) {
        arg = (unsigned long)compat_ptr(arg);
    }

    return filp->f_op->unlocked_ioctl(filp, cmd, arg);
}
#endif

/**
 * @brief 分配PDM设备ID
 *
 * 该函数用于分配一个唯一的ID给PDM设备。
 *
 * @param adapter PDM Adapter
 * @param pdmdev PDM设备
 * @return 成功返回 0，失败返回负错误码
 */
static int pdm_client_id_alloc(struct pdm_adapter *adapter, struct pdm_client *client)
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
 * @brief 释放PDM设备的ID
 *
 * 该函数用于释放PDM设备的ID。
 *
 * @param adapter PDM Adapter
 * @param pdmdev PDM设备
 */
static void pdm_client_id_free(struct pdm_adapter *adapter, struct pdm_client *client)
{
    mutex_lock(&adapter->idr_mutex_lock);
    idr_remove(&adapter->device_idr, client->index);
    mutex_unlock(&adapter->idr_mutex_lock);
}

/**
 * @brief 向PDM Adapter添加设备
 *
 * 该函数用于向PDM Adapter添加一个新的PDM设备。
 *
 * @param adapter PDM Adapter
 * @param pdmdev 要添加的PDM设备
 * @return 成功返回 0，参数无效返回 -EINVAL
 */
int pdm_client_register(struct pdm_adapter *adapter, struct pdm_client *client)
{
    int status;

    if (!adapter || !client) {
        OSA_ERROR("Invalid input parameters (adapter: %p, client: %p).\n", adapter, client);
        return -EINVAL;
    }

    status = pdm_client_id_alloc(adapter, client);
    if (status) {
        OSA_ERROR("Alloc id for client failed: %d\n", status);
        return status;
    }

    snprintf(client->name, PDM_DEVICE_NAME_SIZE, "pdm_%s_client.%d", adapter->name, client->index);

    mutex_lock(&adapter->client_list_mutex_lock);
    list_add_tail(&client->entry, &adapter->client_list);
    mutex_unlock(&adapter->client_list_mutex_lock);

    return 0;
}

/**
 * @brief 从PDM Adapter删除设备
 *
 * 该函数用于从PDM Adapter中删除一个PDM设备。
 *
 * @param adapter PDM Adapter
 * @param pdmdev 要删除的PDM设备
 * @return 成功返回 0，参数无效返回 -EINVAL
 */
void pdm_client_unregister(struct pdm_adapter *adapter, struct pdm_client *client)
{
    if (!adapter || !client) {
        OSA_ERROR("Invalid input parameters (adapter: %p, client: %p).\n", adapter, client);
        return;
    }

    mutex_lock(&adapter->client_list_mutex_lock);
    list_del(&client->entry);
    mutex_unlock(&adapter->client_list_mutex_lock);
    pdm_client_id_free(adapter, client);

    OSA_DEBUG("Device %s removed from %s adapter.\n", client->name, adapter->name);
    return;
}

static inline void *pdm_client_get_devdata(struct pdm_client *client)
{
	return dev_get_drvdata(&client->pdmdev->dev);
}

static inline void pdm_client_set_devdata(struct pdm_client *client, void *data)
{
	dev_set_drvdata(&client->pdmdev->dev, data);
}

/**
 * @brief 分配PDM Client 结构
 *
 * 该函数用于分配PDM Client 结构及其私有数据。
 *
 * @param data_size 私有数据的大小
 * @return 指向分配的PDM Client 的指针，或NULL（失败）
 */
struct pdm_client *pdm_client_alloc(unsigned int data_size)
{
    struct pdm_client *client;
    size_t client_size = sizeof(struct pdm_client);

    client = kzalloc(client_size + data_size, GFP_KERNEL);
    if (!client) {
        OSA_ERROR("Failed to allocate memory for pdm_client.\n");
        return NULL;
    }

    pdm_client_set_devdata(client, (void *)client + client_size);

    return client;
}

/**
 * @brief 释放PDM Client 结构
 *
 * 该函数用于释放PDM Client 结构。
 *
 * @param client PDM Client
 */
void pdm_client_free(struct pdm_client *client)
{
    if (client) {
        kfree(client);
    }
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Client Module");
