#include <linux/slab.h>

#include "pdm.h"

struct list_head     pdm_master_list;
struct mutex         pdm_master_list_mutex_lock;


/*                                                                              */
/*                            pdm_master_type                                   */
/*                                                                              */

/*
    pdm_master->file_operations
*/
static struct pdm_master *inode_to_pdm_master(struct inode *inode)
{
    struct pdm_master *master;
    struct device *dev;
    struct cdev *cdev;

    cdev = inode->i_cdev;
    if (!cdev) {
        printk(KERN_ERR "Failed to get cdev from inode\n");
        return NULL;
    }

    dev = container_of(&cdev->kobj, struct device, kobj);
    if (!dev) {
        printk(KERN_ERR "Failed to get device from cdev\n");
        return NULL;
    }

    master = dev_to_pdm_master(dev);
    if (!master) {
        printk(KERN_ERR "Failed to convert device to pdm_master\n");
        return NULL;
    }
    return master;
}

static int pdm_master_fops_open_default(struct inode *inode, struct file *filp)
{
    struct pdm_master *master = inode_to_pdm_master(inode);
    filp->private_data = master;
    pr_info("Master %s opened\n", master->name);
    return 0;
}

static int pdm_master_fops_release_default(struct inode *inode, struct file *filp)
{
    struct pdm_master *master = filp->private_data;
    pr_info("Master %s closed\n", master->name);
    return 0;
}

static ssize_t pdm_master_fops_read_default(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
    struct pdm_master *master = filp->private_data;
    pr_info("Master %s read\n", master->name);
    return 0;
}

static ssize_t pdm_master_fops_write_default(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
    struct pdm_master *master = filp->private_data;
    pr_info("Master %s write\n", master->name);
    return 0;
}

static long pdm_master_fops_unlocked_ioctl_default(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct pdm_master *master = filp->private_data;
    pr_info("Master %s do not support ioctl operations.\n", master->name);
    return -ENOTSUPP;
}


/*
    pdm_master_device_type
*/
static ssize_t name_show(struct device *dev, struct device_attribute *da, char *buf)
{
    struct pdm_master *master = dev_to_pdm_master(dev);
    ssize_t ret;

    down_read(&master->rwlock);
    ret = sysfs_emit(buf, "%s\n", master->name);
    up_read(&master->rwlock);

    return ret;
}

static DEVICE_ATTR_RO(name);

static struct attribute *pdm_master_device_attrs[] = {
    &dev_attr_name.attr,
    NULL,
};
ATTRIBUTE_GROUPS(pdm_master_device);

static const struct device_type pdm_master_device_type = {
    .groups = pdm_master_device_groups,
};

static void pdm_master_dev_release(struct device *dev)
{
    struct pdm_master *master = dev_to_pdm_master(dev);
    printk(KERN_INFO "Master %s released.\n", dev_name(&master->dev));
    //WARN_ON(!list_empty(&master->clients));
    return;
}


static void pdm_master_class_dev_release(struct device *dev)
{
    struct pdm_master *master;
    master = dev_to_pdm_master(dev);
    kfree(master);
}

struct class pdm_master_class = {
    .name       = "pdm_master",
    .dev_release    = pdm_master_class_dev_release,
};


static void pdm_master_class_cdev_release(struct device *dev)
{
#if 1
    struct pdm_master *master;
    master = dev_to_pdm_master(dev);
    kfree(master);
#endif
    return;
}

struct class pdm_master_cdev_class = {
    .name       = "pdm_master_cdev",
    .dev_release    = pdm_master_class_cdev_release,
};

static int pdm_master_add_cdev(struct pdm_master *master)
{
    int ret;

    // 分配设备号
    ret = alloc_chrdev_region(&master->devno, 0, 1, dev_name(&master->dev));
    if (ret < 0) {
        printk(KERN_ERR "Failed to allocate char device region for %s, error: %d\n", dev_name(&master->dev), ret);
        return ret;
    }

    // 初始化默认文件操作句柄，在master注册后，可以动态修改为实际的操作句柄
    master->fops.open = pdm_master_fops_open_default;
    master->fops.release = pdm_master_fops_release_default;
    master->fops.read = pdm_master_fops_read_default;
    master->fops.write = pdm_master_fops_write_default;
    master->fops.unlocked_ioctl = pdm_master_fops_unlocked_ioctl_default;

    // 初始化字符设备
    cdev_init(&master->cdev, &master->fops);
    master->cdev.owner = THIS_MODULE;

    ret = cdev_add(&master->cdev, master->devno, 1);
    if (ret < 0) {
        unregister_chrdev_region(master->devno, 1);
        printk(KERN_ERR "Failed to add char device for %s, error: %d\n", dev_name(&master->dev), ret);
        return ret;
    }

    // 注册到pdm_master_class
    device_create(&pdm_master_cdev_class, NULL, master->devno, NULL, "pdm_master_%s", master->name);

    printk(KERN_INFO "Add char device for %s ok\n", dev_name(&master->dev));

    return 0;
}

