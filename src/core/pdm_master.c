#include "linux/compat.h"

#include "pdm.h"
#include "pdm_component.h"
#include "pdm_adapter_drivers.h"

/**
 * @brief PDM 主设备列表
 *
 * 该列表用于存储所有注册的 PDM 主设备。
 */
static struct list_head pdm_master_device_list;

/**
 * @brief 保护 PDM 主设备列表的互斥锁
 *
 * 该互斥锁用于同步对 PDM 主设备列表的访问，防止并发访问导致的数据竞争。
 */
static struct mutex pdm_master_device_list_mutex_lock;

/**
 * @brief 获取PDM主控制器的引用
 *
 * 该函数用于获取 PDM 主控制器的引用计数。
 *
 * @param master PDM主控制器
 * @return 指向PDM主控制器的指针，或NULL（失败）
 */
static struct pdm_master *pdm_master_get(struct pdm_master *master)
{
    if (!master || !get_device(master->dev)) {
        OSA_ERROR("Invalid input parameter or unable to get device reference (master: %p).\n", master);
        return NULL;
    }

    return master;
}

/**
 * @brief 释放PDM主控制器的引用
 *
 * 该函数用于释放 PDM 主控制器的引用计数。
 *
 * @param master PDM主控制器
 */
static void pdm_master_put(struct pdm_master *master)
{
    if (master) {
        put_device(master->dev);
    }
}

/**
 * @brief 从设备结构转换为PDM主控制器
 *
 * 该函数用于从给定的设备结构中提取关联的 PDM 主控制器。
 *
 * @param dev 设备结构
 * @return 指向PDM主控制器的指针，或NULL（失败）
 */
struct pdm_master *dev_to_pdm_master(struct device *dev)
{
    struct pdm_master *master;

    if (!dev) {
        OSA_ERROR("Invalid input parameter.\n");
        return NULL;
    }

    master = (struct pdm_master *)dev_get_drvdata(dev);
    return master;
}

/**
 * @brief 显示所有已注册的 PDM 设备列表
 *
 * 该函数用于显示当前已注册的所有 PDM 设备的名称。
 * 如果主设备未初始化，则会返回错误。
 *
 * @param master PDM主控制器
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_master_client_show(struct pdm_master *master)
{
    struct pdm_device *client;
    int index = 1;

    if (!master) {
        OSA_ERROR("Master is not initialized.\n");
        return -ENODEV;
    }

    OSA_INFO("-------------------------\n");
    OSA_INFO("Device List:\n");

    mutex_lock(&master->client_list_mutex_lock);
    list_for_each_entry(client, &master->client_list, entry) {
        OSA_INFO("  [%d] Client Name: %s.\n", index++, client->name);
    }
    mutex_unlock(&master->client_list_mutex_lock);

    OSA_INFO("-------------------------\n");

    return 0;
}

/**
 * @brief 分配PDM设备ID
 *
 * 该函数用于分配一个唯一的ID给PDM设备。
 *
 * @param master PDM主控制器
 * @param pdmdev PDM设备
 * @return 成功返回 0，失败返回负错误码
 */
static int pdm_master_client_id_alloc(struct pdm_master *master, struct pdm_device *pdmdev)
{
    int id;
    int client_index = 0;

    if (!pdmdev) {
        OSA_ERROR("Invalid input parameters.\n");
        return -EINVAL;
    }

    if (of_property_read_s32(pdmdev->dev.parent->of_node, "client-index", &client_index)) {
        if (pdmdev->force_dts_id) {
            OSA_ERROR("Cannot get index from dts, force_dts_id was set\n");
            return -EINVAL;
        }
        OSA_DEBUG("Cannot get index from dts\n");
    }

    if (client_index < 0) {
        OSA_ERROR("Invalid client index: %d.\n", pdmdev->client_index);
        return -EINVAL;
    }

    mutex_lock(&master->idr_mutex_lock);
    id = idr_alloc(&master->device_idr, NULL, client_index, PDM_MASTER_CLIENT_IDR_END, GFP_KERNEL);
    mutex_unlock(&master->idr_mutex_lock);

    if (id < 0) {
        if (id == -ENOSPC) {
            OSA_ERROR("No available IDs in the range.\n");
            return -EBUSY;
        } else {
            OSA_ERROR("Failed to allocate ID: %d.\n", id);
            return id;
        }
    }

    pdmdev->client_index = id;
    return 0;
}

