#include <linux/debugfs.h>
#include <linux/proc_fs.h>
#include <linux/of_device.h>

#include "pdm.h"


/**
 * @brief 调试文件系统目录
 *
 * 该指针用于存储 PDM 调试文件系统的目录。
 */
static struct dentry *pdm_debugfs_dir;
static struct proc_dir_entry *pdm_procfs_dir;

/*                                                                              */
/*                            pdm_bus_type                                      */
/*                                                                              */

/**
 * @brief 将 device_driver 转换为 pdm_driver
 *
 * 该函数用于将 device_driver 转换为 pdm_driver。
 *
 * @param drv device_driver 结构体指针
 * @return pdm_driver 结构体指针
 */
static inline struct pdm_driver *drv_to_pdmdrv(struct device_driver *drv)
{
    return container_of(drv, struct pdm_driver, driver);
}

/**
 * @brief 匹配 PDM 设备和驱动
 *
 * 该函数用于匹配 PDM 设备和驱动。
 *
 * @param dev 设备指针
 * @param drv 驱动指针
 * @return 匹配成功返回 1，失败返回 0
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 10, 0)
static int pdm_bus_device_match(struct device *dev, struct device_driver *drv) {
#else
static int pdm_bus_device_match(struct device *dev, const struct device_driver *drv) {
#endif
    struct pdm_device *pdmdev;
    struct pdm_driver *pdmdrv;

    if (dev->type != &pdm_device_type)
        return 0;

    pdmdev = dev_to_pdm_device(dev);
    pdmdrv = drv_to_pdmdrv(drv);

    /* Attempt an OF style match */
    if (of_driver_match_device(pdm_device_to_physical_dev(pdmdev), drv))
        return 1;

    return 0;
}


/**
 * @brief 为PDM设备分配ID
 * @master: PDM主控制器
 * @pdmdev: PDM设备
 *
 * 返回值:
 * 0 - 成功
 * -EINVAL - 参数无效
 * -EBUSY - 没有可用的ID
 * 其他负值 - 其他错误码
 */
