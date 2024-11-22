#include <linux/slab.h>
#include <linux/platform_device.h>

#include "pdm.h"

static LIST_HEAD(pdm_master_list);
static DEFINE_MUTEX(pdm_master_list_mutex_lock);

/**
 * pdm_master_fops_open_default - 默认打开函数
 * @inode: inode 结构
 * @filp: 文件结构
 *
 * 返回值:
 * 0 - 成功
 */
static int pdm_master_fops_open_default(struct inode *inode, struct file *filp)
{
    OSA_INFO("Open function called.\n");
    return 0;
}

/**
 * pdm_master_fops_release_default - 默认释放函数
 * @inode: inode 结构
 * @filp: 文件结构
 *
 * 返回值:
 * 0 - 成功
 */
static int pdm_master_fops_release_default(struct inode *inode, struct file *filp)
{
    OSA_INFO("Release function called.\n");
    return 0;
}

/**
 * pdm_master_fops_read_default - 默认读取函数
 * @filp: 文件结构
 * @buf: 用户空间缓冲区
 * @count: 要读取的字节数
 * @ppos: 当前文件位置
 *
 * 返回值:
 * 0 - 成功
 */
static ssize_t pdm_master_fops_read_default(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
    OSA_INFO("Read function called.\n");
    return 0;
}

/**
 * pdm_master_fops_write_default - 默认写入函数
 * @filp: 文件结构
 * @buf: 用户空间缓冲区
 * @count: 要写入的字节数
 * @ppos: 当前文件位置
 *
 * 返回值:
 * 0 - 成功
 */
static ssize_t pdm_master_fops_write_default(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
    OSA_INFO("Write function called.\n");
    return 0;
}

/**
 * pdm_master_fops_unlocked_ioctl_default - 默认ioctl函数
 * @filp: 文件结构
 * @cmd: ioctl命令
 * @arg: 命令参数
 *
 * 返回值:
 * -ENOTSUPP - 不支持的ioctl操作
 */
static long pdm_master_fops_unlocked_ioctl_default(struct file *filp, unsigned int cmd, unsigned long arg)
{
    OSA_INFO("Master does not support ioctl operations.\n");
    return -ENOTSUPP;
}


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
    struct pdm_master *master = dev_to_pdm_master(dev);
    ssize_t ret;

    down_read(&master->rwlock);
    ret = sysfs_emit(buf, "%s\n", master->name);
    up_read(&master->rwlock);

    OSA_INFO("Device name: %s.\n", master->name);
    return ret;
}

static DEVICE_ATTR_RO(name);

static struct attribute *pdm_master_device_attrs[] = {
    &dev_attr_name.attr,
    NULL,
};
ATTRIBUTE_GROUPS(pdm_master_device);

/**
 * pdm_master_device_type - PDM主控制器设备类型
 */
static const struct device_type pdm_master_device_type = {
    .groups = pdm_master_device_groups,
};

/**
 * pdm_master_class - PDM主控制器类
 */
struct class pdm_master_class = {
    .name = "pdm_master",
};

/**
 * pdm_master_add_cdev - 添加PDM主控制器字符设备
 * @master: PDM主控制器
 *
 * 返回值:
 * 0 - 成功
 * 负值 - 失败
 */