/**
 * @brief 释放PDM设备的ID
 *
 * 该函数用于释放PDM设备的ID。
 *
 * @param master PDM主控制器
 * @param pdmdev PDM设备
 */
static void pdm_master_client_id_free(struct pdm_master *master, struct pdm_device *pdmdev)
{
    mutex_lock(&master->idr_mutex_lock);
    idr_remove(&master->device_idr, pdmdev->client_index);
    mutex_unlock(&master->idr_mutex_lock);
}

/**
 * @brief 向PDM主控制器添加设备
 *
 * 该函数用于向PDM主控制器添加一个新的PDM设备。
 *
 * @param master PDM主控制器
 * @param pdmdev 要添加的PDM设备
 * @return 成功返回 0，参数无效返回 -EINVAL
 */
int pdm_master_client_add(struct pdm_master *master, struct pdm_device *pdmdev)
{
    int status;

    if (!master || !pdmdev) {
        OSA_ERROR("Invalid input parameters (master: %p, pdmdev: %p).\n", master, pdmdev);
        return -EINVAL;
    }

    pdmdev->master = pdm_master_get(master);

    status = pdm_master_client_id_alloc(master, pdmdev);
    if (status) {
        OSA_ERROR("Alloc id for client failed: %d\n", status);
        return status;
    }

    snprintf(pdmdev->name, PDM_DEVICE_NAME_SIZE, "pdm_%s_client.%d", master->name, pdmdev->client_index);

    mutex_lock(&master->client_list_mutex_lock);
    list_add_tail(&pdmdev->entry, &master->client_list);
    mutex_unlock(&master->client_list_mutex_lock);

    return 0;
}

/**
 * @brief 从PDM主控制器删除设备
 *
 * 该函数用于从PDM主控制器中删除一个PDM设备。
 *
 * @param master PDM主控制器
 * @param pdmdev 要删除的PDM设备
 * @return 成功返回 0，参数无效返回 -EINVAL
 */
int pdm_master_client_delete(struct pdm_master *master, struct pdm_device *pdmdev)
{
    if (!master || !pdmdev) {
        OSA_ERROR("Invalid input parameters (master: %p, pdmdev: %p).\n", master, pdmdev);
        return -EINVAL;
    }

    mutex_lock(&master->client_list_mutex_lock);
    list_del(&pdmdev->entry);
    mutex_unlock(&master->client_list_mutex_lock);
    pdm_master_client_id_free(master, pdmdev);
    pdm_master_put(master);

    OSA_DEBUG("Device %s removed from %s master.\n", dev_name(&pdmdev->dev), master->name);
    return 0;
}

/**
 * @brief 默认打开函数
 *
 * 该函数是默认的文件打开操作。
 *
 * @param inode inode 结构
 * @param filp 文件结构
 * @return 成功返回 0
 */
static int pdm_master_fops_default_open(struct inode *inode, struct file *filp)
{
    struct pdm_master *master;

    OSA_INFO("fops_default_open.\n");

    master = container_of(inode->i_cdev, struct pdm_master, cdev);
    if (!master) {
        OSA_ERROR("Invalid master.\n");
        return -EINVAL;
    }

    filp->private_data = master;

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
static int pdm_master_fops_default_release(struct inode *inode, struct file *filp)
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
static ssize_t pdm_master_fops_default_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
    struct pdm_master *master;

    OSA_INFO("fops_default_read.\n");

    master = filp->private_data;
    if (!master) {
        OSA_ERROR("Invalid master.\n");
        return -EINVAL;
    }

    (void)pdm_master_client_show(master);
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
static ssize_t pdm_master_fops_default_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
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
static long pdm_master_fops_default_unlocked_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    OSA_INFO("Called pdm_master_fops_default_unlocked_ioctl\n");

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
static long pdm_master_fops_default_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    OSA_INFO("pdm_master_fops_default_compat_ioctl.\n");

    if (_IOC_DIR(cmd) & (_IOC_READ | _IOC_WRITE)) {
        arg = (unsigned long)compat_ptr(arg);
    }

    return filp->f_op->unlocked_ioctl(filp, cmd, arg);
}

/**
 * @brief 显示设备名称
 *
 * 该函数用于在sysfs中显示设备名称。
 *
 * @param dev 设备结构
 * @param da 设备属性结构
 * @param buf 输出缓冲区
 * @return 实际写入的字节数
 */
