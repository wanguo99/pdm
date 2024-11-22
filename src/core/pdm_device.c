#include <linux/slab.h>

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

    // 生成 MODALIAS 字符串, 例如：[pdm:pdm_master_template:template_i2c:0001]
    return add_uevent_var(env, "MODALIAS=pdm:pdm_master_%s:%s-%04X", master_name, compatible, id);
}

static ssize_t id_show(struct device *dev, struct device_attribute *da, char *buf)
{
    const struct pdm_device *pdmdev = dev_to_pdmdev(dev);
    return sysfs_emit(buf, "%d\n", pdmdev->id);
}
static DEVICE_ATTR_RO(id);

static ssize_t compatible_show(struct device *dev, struct device_attribute *da, char *buf)
{
    const struct pdm_device *pdmdev = dev_to_pdmdev(dev);
    return sysfs_emit(buf, "%s\n", pdmdev->compatible);
}
static DEVICE_ATTR_RO(compatible);

static ssize_t master_name_show(struct device *dev, struct device_attribute *da, char *buf)
{
    const struct pdm_device *pdmdev = dev_to_pdmdev(dev);
    const char *master_name = pdmdev->master ? pdmdev->master->name : "unknown";
    return sysfs_emit(buf, "%s\n", master_name);
}
static DEVICE_ATTR_RO(master_name);

// 属性组定义
static struct attribute *pdm_device_attrs[] = {
    &dev_attr_id.attr,
    &dev_attr_compatible.attr,
    &dev_attr_master_name.attr,
    NULL,
};

// ATTRIBUTE_GROUPS将pdm_device(_addr)注册到pdm_deivce(_groups)和pdm_deivce(_groups)
ATTRIBUTE_GROUPS(pdm_device);

const struct device_type pdm_device_type = {
    .name = "pdm_device",
    .groups = pdm_device_groups,
    .uevent = pdm_device_uevent,
};

static void pdm_device_release(struct device *dev)
{
    struct pdm_device *pdmdev = dev_to_pdmdev(dev);
    kfree(pdmdev);
}

void *pdm_device_get_devdata(struct pdm_device *pdmdev)
{
    return dev_get_drvdata(&pdmdev->dev);
}

void pdm_device_set_devdata(struct pdm_device *pdmdev, void *data)
{
    dev_set_drvdata(&pdmdev->dev, data);
}

struct pdm_device *pdm_device_alloc(unsigned int data_size)
{
    struct pdm_device *pdmdev;
    size_t pdmdev_size = sizeof(struct pdm_device);

    pdmdev = kzalloc(pdmdev_size + data_size, GFP_KERNEL);
    if (!pdmdev)
        return NULL;

    device_initialize(&pdmdev->dev);
    pdmdev->dev.type = &pdm_device_type;
    pdmdev->dev.bus = &pdm_bus_type;
    pdmdev->dev.release = pdm_device_release;

    pdm_device_set_devdata(pdmdev, (void *)pdmdev + pdmdev_size);

    return pdmdev;
}

void pdm_device_free(struct pdm_device *pdmdev)
{
    if (!pdmdev)
        return;

    put_device(&pdmdev->dev);
}


static int pdm_device_check(struct device *dev, void *data)
{
    struct pdm_device *new_dev = dev_to_pdmdev(dev);
    struct pdm_device *on_bus_dev = data;

    if ((new_dev->master == on_bus_dev->master)
        && (new_dev->id == on_bus_dev->id)
        && strcmp(new_dev->compatible, on_bus_dev->compatible) != 0) // Use strcmp for string comparison
    {
        // 根据device master、device id、device compatible唯一确定一个pdm_device
        return -EBUSY;
    }
    return 0;
}

int pdm_device_register(struct pdm_device *pdmdev)
{
    struct pdm_master *master = pdmdev->master;
    int status;

    if (!master)
    {
        OSA_ERROR("pdm_device_register: master is NULL\n");
        return -EINVAL;
    }

    if (!pdm_master_get(master)) {
        pr_err("pdm_device_register: Failed to get master\n");
        return -EBUSY;
    }

    status = pdm_master_id_alloc(master, pdmdev);
    if (status) {
        OSA_ERROR("id_alloc failed, status %d\n", status);
        goto err_put_master;
    }

    status = bus_for_each_dev(&pdm_bus_type, NULL, pdmdev, pdm_device_check);
    if (status) {
        OSA_ERROR("Device %s already exists\n", dev_name(&pdmdev->dev));
        goto err_free_id;
    }

    pdmdev->dev.parent = &master->dev;
    dev_set_name(&pdmdev->dev, "pdm_device_%s-%d.0", master->name, pdmdev->id);
    status = device_add(&pdmdev->dev);
    if (status < 0)
    {
        OSA_ERROR("Can't add %s, status %d\n", dev_name(&pdmdev->dev), status);
        goto err_free_id;
    }

    OSA_INFO("Device %s registered.\n", dev_name(&pdmdev->dev));
    return 0;

err_free_id:
    pdm_master_id_free(master, pdmdev);
err_put_master:
    pdm_master_put(master);
    return status;
}

void pdm_device_unregister(struct pdm_device *pdmdev)
{
    if (!pdmdev) {
        pr_err("pdm_device_unregister: pdmdev is NULL\n");
        return;
    }

    device_unregister(&pdmdev->dev);
    pdm_master_id_free(pdmdev->master, pdmdev);
    pdm_master_put(pdmdev->master);
    OSA_INFO("Device %s unregistered.\n", dev_name(&pdmdev->dev));
}