static int pdm_master_add_cdev(struct pdm_master *master)
{
    int ret;

    if (!master) {
        OSA_ERROR("Invalid input parameter (master: %p).\n", master);
        return -EINVAL;
    }

    ret = alloc_chrdev_region(&master->devno, 0, 1, dev_name(&master->dev));
    if (ret < 0) {
        OSA_ERROR("Failed to allocate char device region for %s, error: %d.\n", dev_name(&master->dev), ret);
        goto err_out;
    }

    master->fops.open = pdm_master_fops_open_default;
    master->fops.release = pdm_master_fops_release_default;
    master->fops.read = pdm_master_fops_read_default;
    master->fops.write = pdm_master_fops_write_default;
    master->fops.unlocked_ioctl = pdm_master_fops_unlocked_ioctl_default;

    cdev_init(&master->cdev, &master->fops);
    master->cdev.owner = THIS_MODULE;
    ret = cdev_add(&master->cdev, master->devno, 1);
    if (ret < 0) {
        OSA_ERROR("Failed to add char device for %s, error: %d.\n", dev_name(&master->dev), ret);
        goto err_unregister_chrdev;
    }

    if (!device_create(&pdm_master_class, NULL, master->devno, NULL, "pdm_master_%s_cdev", master->name)) {
        OSA_ERROR("Failed to create device for %s.\n", master->name);
        goto err_cdev_del;
    }

    OSA_INFO("Add cdev for %s ok.\n", dev_name(&master->dev));

    return 0;

err_cdev_del:
    cdev_del(&master->cdev);
err_unregister_chrdev:
    unregister_chrdev_region(master->devno, 1);
err_out:
    return ret;
}


/**
 * pdm_master_delete_cdev - 删除PDM主控制器字符设备
 * @master: PDM主控制器
 */
static void pdm_master_delete_cdev(struct pdm_master *master)
{
    if (!master) {
        OSA_ERROR("Invalid input parameter (master: %p).\n", master);
        return;
    }

    device_destroy(&pdm_master_class, master->devno);
    cdev_del(&master->cdev);
    unregister_chrdev_region(master->devno, 1);
}


/**
 * pdm_master_get_devdata - 获取PDM主控制器的私有数据
 * @master: PDM主控制器
 *
 * 返回值:
 * 指向私有数据的指针
 */
void *pdm_master_get_devdata(struct pdm_master *master)
{
    if (!master) {
        OSA_ERROR("Invalid input parameter (master: %p).\n", master);
        return NULL;
    }

    return dev_get_drvdata(&master->dev);
}

/**
 * pdm_master_set_devdata - 设置PDM主控制器的私有数据
 * @master: PDM主控制器
 * @data: 私有数据指针
 */
void pdm_master_set_devdata(struct pdm_master *master, void *data)
{
    if (!master) {
        OSA_ERROR("Invalid input parameter (master: %p).\n", master);
        return;
    }

    dev_set_drvdata(&master->dev, data);
}

/**
 * pdm_master_get - 获取PDM主控制器的引用
 * @master: PDM主控制器
 *
 * 返回值:
 * 指向PDM主控制器的指针，或NULL（失败）
 */
struct pdm_master *pdm_master_get(struct pdm_master *master)
{
    if (!master || !get_device(&master->dev)) {
        OSA_ERROR("Invalid input parameter or unable to get device reference (master: %p).\n", master);
        return NULL;
    }

    return master;
}

/**
 * pdm_master_put - 释放PDM主控制器的引用
 * @master: PDM主控制器
 */
void pdm_master_put(struct pdm_master *master)
{
    if (master) {
        put_device(&master->dev);
    }
}

/**
 * pdm_master_dev_release - 释放PDM主控制器设备
 * @dev: 设备结构
 */
static void pdm_master_dev_release(struct device *dev)
{
    struct pdm_master *master = dev_to_pdm_master(dev);
    OSA_INFO("Master %s released.\n", dev_name(&master->dev));
    kfree(master);
}

/**
 * pdm_master_alloc - 分配PDM主控制器结构
 * @data_size: 私有数据的大小
 *
 * 返回值:
 * 指向分配的PDM主控制器的指针，或NULL（失败）
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

    device_initialize(&master->dev);
    master->dev.release = pdm_master_dev_release;
    pdm_master_set_devdata(master, (void *)master + master_size);

    return master;
}


/**
 * pdm_master_free - 释放PDM主控制器结构
 * @master: PDM主控制器
 */
void pdm_master_free(struct pdm_master *master)
{
    if (master) {
        pdm_master_put(master);
    }
}

/**
 * pdm_master_id_alloc - 为PDM设备分配ID
 * @master: PDM主控制器
 * @pdmdev: PDM设备
 *
 * 返回值:
 * 0 - 成功
 * -EINVAL - 参数无效
 * -EBUSY - 没有可用的ID
 * 其他负值 - 其他错误码
 */
