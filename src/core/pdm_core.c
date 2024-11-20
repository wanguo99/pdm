#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/idr.h>
#include <linux/of.h>
#include <linux/debugfs.h>
#include <linux/version.h>

#include "pdm.h"
#include "pdm_submodule.h"




/*                                                                              */
/*                         pdm_device_type                                      */
/*                                                                              */
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
static int pdm_device_uevent(struct device *dev, struct kobj_uevent_env *env)
#else
static int pdm_device_uevent(const struct device *dev, struct kobj_uevent_env *env)
#endif
{
    const struct pdm_device *pdmdev = dev_to_pdmdev(dev);
    const char *compatible = pdmdev->compatible;
    int id = pdmdev->id;
    const char *master_name = pdmdev->master ? pdmdev->master->name : "unknown";

    // 生成 MODALIAS 字符串
    // pdm:pdm_master_cpld:cpld_i2c:0001
    return add_uevent_var(env, "MODALIAS=pdm:pdm_master_%s:%s-%04X", master_name, compatible, id);
}

static ssize_t id_show(struct device *dev, struct device_attribute *da, char *buf)
{
    const struct pdm_device *pdmdev = dev_to_pdmdev(dev);
    return sprintf(buf, "%d\n", pdmdev->id);
}
static DEVICE_ATTR_RO(id);

static ssize_t compatible_show(struct device *dev, struct device_attribute *da, char *buf)
{
    const struct pdm_device *pdmdev = dev_to_pdmdev(dev);
    return sprintf(buf, "%s\n", pdmdev->compatible);
}
static DEVICE_ATTR_RO(compatible);

static ssize_t parent_name_show(struct device *dev, struct device_attribute *da, char *buf)
{
    const struct pdm_device *pdmdev = dev_to_pdmdev(dev);
    const char *master_name = pdmdev->master ? pdmdev->master->name : "unknown";
    return sprintf(buf, "%s\n", master_name);
}
static DEVICE_ATTR_RO(parent_name);

// 属性组定义
static struct attribute *pdm_device_attrs[] = {
    &dev_attr_id.attr,
    &dev_attr_compatible.attr,
    &dev_attr_parent_name.attr,
    NULL,
};

// ATTRIBUTE_GROUPS将pdm_device(_addr)注册到pdm_deivce(_groups)和pdm_deivce(_groups)
ATTRIBUTE_GROUPS(pdm_device);

const struct device_type pdm_device_type = {
    .name = "pdm_device",
    .groups = pdm_device_groups,
    .uevent = pdm_device_uevent,
};


/*                                                                              */
/*                            pdm_bus_type                                      */
/*                                                                              */
const struct pdm_device_id *pdm_match_id(const struct pdm_device_id *id, struct pdm_device *pdmdev)
{
	if (!(id && pdmdev))
		return NULL;

	while (id->compatible[0]) {
		if (strcmp(pdmdev->compatible, id->compatible) == 0)
			return id;
		id++;
	}
	return NULL;
}
EXPORT_SYMBOL_GPL(pdm_match_id);


