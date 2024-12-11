#include <linux/debugfs.h>
#include <linux/proc_fs.h>
#include <linux/of_device.h>

#include "pdm.h"

static struct pdm_bus_private_data pdm_bus_priv_data;

static int pdm_bus_device_match_parent(struct device *dev, const void *data)
{
    struct pdm_device *pdmdev = dev_to_pdm_device(dev);
    struct device *parent = (struct device *)data;
    return (pdmdev->dev.parent == parent) ? 1 : 0;
}

/**
 * @brief 根据of_node值在pdm_bus_type 总线上查找设备
 *
 * 该函数遍历 `pdm_bus_type` 总线上的所有设备，找到of_node值匹配的设备。
 *
 * @param data 传递给回调函数的数据
 * @param fn 回调函数指针，用于处理每个设备
 * @return 返回遍历结果，0 表示成功，非零值表示失败
 */
struct pdm_device *pdm_bus_find_device_by_parent(struct device *parent)
{
    struct device *dev = bus_find_device(&pdm_bus_type, NULL, parent, pdm_bus_device_match_parent);
    if (!dev) {
        return NULL;
    }
    return dev_to_pdm_device(dev);
}


/**
 * @brief 为PDM设备分配ID
 * @pdmdev: PDM Device
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

    mutex_lock(&pdm_bus_priv_data.idr_mutex_lock);
    id = idr_alloc(&pdm_bus_priv_data.device_idr, pdmdev, PDM_BUS_DEVICE_IDR_START, PDM_BUS_DEVICE_IDR_END, GFP_KERNEL);
    mutex_unlock(&pdm_bus_priv_data.idr_mutex_lock);
    if (id < 0) {
        if (id == -ENOSPC) {
            OSA_ERROR("No available IDs in the range.\n");
            return -EBUSY;
        } else {
            OSA_ERROR("Failed to allocate ID: %d.\n", id);
            return id;
        }
    }

    pdmdev->device_id = id;
    return 0;
}

/**
 * @brief 释放PDM设备的ID
 * @pdmdev: PDM Device
 */
void pdm_bus_device_id_free(struct pdm_device *pdmdev)
{
    if (!pdmdev) {
        OSA_ERROR("Invalid input parameters (pdmdev: %p).\n", pdmdev);
        return;
    }

    mutex_lock(&pdm_bus_priv_data.idr_mutex_lock);
    idr_remove(&pdm_bus_priv_data.device_idr, pdmdev->device_id);
    mutex_unlock(&pdm_bus_priv_data.idr_mutex_lock);
}

/**
 * @brief 注册PDM驱动程序
 * @owner: 模块所有者
 * @driver: PDM驱动程序
 *
 * 返回值:
 * 0 - 成功
 * 非零值 - 失败
 */
int pdm_bus_register_driver(struct module *owner, struct pdm_driver *driver)
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

/**
 * @brief 注销PDM驱动程序
 * @driver: PDM驱动程序
 */
void pdm_bus_unregister_driver(struct pdm_driver *driver)
{
    if (driver)
        driver_unregister(&driver->driver);
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
    struct pdm_device *pdmdev;
    struct pdm_driver *pdmdrv;

    if (!dev) {
        OSA_WARN("Device pointer is NULL\n");
        return -EINVAL;
    }

    pdmdev = dev_to_pdm_device(dev);
    pdmdrv = drv_to_pdm_driver(dev->driver);
    if (pdmdev && pdmdrv && pdmdrv->probe) {
        return pdmdrv->probe(pdmdev);
    }

    OSA_WARN("Driver or device not found or probe function not available\n");
    return -ENODEV;
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
    struct pdm_device *pdmdev;
    struct pdm_driver *pdmdrv;

    if (!dev) {
        OSA_WARN("Device pointer is NULL\n");
        return;
    }

    pdmdev = dev_to_pdm_device(dev);
    pdmdrv = drv_to_pdm_driver(dev->driver);
    if (pdmdev && pdmdrv && pdmdrv->remove) {
        pdmdrv->remove(pdmdev);
    }
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
static int pdm_bus_device_real_match(struct device *dev, const struct device_driver *drv) {

    if (dev->type != &pdm_device_type) {
        return 0;
    }

    /* Attempt an OF style match */
    if (of_driver_match_device(dev, drv)) {
        return 1;
    }

    return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 10, 0)
static int pdm_bus_device_match(struct device *dev, struct device_driver *drv) {
    return pdm_bus_device_real_match(dev, (const struct device_driver *)drv);
}
#else
static int pdm_bus_device_match(struct device *dev, const struct device_driver *drv) {
    return pdm_bus_device_real_match(dev, drv);
}
#endif

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
    .probe  = pdm_bus_device_probe,
    .remove = pdm_bus_device_remove,
    .match  = pdm_bus_device_match,
};

/**
 * @brief 调试文件系统目录
 *
 * 该指针用于存储 PDM 调试文件系统的目录。
 */
static struct dentry *pdm_debugfs_dir;
static struct proc_dir_entry *pdm_procfs_dir;

/**
 * @brief 初始化 PDM 调试文件系统
 *
 * 该函数用于在 debugfs 和 procfs 中创建 PDM 相关的目录。
 */
static int pdm_bus_debug_fs_init(void)
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
static void pdm_bus_debug_fs_exit(void)
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
 * 它会执行以下操作：
 * - 注册 PDM 总线类型
 * - 初始化 PDM 总线实例的相关数据结构
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
    OSA_DEBUG("PDM bus registered\n");

    memset(&pdm_bus_priv_data, 0, sizeof(struct pdm_bus_private_data));
    mutex_init(&pdm_bus_priv_data.idr_mutex_lock);
    idr_init(&pdm_bus_priv_data.device_idr);
    pdm_bus_debug_fs_init();

    OSA_DEBUG("PDM bus initialized\n");
    return 0;
}

/**
 * @brief 卸载 PDM 总线
 *
 * 该函数用于注销 PDM 总线类型，使其不再在内核中使用。
 * 它会执行以下操作：
 * - 注销 PDM 总线类型
 *
 * @note 在调用此函数之前，请确保所有相关的设备已经注销。
 */
static void pdm_bus_exit(void)
{
    pdm_bus_debug_fs_exit();

    mutex_lock(&pdm_bus_priv_data.idr_mutex_lock);
    idr_destroy(&pdm_bus_priv_data.device_idr);
    mutex_unlock(&pdm_bus_priv_data.idr_mutex_lock);

    bus_unregister(&pdm_bus_type);
    OSA_DEBUG("PDM bus unregistered\n");
}

/*                                                                              */
/*                              module_init                                     */
/*                                                                              */

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

    OSA_INFO("===== PDM Init =====\n");

    status = pdm_bus_init();
    if (status < 0) {
        OSA_ERROR("Failed to initialize PDM bus, error %d\n", status);
        return status;
    }

    status = pdm_device_init();
    if (status < 0) {
        OSA_ERROR("Failed to initialize PDM Device, error %d\n", status);
        goto err_bus_exit;
    }

    status = pdm_adapter_init();
    if (status < 0) {
        OSA_ERROR("Failed to initialize PDM Adapter, error %d\n", status);
        goto err_device_exit;
    }

    OSA_INFO("----- PDM Inited ----- \n");
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
    OSA_INFO("===== PDM Exit =====\n");
    pdm_adapter_exit();
    pdm_device_exit();
    pdm_bus_exit();
    OSA_INFO("----- PDM Exited -----\n");
}

module_init(pdm_init);
module_exit(pdm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM <Peripheral Driver Module>");