int pdm_master_id_alloc(struct pdm_master *master, struct pdm_device *pdmdev)
{
    int id;

    if (!master || !pdmdev) {
        OSA_ERROR("Invalid input parameters (master: %p, pdmdev: %p).\n", master, pdmdev);
        return -EINVAL;
    }

    mutex_lock(&master->idr_mutex_lock);
    id = idr_alloc(&master->device_idr, pdmdev, PDM_MASTER_IDR_START, PDM_MASTER_IDR_END, GFP_KERNEL);
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

    pdmdev->id = id;
    return 0;
}

/**
 * pdm_master_id_free - 释放PDM设备的ID
 * @master: PDM主控制器
 * @pdmdev: PDM设备
 */
void pdm_master_id_free(struct pdm_master *master, struct pdm_device *pdmdev)
{
    if (!master || !pdmdev) {
        OSA_ERROR("Invalid input parameters (master: %p, pdmdev: %p).\n", master, pdmdev);
        return;
    }

    mutex_lock(&master->idr_mutex_lock);
    idr_remove(&master->device_idr, pdmdev->id);
    mutex_unlock(&master->idr_mutex_lock);
}


/**
 * pdm_master_register - 注册PDM主控制器
 * @master: PDM主控制器
 *
 * 返回值:
 * 0 - 成功
 * -EINVAL - 参数无效
 * -EBUSY - 设备已存在
 * -EEXIST - 主控制器名称已存在
 * 其他负值 - 其他错误码
 */
int pdm_master_register(struct pdm_master *master)
{
    struct pdm_master *existing_master;
    int ret;

    if (!master || !strlen(master->name)) {
        OSA_ERROR("Invalid input parameters (master: %p, name: %s).\n", master, master ? master->name : "NULL");
        return -EINVAL;
    }

    if (!pdm_master_get(master)) {
        OSA_ERROR("Unable to get reference to master %s.\n", master->name);
        return -EBUSY;
    }

    mutex_lock(&pdm_master_list_mutex_lock);
    list_for_each_entry(existing_master, &pdm_master_list, entry) {
        if (!strcmp(existing_master->name, master->name)) {
            OSA_ERROR("Master already exists: %s.\n", master->name);
            mutex_unlock(&pdm_master_list_mutex_lock);
            pdm_master_put(master);
            return -EEXIST;
        }
    }
    mutex_unlock(&pdm_master_list_mutex_lock);

    master->dev.type = &pdm_master_device_type;
    master->dev.class = &pdm_master_class;
    dev_set_name(&master->dev, "pdm_master_%s", master->name);
    ret = device_add(&master->dev);
    if (ret) {
        OSA_ERROR("Failed to add device: %s, error: %d.\n", dev_name(&master->dev), ret);
        goto err_device_put;
    }

    ret = pdm_master_add_cdev(master);
    if (ret) {
        OSA_ERROR("Failed to add cdev, error: %d.\n", ret);
        goto err_device_unregister;
    }

    mutex_lock(&pdm_master_list_mutex_lock);
    list_add_tail(&master->entry, &pdm_master_list);
    mutex_unlock(&pdm_master_list_mutex_lock);

    mutex_lock(&master->idr_mutex_lock);
    idr_init(&master->device_idr);
    mutex_unlock(&master->idr_mutex_lock);

    mutex_lock(&master->client_list_mutex_lock);
    INIT_LIST_HEAD(&master->client_list);
    mutex_unlock(&master->client_list_mutex_lock);

    master->init_done = true;
    OSA_INFO("PDM Master Registered: %s.\n", dev_name(&master->dev));

    return 0;

err_device_unregister:
    device_unregister(&master->dev);

err_device_put:
    pdm_master_put(master);
    return ret;
}

/**
 * pdm_master_unregister - 注销PDM主控制器
 * @master: PDM主控制器
 */
