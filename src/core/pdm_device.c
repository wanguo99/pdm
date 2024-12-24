#include "pdm.h"
#include "pdm_device.h"
#include "pdm_component.h"
#include "pdm_device_priv.h"

/**
 * @brief List to store all registered PDM device drivers.
 */
static struct list_head pdm_device_driver_list;

/**
 * @brief Array of PDM device drivers.
 *
 * Each `pdm_component` structure contains the driver's name, init and exit functions.
 */
static struct pdm_component pdm_device_drivers[] = {
    {
        .name = "SPI PDM Device",
        .enable = true,
        .ignore_failures = true,
        .init = pdm_device_spi_driver_init,
        .exit = pdm_device_spi_driver_exit
    },
    {
        .name = "I2C PDM Device",
        .enable = true,
        .ignore_failures = true,
        .init = pdm_device_i2c_driver_init,
        .exit = pdm_device_i2c_driver_exit
    },
    {
        .name = "PLATFORM PDM Device",
        .enable = true,
        .ignore_failures = true,
        .init = pdm_device_platform_driver_init,
        .exit = pdm_device_platform_driver_exit
    },
    { }
};


static atomic_t pdm_device_no = ATOMIC_INIT(-1);

/**
 * @brief Validates a PDM device.
 *
 * Checks if the provided PDM device pointer is valid.
 *
 * @param pdmdev Pointer to the PDM device structure.
 * @return 0 on success, -EINVAL on failure.
 */
static int pdm_device_verify(struct pdm_device *pdmdev)
{
    if (!pdmdev || !pdmdev->dev.parent) {
        OSA_ERROR("%s is null\n", !pdmdev ? "pdmdev" : "parent");
        return -EINVAL;
    }
    return 0;
}

/**
 * @brief 生成 PDM 设备的 uevent 事件
 *
 * @param dev 设备结构体指针
 * @param env uevent 环境变量结构体指针
 * @return 成功返回 0，失败返回 -EINVAL
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
static int pdm_device_uevent(struct device *dev, struct kobj_uevent_env *env)
#else
static int pdm_device_uevent(const struct device *dev, struct kobj_uevent_env *env)
#endif
{
    struct pdm_device *pdmdev = dev_to_pdm_device(dev);
    if (pdm_device_verify(pdmdev)) {
        return -EINVAL;
    }
    return add_uevent_var(env, "MODALIAS=%s", dev_name(dev));
}

/**
 * id_show - 显示设备ID
 * @dev: 设备结构
 * @da: 设备属性结构
 * @buf: 输出缓冲区
 *
 * 返回值:
 * 实际写入的字节数
 */
static ssize_t name_show(struct device *dev, struct device_attribute *da, char *buf)
{
    struct pdm_device *pdmdev = dev_to_pdm_device(dev);
    if (pdm_device_verify(pdmdev)) {
        return -EINVAL;
    }

    return sysfs_emit(buf, "%s\n", dev_name(dev));
}
static DEVICE_ATTR_RO(name);

/**
 * @brief 定义 PDM 设备的属性数组
 *
 * 这个数组包含 PDM 设备的所有属性，每个属性都是一个 `struct attribute` 类型的指针。
 * 属性数组以 NULL 结尾，表示属性列表的结束。
 * 使用 `ATTRIBUTE_GROUPS` 宏将属性数组转换为属性组，以便在设备模型中注册。
 */
static struct attribute *pdm_device_attrs[] = {
    &dev_attr_name.attr,
    NULL,
};
ATTRIBUTE_GROUPS(pdm_device);

/**
 * @brief Releases resources associated with a PDM device.
 *
 * Frees the memory allocated for the PDM device structure.
 *
 * @param dev Pointer to the device structure.
 */
static void pdm_device_release(struct device *dev)
{
    struct pdm_device *pdmdev = dev_to_pdm_device(dev);

    OSA_DEBUG("PDM Device Released: %s\n", dev_name(dev));
    kfree(pdmdev);
}

/**
 * pdm_device_type - PDM设备类型
 */
static const struct device_type pdm_device_type = {
    .name   = "pdm_device",
    .groups = pdm_device_groups,
    .release = pdm_device_release,
    .uevent = pdm_device_uevent,
};

/**
 * @brief Registers PDM device drivers.
 *
 * Initializes the PDM device driver list and registers the drivers.
 *
 * @return 0 on success, negative error code on failure.
 */
