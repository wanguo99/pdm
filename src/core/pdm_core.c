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

/**
 * @brief 调试文件系统目录
 *
 * 该指针用于存储 PDM 调试文件系统的目录。
 */
static struct dentry *pdm_debugfs;

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
 * @brief 匹配 PDM 设备 ID
 *
 * 该函数用于匹配 PDM 设备的 ID。
 *
 * @param id PDM 设备 ID 表
 * @param pdmdev PDM 设备指针
 * @return 匹配成功返回相应的 ID，失败返回 NULL
 */
static const struct pdm_device_id *pdm_bus_match_id(const struct pdm_device_id *id, struct pdm_device *pdmdev)
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
    if (pdm_bus_match_id(&pdmdrv->id_table, pdmdev))
        return 1;

    return 0;
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
    if (NULL == dev) {
        return;
    }

    struct pdm_device *pdmdev = dev_to_pdm_device(dev);
    struct pdm_driver *driver = drv_to_pdmdrv(dev->driver);

    if ((NULL == driver) || (NULL == pdmdev) || (NULL == driver->remove)) {
        return;
    }

    driver->remove(pdmdev);
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
    .name = "pdm",
    .match = pdm_bus_device_match,
    .probe = pdm_bus_device_probe,
    .remove = pdm_bus_device_remove,
};

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
    int iRet;

    OSA_DEBUG("Peripheral Driver Module Init.\n");

    iRet = bus_register(&pdm_bus_type);
    if (iRet < 0) {
        OSA_ERROR("Register PDM Bus Failed.\n");
        return iRet;
    }
    OSA_INFO("Register PDM BUS OK.\n");

    iRet = pdm_master_init();
    if (iRet < 0) {
        OSA_ERROR("Initialize PDM Master Failed.\n");
        goto err_bus_unregister;
    }
    OSA_INFO("Initialize PDM Master OK.\n");

    iRet = pdm_submodule_register_drivers();
    if (iRet < 0) {
        OSA_ERROR("Register PDM Submodules Failed.\n");
        goto err_master_exit;
    }
    OSA_INFO("Register PDM Submodules OK.\n");

    pdm_debugfs = debugfs_create_dir("pdm", NULL);
    if (IS_ERR(pdm_debugfs)) {
        OSA_ERROR("Register PDM debugfs Failed.\n");
    } else {
        OSA_DEBUG("Register PDM debugfs OK.\n");
    }

    OSA_INFO("Peripheral Driver Module Init OK.\n");
    return 0;

err_master_exit:
    pdm_master_exit();
err_bus_unregister:
    bus_unregister(&pdm_bus_type);

    return iRet;
}

/**
 * @brief 退出 PDM 模块
 *
 * 该函数用于退出 PDM 模块，包括注销总线、卸载主设备和子模块等。
 */
static void __exit pdm_exit(void)
{
    if (!IS_ERR(pdm_debugfs)) {
        debugfs_remove_recursive(pdm_debugfs);
    }

    pdm_submodule_unregister_drivers();
    pdm_master_exit();
    bus_unregister(&pdm_bus_type);
    OSA_INFO("Peripheral Driver Module Exited.\n");
}

module_init(pdm_init);
module_exit(pdm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wanguo");
MODULE_DESCRIPTION("Peripheral Driver Module Driver");