#if LINUX_VERSION_CODE <= KERNEL_VERSION(5, 15, 0)
static int pdm_device_match(struct device *dev, const struct device_driver *drv) {
#else
static int pdm_device_match(struct device *dev, struct device_driver *drv) {
#endif
	struct pdm_device *pdmdev;
	struct pdm_driver *pdmdrv;

	if (dev->type != &pdm_device_type)
		return 0;

	pdmdev = dev_to_pdmdev(dev);

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
    pdmdrv = drv_to_pdmdrv(drv);
#else
    pdmdrv = drv_to_pdmdrv(drv);
#endif

	if (pdm_match_id(&pdmdrv->id_table, pdmdev))
		return 1;

	return 0;
}


static int pdm_device_probe(struct device *dev)
{
	struct pdm_device *pdmdev = dev_to_pdmdev(dev);
	struct pdm_driver *driver = drv_to_pdmdrv(dev->driver);

	return driver->probe(pdmdev);
}

static void pdm_device_remove(struct device *dev)
{
	struct pdm_device *pdmdev = dev_to_pdmdev(dev);
	struct pdm_driver *driver = drv_to_pdmdrv(dev->driver);

	if (driver->remove)
		driver->remove(pdmdev);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 16, 0)
struct bus_type pdm_bus_type = {
#else
const struct bus_type pdm_bus_type = {
#endif
    .name = "pdm",
    .match = pdm_device_match,
    .probe = pdm_device_probe,      // i3c_device_probe i2c_device_remove
    .remove = pdm_device_remove,      // i3c_device_remove i2c_device_remove
};

/*                                                                              */
/*                        pdm_master->file_operations                            */
/*                                                                              */
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

/*                                                                              */
/*                            pdm_master_type                                   */
/*                                                                              */



static struct list_head pdm_master_list;
static struct mutex     pdm_mutex_lock;

static ssize_t master_name_show(struct device *dev, struct device_attribute *da, char *buf)
{
	struct pdm_master *master = dev_to_pdm_master(dev);
	ssize_t ret;

	down_read(&master->rwlock);
	ret = sysfs_emit(buf, "%s\n", master->name);
	up_read(&master->rwlock);

	return ret;
}

static DEVICE_ATTR_RO(master_name);

static struct attribute *pdm_master_device_attrs[] = {
	&dev_attr_master_name.attr,
	NULL,
};
ATTRIBUTE_GROUPS(pdm_master_device);

static const struct device_type pdm_master_device_type = {
	.groups = pdm_master_device_groups,
};

static void pdm_master_device_release(struct device *dev)
{
    struct pdm_master *master = dev_to_pdm_master(dev);
   printk(KERN_INFO "Master %s released.\n", dev_name(&master->dev));
    //WARN_ON(!list_empty(&master->clients));
    return;
}


static void pdm_master_release(struct device *dev)
{
	struct pdm_master *master;

	master = container_of(dev, struct pdm_master, dev);
	kfree(master);
}

static struct class pdm_master_class = {
	.name		= "pdm_master",
	.dev_release	= pdm_master_release,
};

// 添加字符设备
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
    device_create(&pdm_master_class, NULL, master->devno, NULL, "pdm_master_%s", master->name);

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
    mutex_lock(&pdm_mutex_lock);
    list_for_each_entry(existing_master, &pdm_master_list, node) {
        if (strcmp(existing_master->name, master->name) == 0) {
            printk(KERN_ERR "Master name already exists: %s\n", master->name);
            ret = -EEXIST;
            goto err_unlock;
        }
    }
    mutex_unlock(&pdm_mutex_lock);

    device_initialize(&master->dev);
    master->dev.bus = &pdm_bus_type;
    master->dev.type = &pdm_master_device_type;
    //master->dev.class = &pdm_master_class;
    master->dev.release = pdm_master_device_release;
    dev_set_name(&master->dev, "pdm_master_%s", master->name);

    printk(KERN_INFO "Trying to add device: %s\n", dev_name(&master->dev));

    ret = device_add(&master->dev);
    if (ret) {
        printk(KERN_ERR "Failed to add device: %s, error: %d\n", dev_name(&master->dev), ret);
        goto err_put_device;
    }

    mutex_lock(&pdm_mutex_lock);
    list_add_tail(&master->node, &pdm_master_list);
    mutex_unlock(&pdm_mutex_lock);

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
    mutex_unlock(&pdm_mutex_lock);
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

    mutex_lock(&pdm_mutex_lock);
    list_del(&master->node);
    mutex_unlock(&pdm_mutex_lock);

    device_unregister(&master->dev);

    // TODO: 支持通知机制
    // i3c_bus_notify(&master->bus, I3C_NOTIFY_BUS_REMOVE);

    printk(KERN_INFO "PDM Master unregistered: %s\n", dev_name(&master->dev));
}


static struct dentry *pdm_debugfs_root;

static int __init pdm_init(void)
{
    int iRet;

    mutex_init(&pdm_mutex_lock);
    INIT_LIST_HEAD(&pdm_master_list);
	pdm_debugfs_root = debugfs_create_dir("pdm", NULL);

    iRet = bus_register(&pdm_bus_type);
    if (iRet < 0) {
        pr_err("PDM: Failed to register bus\n");
        goto err_debugfs_destroy;
    }

    iRet = class_register(&pdm_master_class);
    if (iRet < 0) {
        pr_err("PDM: Failed to register class\n");
        goto err_bus_unregister;
    }

    iRet = pdm_submodule_register_drivers();
    if (iRet < 0) {
        pr_err("PDM: Failed to register submodule drivers\n");
        goto err_class_unregister;
    }

    pr_info("PDM: Initialized successfully\n");
    return 0;

err_class_unregister:
    class_unregister(&pdm_master_class);
err_bus_unregister:
    bus_unregister(&pdm_bus_type);
err_debugfs_destroy:
	debugfs_remove_recursive(pdm_debugfs_root);
    return iRet;
}

// 模块退出
static void __exit pdm_exit(void)
{
    pdm_submodule_unregister_drivers();
    class_unregister(&pdm_master_class);
    bus_unregister(&pdm_bus_type);
	debugfs_remove_recursive(pdm_debugfs_root);
    pr_info("PDM: Unregistered successfully\n");
}


module_init(pdm_init);
module_exit(pdm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wanguo");
MODULE_DESCRIPTION("Peripheral Driver Module Driver");
