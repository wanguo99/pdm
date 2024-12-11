#include "pdm.h"
#include "pdm_device.h"
#include "pdm_component.h"
#include "pdm_device_drivers.h"

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

    if (!pdmdev->dev.parent) {
        OSA_ERROR("parent is invalid\n");
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
    return add_uevent_var(env, "MODALIAS=pdm:%04X", pdmdev->id);
}

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
    return 0;
    // TODO: 后续增加compatible解析
    // return sysfs_emit(buf, "%s\n", pdmdev->physical_info.compatible);
}
static DEVICE_ATTR_RO(compatible);

/**
 * @brief 定义 PDM 设备的属性数组
 *
 * 这个数组包含 PDM 设备的所有属性，每个属性都是一个 `struct attribute` 类型的指针。
 * 属性数组以 NULL 结尾，表示属性列表的结束。
 * 使用 `ATTRIBUTE_GROUPS` 宏将属性数组转换为属性组，以便在设备模型中注册。
 */
static struct attribute *pdm_device_attrs[] = {
    &dev_attr_compatible.attr,
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
    struct pdm_device *pdmdev = dev_to_pdm_device(dev);
    if (pdmdev) {
        OSA_DEBUG("PDM Device Released: %s\n", dev_name(dev));
        kfree(pdmdev);
    }
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
    int status;

    pdmdev = kzalloc(sizeof(struct pdm_device), GFP_KERNEL);
    if (!pdmdev) {
        OSA_ERROR("Failed to allocate memory for pdm device.\n");
        return NULL;
    }

    status = pdm_bus_device_id_alloc(pdmdev);
    if (status) {
        OSA_ERROR("id_alloc failed, status %d\n", status);
        kfree(pdmdev);
    }

    device_initialize(&pdmdev->dev);
    pdmdev->dev.bus = &pdm_bus_type;
    pdmdev->dev.type = &pdm_device_type;
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
        pdm_bus_device_id_free(pdmdev);
    }
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

    if (!pdmdev) {
        OSA_ERROR("pdmdev is null\n");
        return -EINVAL;
    }

    if (pdm_device_verify(pdmdev)) {
        return -EINVAL;
    }

    if (pdm_bus_find_device_by_parent(pdmdev->dev.parent)) {
        OSA_ERROR("Device %s already exists\n", dev_name(&pdmdev->dev));
        goto err_free_id;
    }

    dev_set_name(&pdmdev->dev, "pdmdev-%d", pdmdev->id);
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
 * 该函数用于注销 PDM 设备，包括取消设备注册、释放设备 ID 和释放 adapter 引用。
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
    int status;

    status = pdm_device_drivers_register();
    if (status < 0) {
        OSA_ERROR("Failed to register PDM Device Drivers, error: %d.\n", status);
        return status;
    }

    OSA_DEBUG("Initialize PDM Device OK.\n");
    return 0;
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
    pdm_device_drivers_unregister();
    OSA_DEBUG("PDM Device Exit.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Device Module");