static ssize_t name_show(struct device *dev, struct device_attribute *da, char *buf)
{
    return sysfs_emit(buf, "%s\n", dev_name(dev));
}

static DEVICE_ATTR_RO(name);

static struct attribute *pdm_master_device_attrs[] = {
    &dev_attr_name.attr,
    NULL,
};
ATTRIBUTE_GROUPS(pdm_master_device);

/**
 * @brief PDM 主控制器类
 *
 * 该类用于管理PDM主控制器设备。
 */
static struct class pdm_master_class = {
    .name = "pdm_master",
    .dev_groups = pdm_master_device_groups,
};

/**
 * @brief 添加PDM主控制器字符设备
 *
 * 该函数用于注册PDM主控制器字符设备。
 *
 * @param master PDM主控制器
 * @return 成功返回 0，失败返回负错误码
 */
static int pdm_master_device_register(struct pdm_master *master)
{
    int status;
    struct device *master_dev;
    char device_name[PDM_DEVICE_NAME_SIZE];

    if (!master) {
        OSA_ERROR("Invalid input parameter.\n");
        return -EINVAL;
    }

    memset(device_name, 0, PDM_DEVICE_NAME_SIZE);
    snprintf(device_name, PDM_DEVICE_NAME_SIZE, "pdm_master_%s", master->name);
    status = alloc_chrdev_region(&master->devno, 0, 1, device_name);
    if (status < 0) {
        OSA_ERROR("Failed to allocate char device region for %s, error: %d.\n", master->name, status);
        goto err_out;
    }

    master->fops.open = pdm_master_fops_default_open;
    master->fops.release = pdm_master_fops_default_release;
    master->fops.read = pdm_master_fops_default_read;
    master->fops.write = pdm_master_fops_default_write;
    master->fops.unlocked_ioctl = pdm_master_fops_default_unlocked_ioctl;
    master->fops.compat_ioctl =  pdm_master_fops_default_compat_ioctl;

    cdev_init(&master->cdev, &master->fops);
    master->cdev.owner = THIS_MODULE;

    status = cdev_add(&master->cdev, master->devno, 1);
    if (status < 0) {
        OSA_ERROR("Failed to add char device for %s, error: %d.\n", master->name, status);
        goto err_unregister_chrdev;
    }

    master_dev = device_create(&pdm_master_class, NULL, master->devno, NULL, device_name);
    if (IS_ERR(master_dev)) {
        OSA_ERROR("Failed to create device for %s.\n", master->name);
        goto err_cdev_del;
    }

    master->dev = master_dev;
    dev_set_drvdata(master->dev, master);   // 保存master地址至dev_drvdata
    OSA_DEBUG("PDM Master %s Device Registered.\n", master->name);

    return 0;

err_cdev_del:
    cdev_del(&master->cdev);
err_unregister_chrdev:
    unregister_chrdev_region(master->devno, 1);
err_out:
    return status;
}

/**
 * @brief 删除PDM主控制器字符设备
 *
 * 该函数用于注销PDM主控制器字符设备。
 *
 * @param master PDM主控制器
 */
static void pdm_master_device_unregister(struct pdm_master *master)
{
    if (!master) {
        OSA_ERROR("Invalid input parameter.\n");
    }

    if (master->dev) {
        device_destroy(&pdm_master_class, master->devno);
        master->dev = NULL;
    }

    cdev_del(&master->cdev);

    if (master->devno != 0) {
        unregister_chrdev_region(master->devno, 1);
        master->devno = 0;
    }

    OSA_DEBUG("PDM Master Device Unregistered.\n");
}

/**
 * @brief 获取PDM主控制器的私有数据
 *
 * 该函数用于获取PDM主控制器的私有数据。
 *
 * @param master PDM主控制器
 * @return 指向私有数据的指针
 */
void *pdm_master_priv_data_get(struct pdm_master *master)
{
    if (!master) {
        OSA_ERROR("Invalid input parameter.\n");
        return NULL;
    }

    return master->data;
}

/**
 * @brief 分配PDM主控制器结构
 *
 * 该函数用于分配PDM主控制器结构及其私有数据。
 *
 * @param data_size 私有数据的大小
 * @return 指向分配的PDM主控制器的指针，或NULL（失败）
 */
struct pdm_master *pdm_master_alloc(unsigned int data_size)
{
    struct pdm_master *master;
    size_t master_size = sizeof(struct pdm_master);

