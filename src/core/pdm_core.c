#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/idr.h>
#include <linux/of.h>
#include <linux/version.h>

#include "pdm.h"
#include "pdm_submodule.h"



struct pdm_bus pdm_bus_instance;

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


// 设备属性定义
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

static ssize_t master_name_show(struct device *dev, struct device_attribute *da, char *buf)
{
    const struct pdm_device *pdmdev = dev_to_pdmdev(dev);
    const char *master_name = pdmdev->master ? pdmdev->master->name : "unknown";
    return sprintf(buf, "%s\n", master_name);
}
static DEVICE_ATTR_RO(master_name);

// 属性组定义
extern struct device_attribute dev_attr_id;
extern struct device_attribute dev_attr_name;
extern struct device_attribute dev_attr_master_name;


static struct attribute *pdm_device_attrs[] = {
    &dev_attr_id.attr,
    &dev_attr_name.attr,
    &dev_attr_master_name.attr,
    NULL,
};

// 将pdm_device(_addr)注册到pdm_deivce(_groups)和pdm_deivce(_groups)
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

static DEFINE_IDR(pdm_bus_idr);
static DEFINE_MUTEX(pdm_core_lock);
static int __pdm_first_dynamic_bus_num;
static BLOCKING_NOTIFIER_HEAD(pdm_bus_notifier);

const struct bus_type pdm_bus_type = {
    .name = "pdm",
    .match = pdm_device_match,
    .probe = pdm_device_probe,      // i3c_device_probe i2c_device_remove
    .remove = pdm_device_remove,      // i3c_device_remove i2c_device_remove
};



/*                                                                              */
/*                            pdm_master_type                                   */
/*                                                                              */
#if 0
#define dev_to_pdm_dev(__dev)	container_of_const(__dev, struct pdm_device, dev)

static struct pdm_master *dev_to_pdm_master(struct device *dev)
{
	return container_of(dev, struct pdm_master, dev);
}

static struct pdm_bus *dev_to_pdm_bus(struct device *dev)
{
	struct pdm_master *master;

	if (dev->type == &pdm_device_type)
		return dev_to_pdm_dev(dev)->bus;

	master = dev_to_pdm_master(dev);

	return &master->bus;
}

static void i3c_masterdev_release(struct device *dev)
{
	struct pdm_master *master = dev_to_pdm_master(dev);
	struct pdm_bus *bus = dev_to_pdm_bus(dev);

	if (master->wq)
		destroy_workqueue(master->wq);

	WARN_ON(!list_empty(&bus->devs.i2c) || !list_empty(&bus->devs.i3c));
	i3c_bus_cleanup(bus);

	of_node_put(dev->of_node);
}


const struct device_type i3c_masterdev_type = {
	.groups	= i3c_masterdev_groups,
};
EXPORT_SYMBOL_GPL(i3c_masterdev_type);


#endif

static int pdm_driver_probe(struct device *dev) {
    struct pdm_device *pdm_dev = dev_to_pdmdev(dev);
    struct pdm_driver *driver = drv_to_pdmdrv(dev->driver);

    if (driver->probe)
        return driver->probe(pdm_dev);

    return 0;
}

static int pdm_driver_remove(struct device *dev) {
    struct pdm_device *pdm_dev = dev_to_pdmdev(dev);
    struct pdm_driver *driver = drv_to_pdmdrv(dev->driver);

    if (driver->remove)
        driver->remove(pdm_dev);

    return 0;
}

int pdm_driver_register(struct pdm_driver *driver) {
    driver->driver.probe = pdm_driver_probe;
    driver->driver.remove = pdm_driver_remove;
    return driver_register(&driver->driver);
}

void pdm_driver_unregister(struct pdm_driver *driver) {
    driver_unregister(&driver->driver);
}

