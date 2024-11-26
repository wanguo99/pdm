#include "pdm.h"
#include "pdm_device.h"
#include "pdm_driver_manager.h"

/**
 * @brief PDM 主模板驱动程序列表
 *
 * 该列表用于存储所有注册的 PDM 主模板驱动程序。
 */
static struct list_head pdm_device_driver_list;

/**
 * @brief PDM 主模板驱动程序数组
 *
 * 该数组包含所有需要注册的 PDM 主模板驱动程序。每个 `pdm_subdriver` 结构体包含驱动程序的名称、初始化函数和退出函数。
 */
static struct pdm_subdriver pdm_device_drivers[] = {
    {
        .name = "PDM SPI Device",
        .init = pdm_device_spi_driver_init,
        .exit = pdm_device_spi_driver_exit
    },
    {
        .name = "PDM I2C Device",
        .init = pdm_device_i2c_driver_init,
        .exit = pdm_device_i2c_driver_exit
    },
    {
        .name = "PDM PLATFORM Device",
        .init = pdm_device_platform_driver_init,
        .exit = pdm_device_platform_driver_exit
    },
};

/**
 * @brief 验证 PDM 设备的有效性
 *
 * @param pdmdev 要验证的 PDM 设备结构体指针
 * @return 成功返回 0，失败返回 -EINVAL
 */