    master = kzalloc(master_size + data_size, GFP_KERNEL);
    if (!master) {
        OSA_ERROR("Failed to allocate memory for pdm_master.\n");
        return NULL;
    }

    INIT_LIST_HEAD(&master->client_list);
    mutex_init(&master->client_list_mutex_lock);
    master->data = (void *)master + master_size;

    return master;
}

/**
 * @brief 释放PDM主控制器结构
 *
 * 该函数用于释放PDM主控制器结构。
 *
 * @param master PDM主控制器
 */
void pdm_master_free(struct pdm_master *master)
{
    if (master) {
        pdm_master_put(master);
    }
}

/**
 * @brief 注册 PDM 主控制器
 *
 * 该函数用于注册 PDM 主控制器，并将其添加到主控制器列表中。
 *
 * @param master PDM 主控制器指针
 * @return 0 - 成功
 *         -EINVAL - 参数无效
 *         -EEXIST - 主控制器名称已存在
 *         其他负值 - 其他错误码
 */
int pdm_master_register(struct pdm_master *master)
{
    struct pdm_master *existing_master;
    int status;

    if (!master || !strlen(master->name)) {
        OSA_ERROR("Invalid input parameters (master: %p, name: %s).\n", master, master ? master->name : "NULL");
        return -EINVAL;
    }

    mutex_lock(&pdm_master_device_list_mutex_lock);
    list_for_each_entry(existing_master, &pdm_master_device_list, entry) {
        if (!strcmp(existing_master->name, master->name)) {
            OSA_ERROR("Master already exists: %s.\n", master->name);
            mutex_unlock(&pdm_master_device_list_mutex_lock);
            return -EEXIST;
        }
    }
    mutex_unlock(&pdm_master_device_list_mutex_lock);

    status = pdm_master_device_register(master);
    if (status) {
        OSA_ERROR("Failed to add cdev, error: %d.\n", status);
        return status;
    }

    mutex_lock(&pdm_master_device_list_mutex_lock);
    list_add_tail(&master->entry, &pdm_master_device_list);
    mutex_unlock(&pdm_master_device_list_mutex_lock);

    mutex_init(&master->idr_mutex_lock);
    idr_init(&master->device_idr);

    master->init_done = true;
    OSA_DEBUG("PDM Master Registered: %s.\n", master->name);

    return 0;
}

/**
 * @brief 注销 PDM 主控制器
 *
 * 该函数用于注销 PDM 主控制器，并从主控制器列表中移除。
 *
 * @param master PDM 主控制器指针
 */
void pdm_master_unregister(struct pdm_master *master)
{
    if (!master) {
        OSA_ERROR("Invalid input parameters (master: %p).\n", master);
        return;
    }

    OSA_INFO("PDM Master unregistered: %s.\n", master->name);
    master->init_done = false;

    mutex_lock(&pdm_master_device_list_mutex_lock);
    list_del(&master->entry);
    mutex_unlock(&pdm_master_device_list_mutex_lock);

    pdm_master_device_unregister(master);
}

/**
 * @brief 初始化 PDM 主控制器模块
 *
 * 该函数用于初始化 PDM 主控制器模块，包括注册类和驱动。
 *
 * @return 0 - 成功
 *         负值 - 失败
 */
int pdm_master_init(void)
{
    int status;

    status = class_register(&pdm_master_class);
    if (status < 0) {
        OSA_ERROR("Failed to register PDM Master Class, error: %d.\n", status);
        return status;
    }
    OSA_DEBUG("PDM Master Class registered.\n");

    INIT_LIST_HEAD(&pdm_master_device_list);
    mutex_init(&pdm_master_device_list_mutex_lock);

    status = pdm_master_drivers_register();
    if (status < 0) {
        OSA_ERROR("Failed to register PDM Master Drivers, error: %d.\n", status);
        return status;
    }

    OSA_DEBUG("Initialize PDM Master OK.\n");
    return 0;
}

/**
 * @brief 卸载 PDM 主控制器模块
 *
 * 该函数用于卸载 PDM 主控制器模块，包括注销类和驱动。
 */
void pdm_master_exit(void)
{
    // 注销 PDM 主控制器驱动
    pdm_master_drivers_unregister();

    // 注销 PDM 主控制器类
    class_unregister(&pdm_master_class);
    OSA_DEBUG("PDM Master Class unregistered.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Master Module");