int pdm_bus_device_id_alloc(struct pdm_device *pdmdev)
{
    int id;

    if (!pdmdev) {
        OSA_ERROR("Invalid input parameters (pdmdev: %p).\n", pdmdev);
        return -EINVAL;
    }

    mutex_lock(&pdm_bus_instance.idr_mutex_lock);
    id = idr_alloc(&pdm_bus_instance.device_idr, pdmdev, PDM_BUS_DEVICE_IDR_START, PDM_BUS_DEVICE_IDR_END, GFP_KERNEL);
    mutex_unlock(&pdm_bus_instance.idr_mutex_lock);
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
 * @brief 释放PDM设备的ID
 * @master: PDM主控制器
 * @pdmdev: PDM设备
 */
void pdm_bus_device_id_free(struct pdm_device *pdmdev)
{
    if (!pdmdev) {
        OSA_ERROR("Invalid input parameters (pdmdev: %p).\n", pdmdev);
        return;
    }

    mutex_lock(&pdm_bus_instance.idr_mutex_lock);
    idr_remove(&pdm_bus_instance.device_idr, pdmdev->id);
    mutex_unlock(&pdm_bus_instance.idr_mutex_lock);
}


/**
 * @brief 探测 PDM 设备
 *
 * 该函数用于处理 PDM 设备的探测操作。
 *
 * @param dev 设备指针
 * @return 成功返回 0，失败返回负错误码
 */
static int pdm_bus_device_probe(struct device *dev)
{
    struct pdm_device *pdmdev = dev_to_pdm_device(dev);
    struct pdm_driver *driver = drv_to_pdmdrv(dev->driver);

    if (!driver || !driver->probe) {
        OSA_ERROR("Driver probe function is not defined\n");
        return -EINVAL;
    }

    return driver->probe(pdmdev);
}

/**
 * @brief 移除 PDM 设备
 *
 * 该函数用于处理 PDM 设备的移除操作。
 *
 * @param dev 设备指针
 */
static void pdm_bus_device_remove(struct device *dev)
{
#if 1
    return;
#else
    if (!dev) {
        OSA_WARN("Device pointer is NULL\n");
        return;
    }

    struct pdm_device *pdmdev = dev_to_pdm_device(dev);
    struct pdm_driver *driver = drv_to_pdmdrv(dev->driver);

    if (driver && pdmdev && driver->remove) {
        driver->remove(pdmdev);
    }
#endif
}

/**
 * @brief 遍历 pdm_bus_type 总线上的所有设备
 *
 * 该函数用于遍历 `pdm_bus_type` 总线上的所有设备，并对每个设备调用指定的回调函数。
 *
 * @param data 传递给回调函数的数据
 * @param fn 回调函数指针，用于处理每个设备
 * @return 返回遍历结果，0 表示成功，非零值表示失败
 */
int pdm_bus_for_each_dev(void *data, int (*fn)(struct device *dev, void *data))
{
    return bus_for_each_dev(&pdm_bus_type, NULL, data, fn);
}


/**
 * @brief PDM 总线类型
 *
 * 该结构体定义了 PDM 总线的基本信息和操作函数。
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 2, 0)
struct bus_type pdm_bus_type = {
#else
const struct bus_type pdm_bus_type = {
#endif
    .name   = "pdm",
    .match  = pdm_bus_device_match,
    .probe  = pdm_bus_device_probe,
    .remove = pdm_bus_device_remove,
};

int pdm_register_driver(struct module *owner, struct pdm_driver *driver)
{
    int status;

    driver->driver.owner = owner;
    driver->driver.bus = &pdm_bus_type;
    status = driver_register(&driver->driver);
    if (status) {
        OSA_ERROR("Failed to register driver [%s], error %d\n", driver->driver.name, status);
        return status;
    }

    OSA_DEBUG("Driver [%s] registered\n", driver->driver.name);
    return 0;
}

void pdm_unregister_driver(struct pdm_driver *driver)
{
    if (driver)
        driver_unregister(&driver->driver);
}

/**
 * @brief 初始化 PDM 调试文件系统
 *
 * 该函数用于在 debugfs 和 procfs 中创建 PDM 相关的目录。
 */
static int pdm_debug_fs_init(void)
{
    pdm_debugfs_dir = debugfs_create_dir(PDM_DEBUG_FS_DIR_NAME, NULL);
    if (IS_ERR(pdm_debugfs_dir)) {
        OSA_WARN("Failed to register PDM debugfs, error %ld\n", PTR_ERR(pdm_debugfs_dir));
        pdm_debugfs_dir = NULL;  // Set to NULL to indicate failure
    } else {
        OSA_DEBUG("PDM debugfs registered\n");
    }

    pdm_procfs_dir = proc_mkdir(PDM_DEBUG_FS_DIR_NAME, NULL);
    if (!pdm_procfs_dir) {
        OSA_WARN("Failed to register PDM procfs\n");
        if (pdm_debugfs_dir) {
            debugfs_remove_recursive(pdm_debugfs_dir);
            pdm_debugfs_dir = NULL;
        }
    } else {
        OSA_DEBUG("PDM procfs registered\n");
    }

    return 0;
}

/**
 * @brief 卸载 PDM 调试文件系统
 *
 * 该函数用于删除在 debugfs 和 procfs 中创建的 PDM 相关目录。
 */
static void pdm_debug_fs_exit(void)
{
    if (pdm_debugfs_dir) {
        debugfs_remove_recursive(pdm_debugfs_dir);
        OSA_DEBUG("PDM debugfs unregistered\n");
    }
    if (pdm_procfs_dir) {
        remove_proc_entry(PDM_DEBUG_FS_DIR_NAME, NULL);
        OSA_DEBUG("PDM procfs unregistered\n");
    }
}

/**
 * @brief 初始化 PDM 总线
 *
 * 该函数用于注册 PDM 总线类型，使其可以在内核中使用。
 *
 * @return 成功返回 0，失败返回负错误码
 */
static int pdm_bus_init(void)
{
    int status;
    status = bus_register(&pdm_bus_type);
    if (status < 0) {
        OSA_ERROR("Failed to register PDM bus, error %d\n", status);
        return status;
    }

    memset(&pdm_bus_instance, 0, sizeof(struct pdm_bus));

    idr_init(&pdm_bus_instance.device_idr);
    mutex_init(&pdm_bus_instance.idr_mutex_lock);

    OSA_DEBUG("PDM bus initialized\n");
    return 0;
}

/**
 * @brief 卸载 PDM 总线
 *
 * 该函数用于注销 PDM 总线类型，使其不再在内核中使用。
 */
static void pdm_bus_exit(void)
{
    bus_unregister(&pdm_bus_type);
    OSA_DEBUG("PDM bus unregistered\n");
}

/*                                                                              */
/*                              module_init                                     */
/*                                                                              */
struct pdm_bus pdm_bus_instance;

/**
 * @brief 初始化 PDM 模块
 *
 * 该函数用于初始化 PDM 模块，包括注册总线、初始化主设备和子模块等。
 *
 * @return 成功返回 0，失败返回负错误码
 */
static int __init pdm_init(void)
{
    int status;

    OSA_INFO("PDM Init\n");

    status = pdm_bus_init();
    if (status < 0) {
        OSA_ERROR("Failed to initialize PDM bus, error %d\n", status);
        return status;
    }

    status = pdm_device_init();
    if (status < 0) {
        OSA_ERROR("Failed to initialize PDM device, error %d\n", status);
        goto err_bus_exit;
    }

    status = pdm_master_init();
    if (status < 0) {
        OSA_ERROR("Failed to initialize PDM master, error %d\n", status);
        goto err_device_exit;
    }

    pdm_debug_fs_init();

    OSA_DEBUG("PDM initialized successfully\n");
    return 0;

err_device_exit:
    pdm_device_exit();
err_bus_exit:
    pdm_bus_exit();

    return status;
}

/**
 * @brief 退出 PDM 模块
 *
 * 该函数用于退出 PDM 模块，包括注销总线、卸载主设备和子模块等。
 */
static void __exit pdm_exit(void)
{
    pdm_debug_fs_exit();
    pdm_master_exit();
    pdm_device_exit();
    pdm_bus_exit();
    OSA_INFO("PDM exited\n");
}

module_init(pdm_init);
module_exit(pdm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM <Peripheral Driver Module>");