int pdm_device_alloc(struct pdm_device **device, struct pdm_master *master) {
    struct pdm_device *dev;
    int id;

    dev = kmalloc(sizeof(*dev), GFP_KERNEL);
    if (!dev)
        return -ENOMEM;

    dev->master = master;
    id = idr_alloc(&master->device_idr, dev, 0, 0, GFP_KERNEL);
    if (id < 0) {
        kfree(dev);
        return id;
    }

    dev->id = id;
    INIT_LIST_HEAD(&dev->node);

    *device = dev;
    return 0;
}

void pdm_device_free(struct pdm_device *device) {
    if (device) {
        idr_remove(&device->master->device_idr, device->id);
        kfree(device);
    }
}

static void pdm_device_release(struct device *dev) {
    struct pdm_device *device = container_of(dev, struct pdm_device, dev);
    pdm_device_free(device);
}

int pdm_device_register(struct pdm_device *device) {
    int ret;

    device_initialize(&device->dev);
    device->dev.bus = &pdm_bus_type;
    device->dev.parent = &device->master->dev;
    device->dev.type = &pdm_device_type;
    device->dev.release = pdm_device_release;  // 设置 release 回调

    // 检查设备名称是否已设置
    if (dev_name(&device->dev)) {
        ret = 0;
    } else if (device->dev.bus && device->dev.bus->dev_name) {
        ret = dev_set_name(&device->dev, "%s%u", device->dev.bus->dev_name, device->id);
    } else {
        printk(KERN_ERR "[WANGUO] (%s:%d) Device name not set and no bus name available\n", __func__, __LINE__);
        ret = -EINVAL;
    }

    if (ret == 0) {
        ret = device_add(&device->dev);
        if (ret) {
            printk(KERN_ERR "Failed to add PDM device\n");
            return ret;
        }

        list_add_tail(&device->node, &device->master->slaves);
    }

    return ret;
}

void pdm_device_unregister(struct pdm_device *device) {
    if (device) {
        list_del(&device->node);
        device_unregister(&device->dev);
    }
}

int pdm_master_alloc(struct pdm_master **master, const char *name) {
    *master = kzalloc(sizeof(**master), GFP_KERNEL);
    if (!*master)
        return -ENOMEM;

    (*master)->name = kstrdup(name, GFP_KERNEL);
    if (!(*master)->name) {
        kfree(*master);
        return -ENOMEM;
    }

    INIT_LIST_HEAD(&(*master)->slaves);
    INIT_LIST_HEAD(&(*master)->node);
    idr_init(&(*master)->device_idr);

    return 0;
}

void pdm_master_free(struct pdm_master *master) {
    if (master) {
        idr_destroy(&master->device_idr);
        kfree(master->name);
        kfree(master);
    }
}

static void pdm_master_release(struct device *dev) {
    struct pdm_master *master = container_of(dev, struct pdm_master, dev);
    pdm_master_free(master);
}

int pdm_master_register(struct pdm_master *master) {
    int ret;

    master->dev.release = pdm_master_release;  // 设置 release 回调

    // 检查设备名称是否已设置
    if (dev_name(&master->dev)) {
        ret = 0;
    } else if (master->dev.bus && master->dev.bus->dev_name) {
        ret = dev_set_name(&master->dev, "%s%u", master->dev.bus->dev_name, master->dev.id);
    } else {
        dump_stack();
        printk(KERN_ERR "[WANGUO] (%s:%d) Master device name not set and no bus name available\n", __func__, __LINE__);
        ret = -EINVAL;
    }

    if (ret == 0) {
        ret = device_register(&master->dev);
        if (ret) {
            printk(KERN_ERR "Failed to register PDM master device, ret: %d\n", ret);
            return ret;
        }

        list_add_tail(&master->node, &pdm_bus_instance.masters);
    }

    return ret;
}

void pdm_master_unregister(struct pdm_master *master) {
    if (master) {
        list_del(&master->node);
        device_unregister(&master->dev);
    }
}

#define DEBUG_PDM_BUS_NOTIFIER 0        /* 通知器,具体功能还没看懂，后续添加 */

