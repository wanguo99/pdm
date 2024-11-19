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
static int pdm_device_uevent(const struct device *dev, struct kobj_uevent_env *env)
{
    const struct pdm_device *pdmdev = dev_to_pdmdev(dev);
    const char *name = pdmdev->name;
    int id = pdmdev->id;
    const char *master_name = pdmdev->master ? pdmdev->master->name : "unknown";

    // 生成 MODALIAS 字符串
    return add_uevent_var(env, "MODALIAS=pdm:master%s-id%04X-name%s", master_name, id, name);
}

static ssize_t id_show(struct device *dev, struct device_attribute *da, char *buf)
{
    const struct pdm_device *pdmdev = dev_to_pdmdev(dev);
    return sprintf(buf, "%d\n", pdmdev->id);
}
static DEVICE_ATTR_RO(id);

static ssize_t name_show(struct device *dev, struct device_attribute *da, char *buf)
{
    const struct pdm_device *pdmdev = dev_to_pdmdev(dev);
    return sprintf(buf, "%s\n", pdmdev->name);
}
static DEVICE_ATTR_RO(name);

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
    &dev_attr_name.attr,
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

	while (id->name[0]) {
		if (strcmp(pdmdev->name, id->name) == 0)
			return id;
		id++;
	}
	return NULL;
}
EXPORT_SYMBOL_GPL(pdm_match_id);


#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
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

	if (pdm_match_id(pdmdrv->id_table, pdmdev))
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


const struct bus_type pdm_bus_type = {
    .name = "pdm",
    .match = pdm_device_match,
    .probe = pdm_device_probe,      // i3c_device_probe i2c_device_remove
    .remove = pdm_device_remove,      // i3c_device_remove i2c_device_remove
};



/*                                                                              */
/*                            pdm_master_type                                   */
/*                                                                              */


static struct list_head pdm_master_list;
static struct mutex     pdm_mutex_lock;

static struct pdm_master *dev_to_pdm_master(struct device *dev)
{
	return container_of(dev, struct pdm_master, dev);
}

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
	.groups	=pdm_master_device_groups,
};

static void pdm_master_device_release(struct device *dev)
{
    struct pdm_master *master = dev_to_pdm_master(dev);
    WARN_ON(!list_empty(&master->clients));
}

int pdm_master_register(struct pdm_master *master)
{
	int ret;

    // 检查设备名称是否已设置
    if (!dev_name(&master->dev))
    {
        printk(KERN_ERR "Master name not set\n");
        return -EINVAL;
    }

	master->dev.bus = &pdm_bus_type;
	master->dev.type = &pdm_master_device_type;
	master->dev.release = pdm_master_device_release;
	device_initialize(&master->dev);
	dev_set_name(&master->dev, "pdm-%s", master->name);

	ret = device_add(&master->dev); // device_del(&master->dev)
	if (ret)
	{
    	put_device(&master->dev);
    	return ret;
	}

	mutex_lock(&pdm_mutex_lock);
    list_add_tail(&master->node, &pdm_master_list);
	mutex_unlock(&pdm_mutex_lock);

	master->init_done = true;

    // TODO: support notify
    // i3c_bus_notify(i3cbus, I3C_NOTIFY_BUS_ADD);

    printk(KERN_INFO "PDM Master registered: %s\n", dev_name(&master->dev));

	return 0;
}


void pdm_master_unregister(struct pdm_master *master) {

    if (NULL == master)
    {
        return;
    }

    // TODO: support notify
    //i3c_bus_notify(&master->bus, I3C_NOTIFY_BUS_REMOVE);

    list_del(&master->node);
    device_unregister(&master->dev);
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
        goto err_out;
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
err_out:
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
