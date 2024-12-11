#include "linux/compat.h"

#include "pdm.h"

/**
 * @brief PDM 主控制器类
 */
static struct class pdm_client_class = {
    .name = "pdm_client",
};


/**
 * @brief 默认打开函数
 * @inode: inode 结构
 * @filp: 文件结构
 *
 * 返回值:
 * 0 - 成功
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
 * @brief 默认释放函数
 * @inode: inode 结构
 * @filp: 文件结构
 *
 * 返回值:
 * 0 - 成功
 */
static int pdm_client_fops_default_release(struct inode *inode, struct file *filp)
{
    OSA_INFO("fops_default_release.\n");
    return 0;
}

/**
 * @brief 默认读取函数
 * @filp: 文件结构
 * @buf: 用户空间缓冲区
 * @count: 要读取的字节数
 * @ppos: 当前文件位置
 *
 * 返回值:
 * 0 - 成功
 */
static ssize_t pdm_client_fops_default_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
    OSA_INFO("fops_default_read.\n");
    return 0;
}

/**
 * @brief 默认写入函数
 * @filp: 文件结构
 * @buf: 用户空间缓冲区
 * @count: 要写入的字节数
 * @ppos: 当前文件位置
 *
 * 返回值:
 * 0 - 成功
 */
static ssize_t pdm_client_fops_default_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
    OSA_INFO("fops_default_write.\n");
    return count;
}

/**
 * @brief 默认ioctl函数
 * @filp: 文件结构
 * @cmd: ioctl命令
 * @arg: 命令参数
 *
 * 返回值:
 * -ENOTSUPP - 不支持的ioctl操作
 */
static long pdm_client_fops_default_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    OSA_INFO("This client does not support ioctl operations.\n");
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

/**
 * @brief 添加 PDM Client 字符设备
 *
 * 该函数用于注册 PDM Client 字符设备。
 *
 * @param client  PDM Client
 * @return 成功返回 0，失败返回负错误码
 */
static int pdm_client_device_register(struct pdm_client *client)
{
    int status;
    struct device *client_dev;
    char device_name[PDM_CLIENT_NAME_SIZE];

    if (!client) {
        OSA_ERROR("Invalid input parameter.\n");
        return -EINVAL;
    }

    memset(device_name, 0, PDM_CLIENT_NAME_SIZE);
    snprintf(device_name, PDM_CLIENT_NAME_SIZE, "pdm_%s_%d", client->name, client->index);
    status = alloc_chrdev_region(&client->devno, 0, 1, device_name);
    if (status < 0) {
        OSA_ERROR("Failed to allocate char device region for %s, error: %d.\n", client->name, status);
        goto err_out;
    }

    client->fops.open = pdm_client_fops_default_open;
    client->fops.release = pdm_client_fops_default_release;
    client->fops.read = pdm_client_fops_default_read;
    client->fops.write = pdm_client_fops_default_write;
    client->fops.unlocked_ioctl = pdm_client_fops_default_ioctl;
    client->fops.compat_ioctl =  pdm_client_fops_default_compat_ioctl;

    cdev_init(&client->cdev, &client->fops);
    client->cdev.owner = THIS_MODULE;

    status = cdev_add(&client->cdev, client->devno, 1);
    if (status < 0) {
        OSA_ERROR("Failed to add char device for %s, error: %d.\n", client->name, status);
        goto err_unregister_chrdev;
    }

    client_dev = device_create(&pdm_client_class, NULL, client->devno, NULL, device_name);
    if (IS_ERR(client_dev)) {
        OSA_ERROR("Failed to create device for %s.\n", client->name);
        goto err_cdev_del;
    }

    client->dev = client_dev;
    dev_set_drvdata(client->dev, client);   // 保存client地址至dev_drvdata
    OSA_DEBUG("PDM Master %s Device Registered.\n", client->name);

    return 0;

err_cdev_del:
    cdev_del(&client->cdev);
err_unregister_chrdev:
    unregister_chrdev_region(client->devno, 1);
err_out:
    return status;
}

/**
 * @brief 删除 PDM Client 字符设备
 *
 * 该函数用于注销 PDM Client 字符设备。
 *
 * @param client  PDM Client
 */
static void pdm_client_device_unregister(struct pdm_client *client)
{
    if (!client) {
        OSA_ERROR("Invalid input parameter.\n");
    }

    if (client->dev) {
        device_destroy(&pdm_client_class, client->devno);
        client->dev = NULL;
    }

    cdev_del(&client->cdev);

    if (client->devno != 0) {
        unregister_chrdev_region(client->devno, 1);
        client->devno = 0;
    }

    OSA_DEBUG("PDM Master Device Unregistered.\n");
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

    status = pdm_adapter_id_alloc(adapter, client);
    if (status) {
        OSA_ERROR("Alloc id for client failed: %d\n", status);
        return status;
    }

    snprintf(client->name, PDM_DEVICE_NAME_SIZE, "pdm_%s_client.%d", dev_name(&adapter->dev), client->index);
    status = pdm_client_device_register(client);
    if (status) {
        OSA_ERROR("Failed to add cdev, error: %d.\n", status);
        goto err_free_id;
    }

    mutex_lock(&adapter->client_list_mutex_lock);
    list_add_tail(&client->entry, &adapter->client_list);
    mutex_unlock(&adapter->client_list_mutex_lock);

err_free_id:
    pdm_adapter_id_free(adapter, client);

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
    OSA_DEBUG("Device %s removed from %s adapter.\n", client->name, adapter->name);

    mutex_lock(&adapter->client_list_mutex_lock);
    list_del(&client->entry);
    mutex_unlock(&adapter->client_list_mutex_lock);

    pdm_client_device_unregister(client);
    pdm_adapter_id_free(adapter, client);
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

/**
 * @brief 初始化 PDM Client 模块
 *
 * 返回值:
 * 0 - 成功
 * 负值 - 失败
 */
int pdm_client_init(void)
{
    int status;

    status = class_register(&pdm_client_class);
    if (status < 0) {
        OSA_ERROR("Failed to register PDM Client Class, error: %d.\n", status);
        return status;
    }
    OSA_DEBUG("PDM Client Class registered.\n");

    OSA_DEBUG("Initialize PDM Client OK.\n");
    return 0;
}

/**
 * @brief 卸载 PDM Client 模块
 */
void pdm_client_exit(void)
{
    class_unregister(&pdm_client_class);
    OSA_DEBUG("PDM Master Class unregistered.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Client Module");
