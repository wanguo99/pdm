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


static struct dentry       *pdm_debugfs_root;

static void pdm_bus_root_dev_release(struct device *dev)
{
    printk(KERN_INFO "Device %s released.\n", dev_name(dev));
    return;
}

struct device pdm_bus_root = {
	.init_name	= "pdm_bus_root",
};
EXPORT_SYMBOL_GPL(pdm_bus_root);

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
    .probe = pdm_device_probe,
    .remove = pdm_device_remove,
};


/*                                                                              */
/*                              module_init                                     */
/*                                                                              */

static int __init pdm_init(void)
{
    int iRet;

    pdm_bus_root.release = pdm_bus_root_dev_release;
	iRet = device_register(&pdm_bus_root);
	if (iRet) {
		put_device(&pdm_bus_root);
		return iRet;
	}

    iRet = bus_register(&pdm_bus_type);
    if (iRet < 0) {
        pr_err("PDM: Failed to register bus\n");
        goto err_dev_unregister;
    }

    iRet = pdm_master_init();
    if (iRet < 0) {
        pr_err("PDM: Failed to init master\n");
        goto err_bus_unregister;
    }

    iRet = pdm_submodule_register_drivers();
    if (iRet < 0) {
        pr_err("PDM: Failed to register submodule drivers\n");
        goto err_master_exit;
    }

    pdm_debugfs_root = debugfs_create_dir("pdm", NULL);
    if (IS_ERR(pdm_debugfs_root))
    {
        pr_err("PDM: Failed to create debugfs\n");
    }

    pr_info("PDM: Initialized successfully\n");

    return 0;

err_master_exit:
    pdm_master_exit();
err_bus_unregister:
    bus_unregister(&pdm_bus_type);
err_dev_unregister:
    device_unregister(&pdm_bus_root);

    return iRet;
}

// 模块退出
static void __exit pdm_exit(void)
{
    debugfs_remove_recursive(pdm_debugfs_root);
    pdm_submodule_unregister_drivers();
    pdm_master_exit();
    bus_unregister(&pdm_bus_type);
    device_unregister(&pdm_bus_root);
    pr_info("PDM: Unregistered successfully\n");
    return;
}


module_init(pdm_init);
module_exit(pdm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wanguo");
MODULE_DESCRIPTION("Peripheral Driver Module Driver");
