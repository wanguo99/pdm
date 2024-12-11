#include "linux/compat.h"

#include "pdm.h"

/**
 * @brief PDM Client 类
 */
static struct class pdm_client_class = {
    .name = "pdm_client",
};

/**
 * @brief PDM Client 主设备号
 */
static dev_t pdm_client_major;

/**
 * @brief PDM Client ida，用于分配次设备号
 */
static struct ida pdm_client_ida;

static int pdm_client_minor_alloc(void)
{
    return ida_alloc_range(&pdm_client_ida, PDM_CLIENT_FIRST_DEVICE,
                            PDM_CLIENT_MAX_DEVICES - 1, GFP_KERNEL);
}

static void pdm_client_minor_free(unsigned int minor)
{
    ida_free(&pdm_client_ida, minor);
}

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

static inline void pdm_client_put_device(struct pdm_client *client)
{
    if (client)
        put_device(&client->dev);
}

static void pdm_client_device_release(struct device *dev)
{
    struct pdm_client *client;

    client = container_of(dev, struct pdm_client, dev);
    pdm_client_put_device(client);
    kfree(client);
}

static inline void *pdm_client_get_devdata(struct pdm_client *client)
{
    return dev_get_drvdata(&client->dev);
}

static inline void pdm_client_set_devdata(struct pdm_client *client, void *data)
{
    dev_set_drvdata(&client->dev, data);
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
    int minor;

    if (!client) {
        OSA_ERROR("Invalid input parameter.\n");
        status = -EINVAL;
        goto err_out;
    }

    minor = pdm_client_minor_alloc();
    if (minor < 0) {
        status = minor;
        OSA_ERROR("Failed to alloc new minor: %d\n", status);
        goto err_out;
    }

    dev_set_name(&client->dev, "%s.%d", dev_name(&client->adapter->dev), client->index);

    client->dev.devt = MKDEV(pdm_client_major, minor);
    client->dev.class = &pdm_client_class;
    client->dev.parent = &client->pdmdev->dev;
    client->dev.release = pdm_client_device_release;
    device_initialize(&client->dev);

    client->fops.open = pdm_client_fops_default_open;
    client->fops.release = pdm_client_fops_default_release;
    client->fops.read = pdm_client_fops_default_read;
    client->fops.write = pdm_client_fops_default_write;
    client->fops.unlocked_ioctl = pdm_client_fops_default_ioctl;
    client->fops.compat_ioctl =  pdm_client_fops_default_compat_ioctl;
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
    pdm_client_put_device(client);
    pdm_client_minor_free(minor);
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
    cdev_device_del(&client->cdev, &client->dev);
    pdm_client_minor_free(client->dev.devt);

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

    client->adapter = adapter;
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
    OSA_DEBUG("Device %s unregistered.\n", dev_name(&client->dev));

    mutex_lock(&adapter->client_list_mutex_lock);
    list_del(&client->entry);
    mutex_unlock(&adapter->client_list_mutex_lock);

    pdm_client_device_unregister(client);
    pdm_adapter_id_free(adapter, client);
    return;
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

    client = kzalloc(sizeof(struct pdm_client) + data_size, GFP_KERNEL);
    if (!client) {
        OSA_ERROR("Failed to allocate memory for pdm_client.\n");
        return NULL;
    }

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

    status = alloc_chrdev_region(&pdm_client_major, 0, 1, PDM_CLIENT_DEVICE_NAME);
    if (status < 0) {
        OSA_ERROR("Failed to allocate device region for %s, error: %d.\n", PDM_CLIENT_DEVICE_NAME, status);
        class_unregister(&pdm_client_class);
        return status;
    }

    ida_init(&pdm_client_ida);

    OSA_DEBUG("PDM Client Initialize OK.\n");
    return 0;
}

/**
 * @brief 卸载 PDM Client 模块
 */
void pdm_client_exit(void)
{
    unregister_chrdev_region(pdm_client_major, 1);
    class_unregister(&pdm_client_class);
    OSA_DEBUG("PDM Client Exited.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Client Module");