void pdm_master_unregister(struct pdm_master *master)
{
    if (!master) {
        OSA_ERROR("Invalid input parameters (master: %p).\n", master);
        return;
    }

    mutex_lock(&master->client_list_mutex_lock);
    WARN_ONCE(!list_empty(&master->client_list), "Not all clients removed.");
    mutex_unlock(&master->client_list_mutex_lock);

    master->init_done = false;

    mutex_lock(&pdm_master_list_mutex_lock);
    list_del(&master->entry);
    mutex_unlock(&pdm_master_list_mutex_lock);

    mutex_lock(&master->idr_mutex_lock);
    idr_destroy(&master->device_idr);
    mutex_unlock(&master->idr_mutex_lock);

    pdm_master_delete_cdev(master);
    device_unregister(&master->dev);
    OSA_INFO("PDM Master unregistered: %s.\n", dev_name(&master->dev));
}

/**
 * pdm_master_add_device - 向PDM主控制器添加设备
 * @master: PDM主控制器
 * @pdmdev: 要添加的PDM设备
 *
 * 返回值:
 * 0 - 成功
 * -EINVAL - 参数无效
 */
int pdm_master_add_device(struct pdm_master *master, struct pdm_device *pdmdev)
{
    if (!master || !pdmdev) {
        OSA_ERROR("Invalid input parameters (master: %p, pdmdev: %p).\n", master, pdmdev);
        return -EINVAL;
    }

    pdmdev->master = master;

    mutex_lock(&master->client_list_mutex_lock);
    list_add_tail(&pdmdev->entry, &master->client_list);
    mutex_unlock(&master->client_list_mutex_lock);

    OSA_INFO("Device %p added to master %p.\n", pdmdev, master);
    return 0;
}

/**
 * pdm_master_delete_device - 从PDM主控制器删除设备
 * @master: PDM主控制器
 * @pdmdev: 要删除的PDM设备
 *
 * 返回值:
 * 0 - 成功
 * -EINVAL - 参数无效
 */
int pdm_master_delete_device(struct pdm_master *master, struct pdm_device *pdmdev)
{
    if (!master || !pdmdev) {
        OSA_ERROR("Invalid input parameters (master: %p, pdmdev: %p).\n", master, pdmdev);
        return -EINVAL;
    }

    mutex_lock(&master->client_list_mutex_lock);
    list_del(&pdmdev->entry);
    mutex_unlock(&master->client_list_mutex_lock);

    OSA_INFO("Device %p removed from master %p.\n", pdmdev, master);
    return 0;
}

/**
 * pdm_master_find_pdmdev - 查找与给定实际设备关联的PDM设备
 * @master: PDM主控制器
 * @real_device: 实际设备指针
 *
 * 返回值:
 * 指向找到的PDM设备的指针，或NULL（未找到）
 */
struct pdm_device *pdm_master_find_pdmdev(struct pdm_master *master, void *real_device)
{
    struct pdm_device *existing_pdmdev;

    if (!master) {
        OSA_ERROR("Invalid input parameter (master: %p).\n", master);
        return NULL;
    }

    mutex_lock(&master->client_list_mutex_lock);
    list_for_each_entry(existing_pdmdev, &master->client_list, entry) {
        if (existing_pdmdev->real_device == real_device) {
            mutex_unlock(&master->client_list_mutex_lock);
            OSA_INFO("Device found for real_device at %p.\n", real_device);
            return existing_pdmdev;
        }
    }
    mutex_unlock(&master->client_list_mutex_lock);

    OSA_ERROR("Failed to find device for real_device at %p.\n", real_device);
    return NULL;
}

/**
 * pdm_master_init - 初始化PDM主控制器模块
 *
 * 返回值:
 * 0 - 成功
 * 负值 - 失败
 */
int pdm_master_init(void)
{
    int ret = class_register(&pdm_master_class);
    if (ret < 0) {
        OSA_ERROR("Failed to register PDM Master Class, error: %d.\n", ret);
        return ret;
    }

    OSA_INFO("PDM Master Class registered.\n");
    return 0;
}

/**
 * pdm_master_exit - 卸载PDM主控制器模块
 */
void pdm_master_exit(void)
{
    class_unregister(&pdm_master_class);
    OSA_INFO("PDM Master Class unregistered.\n");
}