int pdm_device_drivers_register(void)
{
    int status;
    struct pdm_component_params params = {
        .components = pdm_device_drivers,
        .count = ARRAY_SIZE(pdm_device_drivers),
        .list = &pdm_device_driver_list,
    };

    INIT_LIST_HEAD(&pdm_device_driver_list);
    status = pdm_component_register(&params);
    if (status < 0) {
        OSA_ERROR("Failed to register PDM Device Drivers, error: %d.\n", status);
        return status;
    }

    OSA_DEBUG("PDM Device Drivers initialized successfully.\n");
    return 0;
}

/**
 * @brief Unregisters PDM device drivers.
 *
 * Cleans up and unregisters the PDM device drivers.
 */
void pdm_device_drivers_unregister(void)
{
    pdm_component_unregister(&pdm_device_driver_list);
    OSA_DEBUG("PDM Device Drivers exited successfully.\n");
}


/**
 * @brief Allocates a new PDM device structure.
 *
 * @return Pointer to the allocated PDM device structure, or NULL on failure.
 */
struct pdm_device *pdm_device_alloc(struct device *dev)
{
    struct pdm_device *pdmdev;

    if (!dev) {
        OSA_ERROR("invalid device pointer.\n");
        return ERR_PTR(-ENODEV);
    }

    pdmdev = kzalloc(sizeof(*pdmdev), GFP_KERNEL);
    if (!pdmdev) {
        OSA_ERROR("Failed to allocate memory for PDM device.\n");
        return ERR_PTR(-ENOMEM);
    }

    pdmdev->dev.parent = dev;
    pdmdev->dev.bus = &pdm_bus_type;
    pdmdev->dev.type = &pdm_device_type;
    device_initialize(&pdmdev->dev);

    dev_set_name(&pdmdev->dev, "pdmdev%lu",
            (unsigned long)atomic_inc_return(&pdm_device_no));

    return pdmdev;
}

/**
 * @brief Frees a PDM device structure.
 *
 * Decrements the device reference count; when it reaches zero, the device is freed.
 *
 * @param pdmdev Pointer to the PDM device structure.
 */
void pdm_device_free(struct pdm_device *pdmdev)
{
    if (pdmdev) {
        pdm_device_put(pdmdev);
    }
}

/**
 * @brief Registers a PDM device.
 *
 * Verifies the device, allocates an ID, checks for duplicates, sets the device name,
 * and adds the device to the system.
 *
 * @param pdmdev Pointer to the PDM device structure.
 * @return 0 on success, negative error code on failure.
 */
int pdm_device_register(struct pdm_device *pdmdev)
{
    int status;

    if (pdm_device_verify(pdmdev))
        return -EINVAL;

    if (pdm_bus_find_device_by_parent(pdmdev->dev.parent)) {
        OSA_ERROR("Device with parent %s already exists\n", dev_name(pdmdev->dev.parent));
        return -EEXIST;
    }

    status = device_add(&pdmdev->dev);
    if (status) {
        OSA_ERROR("Failed to add device %s, error: %d\n", dev_name(&pdmdev->dev), status);
        goto err_free_id;
    }

    OSA_DEBUG("Device %s registered successfully.\n", dev_name(&pdmdev->dev));
    return 0;

err_free_id:
    pdm_device_put(pdmdev);
    return status;
}

/**
 * @brief Unregisters a PDM device.
 *
 * Removes the device from the system and frees its ID.
 *
 * @param pdmdev Pointer to the PDM device structure.
 */
void pdm_device_unregister(struct pdm_device *pdmdev)
{
    if (!pdmdev) {
        OSA_ERROR("Invalid input parameters (pdmdev: %p).\n", pdmdev);
        return;
    }

    OSA_DEBUG("Unregistering device %s.\n", dev_name(&pdmdev->dev));
    device_unregister(&pdmdev->dev);
}

/**
 * @brief Initializes the PDM device module.
 *
 * Registers the device class and device drivers.
 *
 * @return 0 on success, negative error code on failure.
 */
int pdm_device_init(void)
{
    int status;

    status = pdm_device_drivers_register();
    if (status < 0) {
        OSA_ERROR("Failed to register PDM Device Drivers, error: %d\n", status);
        return status;
    }

    OSA_INFO("PDM Device module initialized successfully.\n");
    return 0;
}

/**
 * @brief Exits the PDM device module.
 *
 * Unregisters the device drivers and cleans up resources.
 */
void pdm_device_exit(void)
{
    pdm_device_drivers_unregister();
    OSA_INFO("PDM Device module exited successfully.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Device Module");
