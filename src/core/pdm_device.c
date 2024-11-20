#include "pdm.h"

/*                                                                              */
/*                         pdm_device_type                                      */
/*                                                                              */
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
static int pdm_device_uevent(struct device *dev, struct kobj_uevent_env *env)
#else
static int pdm_device_uevent(const struct device *dev, struct kobj_uevent_env *env)
#endif
{
    const struct pdm_device *pdmdev = dev_to_pdmdev(dev);
    const char *compatible = pdmdev->compatible;
    int id = pdmdev->id;
    const char *master_name = pdmdev->master ? pdmdev->master->name : "unknown";

    // 生成 MODALIAS 字符串, 例如：[pdm:pdm_master_cpld:cpld_i2c:0001]
    return add_uevent_var(env, "MODALIAS=pdm:pdm_master_%s:%s-%04X", master_name, compatible, id);
}

static ssize_t id_show(struct device *dev, struct device_attribute *da, char *buf)
{
    const struct pdm_device *pdmdev = dev_to_pdmdev(dev);
    return sprintf(buf, "%d\n", pdmdev->id);
}
static DEVICE_ATTR_RO(id);

static ssize_t compatible_show(struct device *dev, struct device_attribute *da, char *buf)
{
    const struct pdm_device *pdmdev = dev_to_pdmdev(dev);
    return sprintf(buf, "%s\n", pdmdev->compatible);
}
static DEVICE_ATTR_RO(compatible);

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
    &dev_attr_compatible.attr,
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