static int pdm_device_verify(struct pdm_device *pdmdev)
{
    if (!pdmdev) {
        OSA_ERROR("pdmdev is null\n");
        return -EINVAL;
    }

    if (0 == strlen(pdmdev->compatible)) {
        OSA_ERROR("compatible is invalid\n");
        return -EINVAL;
    }

    if ((pdmdev->physical_info.type <= PDM_DEVICE_INTERFACE_TYPE_UNDEFINED)
        || (pdmdev->physical_info.type >= PDM_DEVICE_INTERFACE_TYPE_INVALID)) {
        OSA_ERROR("interface is invalid\n");
        return -EINVAL;
    }

    if (!pdmdev->physical_info.device) {
        OSA_ERROR("physical_device is invalid\n");
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
    struct pdm_device *pdmdev = NULL;

    if (!dev) {
        OSA_ERROR("dev is null\n");
        return -EINVAL;
    }

    pdmdev = dev_to_pdm_device(dev);
    if (pdm_device_verify(pdmdev)) {
        return -EINVAL;
    }

    OSA_DEBUG("Generating MODALIAS for device %s\n", dev_name(dev));

    return add_uevent_var(env, "MODALIAS=pdm:pdm_master_%s:%s-%04X",
                                                pdmdev->master->name,
                                                pdmdev->compatible, pdmdev->id);
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
static ssize_t id_show(struct device *dev, struct device_attribute *da, char *buf)
{
    struct pdm_device *pdmdev = NULL;

    if (!dev) {
        OSA_ERROR("dev is null\n");
        return -EINVAL;
    }

    pdmdev = dev_to_pdm_device(dev);
    if (pdm_device_verify(pdmdev)) {
        return -EINVAL;
    }

    OSA_INFO("Showing ID for device %s\n", dev_name(dev));

    return sysfs_emit(buf, "%d\n", pdmdev->id);
}
static DEVICE_ATTR_RO(id);

/**
 * compatible_show - 显示设备兼容性字符串
 * @dev: 设备结构
 * @da: 设备属性结构
 * @buf: 输出缓冲区
 *
 * 返回值:
 * 实际写入的字节数
 */
static ssize_t compatible_show(struct device *dev, struct device_attribute *da, char *buf)
{
    struct pdm_device *pdmdev = NULL;

    if (!dev) {
        OSA_ERROR("dev is null\n");
        return -EINVAL;
    }

    pdmdev = dev_to_pdm_device(dev);
    if (pdm_device_verify(pdmdev)) {
        return -EINVAL;
    }

    OSA_INFO("Showing compatible string for device %s\n", dev_name(dev));

    return sysfs_emit(buf, "%s\n", pdmdev->compatible);
}
static DEVICE_ATTR_RO(compatible);

/**
 * master_name_show - 显示设备所属主控制器的名称
 * @dev: 设备结构
 * @da: 设备属性结构
 * @buf: 输出缓冲区
 *
 * 返回值:
 * 实际写入的字节数
 */
static ssize_t master_name_show(struct device *dev, struct device_attribute *da, char *buf)
{
    struct pdm_device *pdmdev = NULL;

    if (!dev) {
        OSA_ERROR("dev is null\n");
        return -EINVAL;
    }

    pdmdev = dev_to_pdm_device(dev);
    if (pdm_device_verify(pdmdev)) {
        return -EINVAL;
    }

    OSA_INFO("Showing master name for device %s\n", dev_name(dev));

    return sysfs_emit(buf, "%s\n", pdmdev->master->name);
}
static DEVICE_ATTR_RO(master_name);

/**
 * @brief 定义 PDM 设备的属性数组
 *
 * 这个数组包含 PDM 设备的所有属性，每个属性都是一个 `struct attribute` 类型的指针。
 * 属性数组以 NULL 结尾，表示属性列表的结束。
 * 使用 `ATTRIBUTE_GROUPS` 宏将属性数组转换为属性组，以便在设备模型中注册。
 */
static struct attribute *pdm_device_attrs[] = {
    &dev_attr_id.attr,
    &dev_attr_compatible.attr,
    &dev_attr_master_name.attr,
    NULL,
};
ATTRIBUTE_GROUPS(pdm_device);

/**
 * pdm_device_type - PDM设备类型
 */
const struct device_type pdm_device_type = {
    .name   = "pdm_device",
    .groups = pdm_device_groups,
    .uevent = pdm_device_uevent,
};


/**
 * @brief PDM 设备类
 */
static struct class pdm_device_class = {
    .name = "pdm_device",
};

/**
 * @brief 将 PDM 设备转换为实际的设备指针
 *
 * 该函数根据 PDM 设备的接口类型，返回相应的实际设备指针。
 *
 * @param pdmdev 指向 PDM 设备的指针
 * @return 成功返回实际的设备指针，失败返回 NULL
 */
struct device *pdm_device_to_physical_dev(struct pdm_device *pdmdev)
{
    struct device *dev = NULL;

    if (pdm_device_verify(pdmdev)) {
        return NULL;
    }

    switch (pdmdev->physical_info.type) {
        case PDM_DEVICE_INTERFACE_TYPE_I2C:
            dev = &((struct i2c_client *)pdmdev->physical_info.device)->dev;
            break;
        case PDM_DEVICE_INTERFACE_TYPE_I3C:
            dev = &((struct i3c_device *)pdmdev->physical_info.device)->dev;
            break;
        case PDM_DEVICE_INTERFACE_TYPE_SPI:
            dev = &((struct spi_device *)pdmdev->physical_info.device)->dev;
            break;
        default:
            pr_err("Unsupported interface type %d\n", pdmdev->physical_info.type);
            return NULL;
    }

    return dev;
}

/**
 * @brief 释放 PDM 设备资源
 *
 * 该函数用于释放 PDM 设备的资源，主要是释放设备结构体本身。
 *
 * @param dev 设备结构体指针
 */
static void pdm_device_release(struct device *dev)
{
    struct pdm_device *pdmdev = dev_to_pdm_device(dev);
    if (pdmdev) {
        OSA_DEBUG("PDM device released: %p\n", pdmdev);
        kfree(pdmdev);
    }else {
        OSA_ERROR("pdmdev is null\n");
    }
}

/**
 * @brief 获取 PDM 设备的私有数据
 *
 * 该函数用于获取与 PDM 设备关联的私有数据。
 *
 * @param pdmdev PDM 设备结构体指针
 * @return 成功返回私有数据指针，失败返回 NULL
 */
void *pdm_device_devdata_get(struct pdm_device *pdmdev)
{
    if (!pdmdev) {
        OSA_ERROR("pdmdev is null\n");
        return NULL;
    }

    return dev_get_drvdata(&pdmdev->dev);
}

/**
 * @brief 设置 PDM 设备的私有数据
 *
 * 该函数用于设置与 PDM 设备关联的私有数据。
 *
 * @param pdmdev PDM 设备结构体指针
 * @param data 私有数据指针
 */
void pdm_device_devdata_set(struct pdm_device *pdmdev, void *data)
{
    if (!pdmdev) {
        OSA_ERROR("pdmdev is null\n");
        return;
    }

    dev_set_drvdata(&pdmdev->dev, data);
}

/**
 * @brief 分配 PDM 设备结构体
 *
 * 该函数用于分配一个新的 PDM 设备结构体，并初始化设备的相关字段。
 * 分配的内存大小包括 PDM 设备结构体本身的大小和额外的私有数据区域。
 *
 * @param data_size 私有数据区域的大小
 * @return 成功返回分配的 PDM 设备结构体指针，失败返回 NULL
 */
struct pdm_device *pdm_device_alloc(void)
{
    struct pdm_device *pdmdev;

    pdmdev = kzalloc(sizeof(struct pdm_device), GFP_KERNEL);
    if (!pdmdev) {
        OSA_ERROR("Failed to allocate memory for pdm device.\n");
        return NULL;
    }

    device_initialize(&pdmdev->dev);
    pdmdev->dev.bus = &pdm_bus_type;
    pdmdev->dev.type = &pdm_device_type;
    pdmdev->dev.class = &pdm_device_class;
    pdmdev->dev.release = pdm_device_release;

    return pdmdev;
}

/**
 * @brief 释放 PDM 设备结构体
 *
 * 该函数用于释放 PDM 设备结构体及其相关资源。
 * 通过调用 `put_device` 函数来减少设备引用计数，当引用计数为零时，设备将被自动释放。
 *
 * @param pdmdev PDM 设备结构体指针
 */
void pdm_device_free(struct pdm_device *pdmdev)
{
    if (pdmdev) {
        put_device(&pdmdev->dev);
    }
}

/**
 * @brief 检查设备是否匹配给定的物理信息
 *
 * 该函数用于检查给定的 PDM 设备是否匹配指定的物理信息。
 *
 * @param dev 当前遍历到的设备
 * @param data 传递给回调函数的数据，包含要匹配的物理信息
 * @return 返回遍历结果，1 表示匹配并停止遍历，0 表示不匹配并继续遍历
 */
static int pdm_device_physical_info_check(struct device *dev, void *data) {
    struct pdm_device *pdmdev = dev_to_pdm_device(dev);
    struct pdm_device **checked_pdmdev = (struct pdm_device **)data;

    if (pdm_device_verify(pdmdev)) {
        goto unmatch;
    }

    if (!checked_pdmdev || !*checked_pdmdev) {
        goto unmatch;
    }

    if ((pdmdev->physical_info.type == (*checked_pdmdev)->physical_info.type) &&
        (pdmdev->physical_info.device == (*checked_pdmdev)->physical_info.device)) {
        *checked_pdmdev = pdmdev;
        return 1;
    }

unmatch:
    *checked_pdmdev = NULL;
    return 0;
}

/**
 * @brief 查找与给定物理信息匹配的 PDM 设备
 *
 * 该函数用于查找与给定物理信息匹配的 PDM 设备。
 *
 * @param physical_info 要匹配的物理设备信息
 * @return 返回匹配的设备指针，如果没有找到则返回 NULL
 */
struct pdm_device *pdm_device_match_physical_info(struct pdm_device_physical_info *physical_info)
{
    struct pdm_device *check_pdmdev;        // 用于接收physical_info
    struct pdm_device *target_pdmdev_ptr;   // 用于传入physical_info以及传出匹配到的pdm_device

    if (!physical_info) {
        OSA_ERROR("Invalid physical info.\n");
        return NULL;
    }

    check_pdmdev = kzalloc(sizeof(struct pdm_device), GFP_KERNEL);
    if (!check_pdmdev) {
        OSA_ERROR("Failed to allocate memory for pdm device.\n");
        return NULL;
    }

    check_pdmdev->physical_info.type = physical_info->type;
    check_pdmdev->physical_info.device = physical_info->device;

    target_pdmdev_ptr = check_pdmdev;
    class_for_each_device(&pdm_device_class, NULL, &target_pdmdev_ptr, pdm_device_physical_info_check);

    if (target_pdmdev_ptr == check_pdmdev) {
        target_pdmdev_ptr = NULL;
    }

    kfree(check_pdmdev);
    return target_pdmdev_ptr;
}


/**
 * @brief 注册 PDM 设备
 *
 * 该函数用于注册 PDM 设备，包括验证设备有效性、分配设备 ID、检查设备是否存在、设置设备名称和添加设备。
 *
 * @param pdmdev PDM 设备结构体指针
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_device_register(struct pdm_device *pdmdev)
{
    int status;

    if (pdm_device_verify(pdmdev)) {
        return -EINVAL;
    }

    status = pdm_bus_device_id_alloc(pdmdev);
    if (status) {
        OSA_ERROR("id_alloc failed, status %d\n", status);
        return -ENOMEM;
    }

    if (pdm_device_match_physical_info(&pdmdev->physical_info)) {
        OSA_ERROR("Device %s already exists\n", dev_name(&pdmdev->dev));
        goto err_free_id;
    }

    dev_set_name(&pdmdev->dev, "%s-%d", pdmdev->compatible, pdmdev->id);
    status = device_add(&pdmdev->dev);
    if (status < 0) {
        OSA_ERROR("Can't add %s, status %d\n", dev_name(&pdmdev->dev), status);
        goto err_free_id;
    }

    OSA_DEBUG("Device %s registered.\n", dev_name(&pdmdev->dev));
    return 0;

err_free_id:
    pdm_bus_device_id_free(pdmdev);
    return status;
}

/**
 * @brief 注销 PDM 设备
 *
 * 该函数用于注销 PDM 设备，包括取消设备注册、释放设备 ID 和释放 master 引用。
 *
 * @param pdmdev PDM 设备结构体指针
 */
void pdm_device_unregister(struct pdm_device *pdmdev)
{
    if (!pdmdev) {
        OSA_ERROR("pdmdev is null\n");
        return;
    }

    OSA_DEBUG("Device %s unregistered.\n", dev_name(&pdmdev->dev));

    device_unregister(&pdmdev->dev);
    pdm_bus_device_id_free(pdmdev);
}

/**
 * @brief 初始化 PDM 设备
 *
 * 该函数用于初始化 PDM 设备，包括注册设备类和设备驱动。
 * 它会执行以下操作：
 * - 注册 PDM 设备类
 * - 初始化子驱动列表
 * - 注册 PDM 设备驱动
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_device_init(void)
{
    struct pdm_subdriver_register_params params;
    int status;

    status = class_register(&pdm_device_class);
    if (status < 0) {
        OSA_ERROR("Failed to register PDM Device Class, error: %d.\n", status);
        return status;
    }
    OSA_DEBUG("PDM Device Class registered.\n");

    INIT_LIST_HEAD(&pdm_device_driver_list);

    params.drivers = pdm_device_drivers;
    params.count = ARRAY_SIZE(pdm_device_drivers);
    params.ignore_failures = true;
    params.list = &pdm_device_driver_list;
    status = pdm_subdriver_register(&params);
    if (status < 0) {
        OSA_ERROR("Failed to register PDM Device Drivers, error: %d.\n", status);
        goto err_class_unregister;
    }

    OSA_DEBUG("Initialize PDM Device OK.\n");
    return 0;

err_class_unregister:
    class_unregister(&pdm_device_class);
    return status;
}

/**
 * @brief 卸载 PDM 设备
 *
 * 该函数用于卸载 PDM 设备，包括注销设备驱动和设备类。
 * 它会执行以下操作：
 * - 注销 PDM 设备驱动
 * - 注销 PDM 设备类
 */
void pdm_device_exit(void)
{
    pdm_subdriver_unregister(&pdm_device_driver_list);
    class_unregister(&pdm_device_class);
    OSA_DEBUG("PDM Device Exit.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Device Module");
