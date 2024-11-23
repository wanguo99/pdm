#include <linux/slab.h>

#include "pdm.h"


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
        goto err_invalid;
    }

    if (!strlen(pdmdev->compatible)) {
        OSA_ERROR("pdmdev->compatible is invalid\n");
        goto err_invalid;
    }

    if ((!pdmdev->master) || (!strlen(pdmdev->master->name))) {
        OSA_ERROR("pdmdev->master is invalid\n");
        goto err_invalid;
    }

    return 0;

err_invalid:
    OSA_ERROR("pdmdev is invalid\n");
    return -EINVAL;
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
 * @brief 释放 PDM 设备资源
 *
 * 该函数用于释放 PDM 设备的资源，主要是释放设备结构体本身。
 *
 * @param dev 设备结构体指针
 */
static void pdm_device_release(struct device *dev)
{
    struct pdm_device *pdmdev = NULL;

    if (!dev) {
        OSA_ERROR("dev is null\n");
        return;
    }

    pdmdev = dev_to_pdm_device(dev);
    if (pdm_device_verify(pdmdev)) {
        return;
    }

    kfree(pdmdev);
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
        OSA_ERROR("dev is null\n");
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
struct pdm_device *pdm_device_alloc(unsigned int data_size)
{
    struct pdm_device *pdmdev;
    size_t pdmdev_size = sizeof(struct pdm_device);

    pdmdev = kzalloc(pdmdev_size + data_size, GFP_KERNEL);
    if (!pdmdev)
    {
        OSA_ERROR("Failed to allocate memory for pdm device.\n");
        return NULL;
    }

    device_initialize(&pdmdev->dev);
    pdmdev->dev.type = &pdm_device_type;
    pdmdev->dev.bus = &pdm_bus_type;
    pdmdev->dev.release = pdm_device_release;

    pdm_device_devdata_set(pdmdev, (void *)pdmdev + pdmdev_size);

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
    if (pdm_device_verify(pdmdev)) {
        return;
    }

    put_device(&pdmdev->dev);
}

/**
 * @brief 检查 PDM 设备是否已存在
 *
 * 该函数用于检查总线上是否存在与新设备具有相同 master、id 和 compatible 的设备。
 *
 * @param dev 当前遍历的设备
 * @param data 新设备的指针
 * @return 成功返回 0，如果设备已存在返回 -EBUSY
 */
static int pdm_device_check_exist(struct device *dev, void *data)
{
    struct pdm_device *new_dev = dev_to_pdm_device(dev);
    struct pdm_device *on_bus_dev = data;

    if ((new_dev->master == on_bus_dev->master)
        && (new_dev->id == on_bus_dev->id)
        && strcmp(new_dev->compatible, on_bus_dev->compatible) != 0)
    {
        OSA_ERROR("Device already exist.\n");
        return -EBUSY;
    }
    return 0;
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
    struct pdm_master *master = NULL;
    int status;

    if (pdm_device_verify(pdmdev)){
        return -EINVAL;
    }

    master = pdmdev->master;
    if (!pdm_master_get(master)) {
        OSA_ERROR("pdm_device_register: Failed to get master\n");
        return -EBUSY;
    }

    status = pdm_master_client_id_alloc(master, pdmdev);
    if (status) {
        OSA_ERROR("id_alloc failed, status %d\n", status);
        goto err_put_master;
    }

    status = bus_for_each_dev(&pdm_bus_type, NULL, pdmdev, pdm_device_check_exist);
    if (status) {
        OSA_ERROR("Device %s already exists\n", dev_name(&pdmdev->dev));
        goto err_free_id;
    }

    pdmdev->dev.parent = &master->dev;
    dev_set_name(&pdmdev->dev, "%s-%s-%d", master->name, pdmdev->compatible, pdmdev->id);
    status = device_add(&pdmdev->dev);
    if (status < 0)
    {
        OSA_ERROR("Can't add %s, status %d\n", dev_name(&pdmdev->dev), status);
        goto err_free_id;
    }

    OSA_DEBUG("Device %s registered.\n", dev_name(&pdmdev->dev));
    return 0;

err_free_id:
    pdm_master_client_id_free(master, pdmdev);
err_put_master:
    pdm_master_put(master);
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

    device_unregister(&pdmdev->dev);
    pdm_master_client_id_free(pdmdev->master, pdmdev);
    pdm_master_put(pdmdev->master);
    OSA_DEBUG("Device %s unregistered.\n", dev_name(&pdmdev->dev));
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Device Module");
