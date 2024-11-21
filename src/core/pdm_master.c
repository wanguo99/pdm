#include <linux/slab.h>
#include <linux/platform_device.h>

#include "pdm.h"

struct list_head     pdm_master_list;
struct mutex         pdm_master_list_mutex_lock;


/*                                                                              */
/*                            pdm_master_type                                   */
/*                                                                              */

/*
    pdm_master->file_operations
*/
#if 0
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
#endif

static int pdm_master_fops_open_default(struct inode *inode, struct file *filp)
{
    // struct pdm_master *master = inode_to_pdm_master(inode);
    // filp->private_data = master;
    printk(KERN_ERR "[WANGUO] (%s:%d) \n", __func__, __LINE__);
    return 0;
}

static int pdm_master_fops_release_default(struct inode *inode, struct file *filp)
{
    // struct pdm_master *master = filp->private_data;
    printk(KERN_ERR "[WANGUO] (%s:%d) \n", __func__, __LINE__);
    return 0;
}

static ssize_t pdm_master_fops_read_default(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
    // struct pdm_master *master = filp->private_data;
    printk(KERN_ERR "[WANGUO] (%s:%d) \n", __func__, __LINE__);
    return 0;
}

static ssize_t pdm_master_fops_write_default(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
    // struct pdm_master *master = filp->private_data;
    printk(KERN_ERR "[WANGUO] (%s:%d) \n", __func__, __LINE__);
    return 0;
}

static long pdm_master_fops_unlocked_ioctl_default(struct file *filp, unsigned int cmd, unsigned long arg)
{
    // struct pdm_master *master = filp->private_data;
    osa_info("Master do not support ioctl operations.\n");
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

    WARN_ON(!list_empty(&master->clients));
    kfree(dev);

    return;
}


static void pdm_master_class_dev_release(struct device *dev)
{
    struct pdm_master *master;
    master = dev_to_pdm_master(dev);
    if (master)
    {
        kfree(master);
    }
}

struct class pdm_master_class = {
    .name       = "pdm_master",
    .dev_release    = pdm_master_class_dev_release,
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
    device_create(&pdm_master_class, NULL, master->devno, NULL, "pdm_master_%s_cdev", master->name);

    printk(KERN_INFO "Add char device for %s ok\n", dev_name(&master->dev));

    return 0;
}

// 卸载字符设备
static void pdm_master_delete_cdev(struct pdm_master *master)
{
    device_destroy(&pdm_master_class, master->devno);
    cdev_del(&master->cdev);
    unregister_chrdev_region(master->devno, 1);
}

void *pdm_master_get_devdata(struct pdm_master *master)
{
    return dev_get_drvdata(&master->dev);
}

void pdm_master_set_devdata(struct pdm_master *master, void *data)
{
    dev_set_drvdata(&master->dev, data);
}

struct pdm_master *pdm_master_alloc(unsigned int data_size)
{
    struct pdm_master   *master;
    size_t master_size = sizeof(struct pdm_master);

    master = kzalloc(master_size + data_size, GFP_KERNEL);
    if (!master)
        return NULL;

    device_initialize(&master->dev);
    master->dev.class = &pdm_master_class;

    pdm_master_set_devdata(master, (void *)master + master_size);

    return master;
}


void pdm_master_free(struct pdm_master *master)
{
    if (!master)
        return;

    kfree(master);
}

int pdm_master_register(struct pdm_master *master)
{
    int ret;
    struct pdm_master *existing_master;

    if (!strlen(master->name)) {
        printk(KERN_ERR "Master name not set\n");
        return -EINVAL;
    }

    mutex_lock(&pdm_master_list_mutex_lock);
    list_for_each_entry(existing_master, &pdm_master_list, node)
    {
        if (!strcmp(existing_master->name, master->name))
        {
            printk(KERN_ERR "Master name already exists: %s\n", master->name);
            mutex_unlock(&pdm_master_list_mutex_lock);
            return -EEXIST;
        }
    }
    mutex_unlock(&pdm_master_list_mutex_lock);

    master->dev.type = &pdm_master_device_type;
    master->dev.class = &pdm_master_class;
    master->dev.release = pdm_master_dev_release;
    dev_set_name(&master->dev, "pdm_master_%s", master->name);
    ret = device_add(&master->dev);
    if (ret) {
        printk(KERN_ERR "Failed to add device: %s, error: %d\n", dev_name(&master->dev), ret);
        goto err_put_device;
    }

    ret = pdm_master_add_cdev(master);
    if (ret) {
        printk(KERN_ERR "Failed to pdm_master_add_cdev, error: %d\n", ret);
        goto err_del_device;
    }

    mutex_init(&master->client_list_mutex_lock);
    INIT_LIST_HEAD(&master->clients);

    idr_init(&master->device_idr);

    mutex_lock(&pdm_master_list_mutex_lock);
    list_add_tail(&master->node, &pdm_master_list);
    mutex_unlock(&pdm_master_list_mutex_lock);

    master->init_done = true;
    printk(KERN_INFO "PDM Master registered: %s\n", dev_name(&master->dev));

    return 0;

err_del_device:
    device_del(&master->dev);

err_put_device:
    put_device(&master->dev);

    return ret;
}


void pdm_master_unregister(struct pdm_master *master)
{
    if (NULL == master)
    {
        printk(KERN_ERR "pdm_master_unregister: master is NULL\n");
        return;
    }

    master->init_done = false;

    mutex_lock(&pdm_master_list_mutex_lock);
    list_del(&master->node);
    mutex_unlock(&pdm_master_list_mutex_lock);

    idr_destroy(&master->device_idr);
    pdm_master_delete_cdev(master);
    device_unregister(&master->dev);

    printk(KERN_INFO "PDM Master unregistered: %s\n", dev_name(&master->dev));
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

    return 0;
}


// 遍历master，查找real_device对应的pdm_device
struct pdm_device *pdm_master_get_pdmdev_of_real_device(struct pdm_master *master, void *real_device)
{
    struct pdm_device *existing_pdmdev;

    mutex_lock(&master->client_list_mutex_lock);
    list_for_each_entry(existing_pdmdev, &master->clients, node)
    {
        if (existing_pdmdev->real_device == real_device)
        {
            mutex_unlock(&master->client_list_mutex_lock);
            return existing_pdmdev;
        }
    }

    printk(KERN_ERR "%s:%d:[%s] Failed \n", __FILE__, __LINE__, __func__);
    return NULL;
}

int pdm_master_init(void)
{
    int iRet = 0;

    mutex_init(&pdm_master_list_mutex_lock);
    INIT_LIST_HEAD(&pdm_master_list);

    iRet = class_register(&pdm_master_class);
    if (iRet < 0) {
        osa_error("PDM: Failed to register class\n");
        return iRet;
    }
    osa_info("Register PDM Master Class.\n");

    return 0;
}

void pdm_master_exit(void)
{
    class_unregister(&pdm_master_class);
    return;
}