#if DEBUG_PDM_BUS_NOTIFIER
static int pdmdev_notify(struct notifier_block *nb, unsigned long action, void *data)
{
    struct device *dev = data;

    switch (action) {
    case BUS_NOTIFY_ADD_DEVICE:
        pr_info("PDM: Device added: %s\n", dev_name(dev));
        // 进行设备添加的处理
        break;
    case BUS_NOTIFY_DEL_DEVICE:
        pr_info("PDM: Device removed: %s\n", dev_name(dev));
        // 进行设备移除的处理
        break;
    default:
        break;
    }

    return NOTIFY_OK;
}

static struct notifier_block i2cdev_notifier = {
    .notifier_call = pdmdev_notify,
};

static struct notifier_block spidev_notifier = {
    .notifier_call = pdmdev_notify,
};

#endif


static int pdm_bus_init(void)
{
	int ret;

	ret = of_alias_get_highest_id("pdm");
	if (ret >= 0) {
		mutex_lock(&pdm_core_lock);
		__pdm_first_dynamic_bus_num = ret + 1;
		mutex_unlock(&pdm_core_lock);
	}

#if DEBUG_PDM_BUS_NOTIFIER
    // 注册 I2C 总线通知器
        ret = bus_register_notifier(&i2c_bus_type, &i2cdev_notifier);
        if (ret) {
            pr_err("PDM: Failed to register I2C notifier: %d\n", ret);
            return ret;
        }

        // 注册 SPI 总线通知器
        ret = bus_register_notifier(&spi_bus_type, &spidev_notifier);
        if (ret) {
            pr_err("PDM: Failed to register SPI notifier: %d\n", ret);
            bus_unregister_notifier(&i2c_bus_type, &i2cdev_notifier);
            return ret;
        }


	ret = bus_register(&pdm_bus_type);
	if (ret)
		goto out_unreg_notifier;

	return 0;

out_unreg_notifier:
	bus_unregister_notifier(&i2c_bus_type, &i2cdev_notifier);
#else
    ret = bus_register(&pdm_bus_type);
    if (ret)
        return ret;

#endif

	return 0;
}

static void pdm_bus_exit(void)
{
#if DEBUG_PDM_BUS_NOTIFIER
        // 取消注册 I2C 总线通知器
        bus_unregister_notifier(&i2c_bus_type, &i2cdev_notifier);

        // 取消注册 SPI 总线通知器
        bus_unregister_notifier(&spi_bus_type, &spidev_notifier);
#endif

	idr_destroy(&pdm_bus_idr);
	bus_unregister(&pdm_bus_type);
}


// 定义一个私有数据结构体
struct pdm_private_data {
    struct class *pdm_class;
    struct proc_dir_entry *proc_pdm_dir;
};

// 私有数据指针
static struct pdm_private_data *g_pstPdmPrivateData;

// 模块初始化
static int __init pdm_init(void) {
    int ret;

    // 分配私有数据结构体
    g_pstPdmPrivateData = kzalloc(sizeof(*g_pstPdmPrivateData), GFP_KERNEL);
    if (!g_pstPdmPrivateData) {
        pr_err("Failed to allocate g_pstPdmPrivateData\n");
        return -ENOMEM;
    }

    // 初始化 PDM 总线
    ret = pdm_bus_init();
    if (ret) {
        pr_err("Failed to initialize PDM bus\n");
        kfree(g_pstPdmPrivateData);
        return ret;
    }

    // 初始化所有子模块驱动
    ret = pdm_submodule_register_drivers();
    if (ret) {
        pr_err("Failed to initialize subdrivers\n");
        pdm_bus_exit();
        kfree(g_pstPdmPrivateData);
        return ret;
    }

    pr_info("PDM driver framework initialized\n");
    return 0;
}

// 模块退出
static void __exit pdm_exit(void) {
    pdm_submodule_unregister_drivers();
    pdm_bus_exit();
    kfree(g_pstPdmPrivateData);
    pr_info("PDM driver framework unregistered\n");
}

module_init(pdm_init);
module_exit(pdm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wanguo");
MODULE_DESCRIPTION("Peripheral Driver Module Driver");