// 卸载字符设备
static void pdm_master_delete_cdev(struct pdm_master *master)
{
    device_destroy(&pdm_master_cdev_class, master->devno);
    cdev_del(&master->cdev);
    unregister_chrdev_region(master->devno, 1);
}

int pdm_master_register(struct pdm_master *master)
{
    int ret;
    struct pdm_master *existing_master;

    // 检查设备名称是否已设置
    if (!strlen(master->name)) {
        printk(KERN_ERR "Master name not set\n");
        return -EINVAL;
    }

    // 检查设备名称是否已存在
    mutex_lock(&pdm_master_list_mutex_lock);
    list_for_each_entry(existing_master, &pdm_master_list, node) {
        if (strcmp(existing_master->name, master->name) == 0) {
            printk(KERN_ERR "Master name already exists: %s\n", master->name);
            ret = -EEXIST;
            goto err_unlock;
        }
    }
    mutex_unlock(&pdm_master_list_mutex_lock);

    // master->dev.bus = &pdm_bus_type;
    master->dev.type = &pdm_master_device_type;
    master->dev.class = &pdm_master_class;
    master->dev.parent = &pdm_bus_root;
    master->dev.release = pdm_master_dev_release;
    dev_set_name(&master->dev, "pdm_master_%s", master->name);

    printk(KERN_INFO "Trying to add device: %s\n", dev_name(&master->dev));
    ret = device_add(&master->dev);
    if (ret) {
        printk(KERN_ERR "Failed to add device: %s, error: %d\n", dev_name(&master->dev), ret);
        goto err_put_device;
    }

    mutex_lock(&pdm_master_list_mutex_lock);
    list_add_tail(&master->node, &pdm_master_list);
    mutex_unlock(&pdm_master_list_mutex_lock);

    // 创建字符设备文件
    ret = pdm_master_add_cdev(master);
    if (ret) {
        printk(KERN_ERR "Failed to pdm_master_add_cdev, error: %d\n", ret);
        goto err_del_device;
    }

    master->init_done = true;
    printk(KERN_INFO "PDM Master registered: %s\n", dev_name(&master->dev));

    return 0;

err_del_device:
    device_del(&master->dev);

err_put_device:
    put_device(&master->dev);

err_unlock:
    mutex_unlock(&pdm_master_list_mutex_lock);
    return ret;
}


void pdm_master_unregister(struct pdm_master *master)
{
    if (NULL == master)
    {
        printk(KERN_ERR "pdm_master_unregister: master is NULL\n");
        return;
    }

    pdm_master_delete_cdev(master);

    mutex_lock(&pdm_master_list_mutex_lock);
    list_del(&master->node);
    mutex_unlock(&pdm_master_list_mutex_lock);

    device_unregister(&master->dev);

    printk(KERN_INFO "PDM Master unregistered: %s\n", dev_name(&master->dev));
}


// 申请一段连续内存，保存master的公共数据和私有数据
struct pdm_master *pdm_master_alloc(unsigned int size)
{
    struct pdm_master   *master;
    size_t master_size = sizeof(struct pdm_master);

    master = kzalloc(size + master_size, GFP_KERNEL);
    if (!master)
        return NULL;

    device_initialize(&master->dev);
    master->dev.class = &pdm_master_class;
    pdm_master_set_devdata(master, (void *)master + master_size);

    return master;
}

struct pdm_master *pdm_master_get(struct pdm_master *master)
{
    if (!master || !get_device(&master->dev))
        return NULL;
    return master;
}

void pdm_master_put(struct pdm_master *master)
{
    if (master)
        put_device(&master->dev);
}

int pdm_master_add_device(struct pdm_master *master, struct pdm_device *pdmdev)
{
    pdmdev->master = master;

    mutex_lock(&master->client_list_mutex_lock);
    list_add_tail(&pdmdev->node, &master->clients);
    mutex_unlock(&master->client_list_mutex_lock);

    return 0;
}

int pdm_master_delete_device(struct pdm_master *master, struct pdm_device *pdmdev)
{
    mutex_lock(&master->client_list_mutex_lock);
    list_del(&pdmdev->node);
    mutex_unlock(&master->client_list_mutex_lock);
    pdmdev->master = NULL;

    return 0;
}


int pdm_master_init(void)
{
    int iRet = 0;

    mutex_init(&pdm_master_list_mutex_lock);
    INIT_LIST_HEAD(&pdm_master_list);

    iRet = class_register(&pdm_master_class);
    if (iRet < 0) {
        pr_err("PDM: Failed to register class\n");
        return iRet;
    }

    iRet = class_register(&pdm_master_cdev_class);
    if (iRet < 0) {
        pr_err("PDM: Failed to register class\n");
        class_destroy(&pdm_master_class);
        return iRet;
    }

    return 0;
}

void pdm_master_exit(void)
{
    class_unregister(&pdm_master_cdev_class);
    //class_destroy(&pdm_master_cdev_class);

    class_unregister(&pdm_master_class);
    //class_destroy(&pdm_master_class);
    return;
}

