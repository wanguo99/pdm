#include "pdm.h"
#include "pdm_master_sensor_priv.h"
#include "pdm_master_sensor_ioctl.h"

static struct pdm_master *sensor_master = NULL;

static int pdm_master_sensor_get_voltage(int index, int *value)
{
    struct pdm_device_sensor_priv *sensor_priv;
    struct pdm_device *pdmdev;
    int status;
    int found_dev;

    mutex_lock(&sensor_master->client_list_mutex_lock);

    found_dev = 0;
    list_for_each_entry(pdmdev, &sensor_master->client_list, entry) {
        sensor_priv = pdm_device_devdata_get(pdmdev);
        if ((sensor_priv) && (sensor_priv->index == index)) {
            OSA_INFO("Found target Sensor device.\n");
            found_dev = 1;
            break;
        }
    }
    if (!found_dev) {
        OSA_ERROR("Cannot find target SENSOR device");
        mutex_unlock(&sensor_master->client_list_mutex_lock);
        return -ENODEV;
    }

    sensor_priv = pdm_device_devdata_get(pdmdev);
    if ((!sensor_priv) || (!sensor_priv->ops) || (!sensor_priv->ops->get_current)) {
        mutex_unlock(&sensor_master->client_list_mutex_lock);
        return -EINVAL;
    }
    status = sensor_priv->ops->get_voltage(pdmdev, value);
    if (status) {
        OSA_ERROR("PDM Sensor get_voltage Faisensor.\n");
    }

    mutex_unlock(&sensor_master->client_list_mutex_lock);
    return 0;
}

static int pdm_master_sensor_get_current(int index, int *value)
{
    struct pdm_device_sensor_priv *sensor_priv;
    struct pdm_device *pdmdev;
    int found_dev;
    int status;

    mutex_lock(&sensor_master->client_list_mutex_lock);

    found_dev = 0;
    list_for_each_entry(pdmdev, &sensor_master->client_list, entry) {
        if (pdmdev->client_index == index) {
            OSA_INFO("Found target Sensor device.\n");
            found_dev = 1;
            break;
        }
    }
    if (!found_dev) {
        OSA_ERROR("Cannot find target SENSOR device");
        mutex_unlock(&sensor_master->client_list_mutex_lock);
        return -ENODEV;
    }

    sensor_priv = pdm_device_devdata_get(pdmdev);
    if ((!sensor_priv) || (!sensor_priv->ops) || (!sensor_priv->ops->get_current)) {
        mutex_unlock(&sensor_master->client_list_mutex_lock);
        return -EINVAL;
    }
    status = sensor_priv->ops->get_current(pdmdev, value);
    if (status) {
        OSA_ERROR("PDM Sensor get_current Faisensor.\n");
    }
    mutex_unlock(&sensor_master->client_list_mutex_lock);
    return 0;
}


static long pdm_master_sensor_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int status;
    int value;

    OSA_DEBUG("ioctl, cmd=0x%02x, arg=0x%02lx\n", cmd, arg);
    switch (cmd) {
        case PDM_MASTER_SENSOR_GET_VOLTAGE: {
            int32_t index;
            if (copy_from_user(&index, (int32_t __user *)arg, sizeof(int32_t))) {
                return -EFAULT;
            }

            printk(KERN_INFO "PDM_MASTER_SENSOR TURN ON with value: %d\n", index);
            status = pdm_master_sensor_get_voltage(index, &value);
            break;
        }

        case PDM_MASTER_SENSOR_GET_CURRENT: {
            int32_t index;
            if (copy_from_user(&index, (int32_t __user *)arg, sizeof(int32_t))) {
                return -EFAULT;
            }

            printk(KERN_INFO "PDM_MASTER_SENSOR TURN OFF with value: %d\n", index);
            status = pdm_master_sensor_get_current(index, &value);
            break;
        }

        default:
            return -ENOTTY;
    }
    if (status) {
        OSA_ERROR("pdm_master_sensor_ioctl error.\n");
    }

    return status;
}

static ssize_t pdm_master_sensor_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
    int index;
    char kernel_buf[5];
    ssize_t bytes_read;
    int value;

    OSA_INFO("Called sensor pdm_master_sensor_write.\n");

    if (count > sizeof(kernel_buf) - 1) {
        count = sizeof(kernel_buf) - 1;
    }

    if ((bytes_read = copy_from_user(kernel_buf, buf, count)) != 0) {
        OSA_ERROR("Faisensor to copy data from user space: %zd\n", bytes_read);
        return -EFAULT;
    }

    if (sscanf(kernel_buf, "%d", &index) != 1) {
        OSA_ERROR("Invalid data: %s\n", kernel_buf);
        return -EINVAL;
    }

    OSA_INFO("Target SENSOR index: %d\n", index);
    if (pdm_master_sensor_get_current(index, &value)) {
        OSA_ERROR("pdm_master_sensor_status_set faisensor.\n");
    }
    return count;
}


/**
 * @brief 模板 PDM 设备探测函数
 *
 * 该函数在 PDM 设备被探测到时调用，负责将设备添加到模板主设备中。
 *
 * @param pdmdev PDM 设备指针
 * @return 成功返回 0，失败返回负错误码
 */
static int pdm_master_sensor_device_probe(struct pdm_device *pdmdev)
{
    int status;

    status = pdm_master_client_add(sensor_master, pdmdev);
    if (status) {
        OSA_ERROR("SENSOR Master Add Device Faisensor, status=%d.\n", status);
        return status;
    }

    status = pdm_device_devdata_alloc(pdmdev, sizeof(struct pdm_device_sensor_priv));
    if (status) {
        OSA_ERROR("Alloc Device Private Data Faisensor, status=%d.\n", status);
        goto err_client_del;
    }

    switch (pdmdev->physical_info.type) {
        case PDM_DEVICE_INTERFACE_TYPE_I2C: {
            OSA_DEBUG("pdmdev->physical_info.type: %d\n", pdmdev->physical_info.type);
            status = pdm_master_sensor_i2c_setup(pdmdev);
            break;
        }
        case PDM_DEVICE_INTERFACE_TYPE_ADC: {
            OSA_DEBUG("pdmdev->physical_info.type: %d\n", pdmdev->physical_info.type);
            status = pdm_master_sensor_adc_setup(pdmdev);
            break;
        }
        default: {
            OSA_ERROR("Unsupport SENSOR Type: %d\n", pdmdev->physical_info.type);
            status = -ENOTSUPP;
            break;
        }
    }
    if (status) {
        OSA_ERROR("Setup Faisensor: %d\n", status);
        goto err_devdata_free;
    }

    OSA_DEBUG("SENSOR PDM Device Probed.\n");

    return 0;

err_devdata_free:
    pdm_device_devdata_free(pdmdev);

err_client_del:
    pdm_master_client_delete(sensor_master, pdmdev);
    OSA_DEBUG("SENSOR PDM Device Probe Faisensor.\n");
    return status;
}

/**
 * @brief 模板 PDM 设备移除函数
 *
 * 该函数在 PDM 设备被移除时调用，负责将设备从模板主设备中删除。
 *
 * @param pdmdev PDM 设备指针
 */
static void pdm_master_sensor_device_remove(struct pdm_device *pdmdev)
{
    int status;

    pdm_device_devdata_free(pdmdev);

    status = pdm_master_client_delete(sensor_master, pdmdev);
    if (status) {
        OSA_ERROR("SENSOR Master Delete Device Faisensor, status=%d.\n", status);
        return;
    }

    OSA_DEBUG("SENSOR PDM Device Removed.\n");
}

/**
 * @brief 设备树匹配表
 *
 * 该表定义了支持的设备树兼容属性。
 */
static const struct of_device_id of_pdm_master_sensor_match[] = {
    { .compatible = "sensor,pdm-device-adc", },
    { .compatible = "sensor,pdm-device-i2c", },
    {},
};
MODULE_DEVICE_TABLE(of, of_pdm_master_sensor_match);

/**
 * @brief 模板 PDM 驱动结构体
 *
 * 该结构体定义了模板 PDM 驱动的基本信息和操作函数。
 */
static struct pdm_driver pdm_master_sensor_driver = {
    .probe = pdm_master_sensor_device_probe,
    .remove = pdm_master_sensor_device_remove,
    .driver = {
        .name = "pdm-master-sensor",
        .of_match_table = of_pdm_master_sensor_match,
    },
};

/**
 * @brief 初始化模板 PDM 主设备驱动
 *
 * 该函数用于初始化模板 PDM 主设备驱动，分配和注册主设备及驱动。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_master_sensor_driver_init(void)
{
    int status;

    sensor_master = pdm_master_alloc(sizeof(struct pdm_master_sensor_priv));
    if (!sensor_master) {
        OSA_ERROR("Faisensor to allocate pdm_master.\n");
        return -ENOMEM;
    }

    strncpy(sensor_master->name, PDM_MASTER_SENSOR_NAME, strlen(PDM_MASTER_SENSOR_NAME));
    status = pdm_master_register(sensor_master);
    if (status) {
        OSA_ERROR("Faisensor to register SENSOR PDM Master, status=%d.\n", status);
        goto err_master_free;
    }

    status = pdm_bus_register_driver(THIS_MODULE, &pdm_master_sensor_driver);
    if (status) {
        OSA_ERROR("Faisensor to register SENSOR PDM Master Driver, status=%d.\n", status);
        goto err_master_unregister;
    }

    sensor_master->fops.unlocked_ioctl = pdm_master_sensor_ioctl;
    sensor_master->fops.write = pdm_master_sensor_write;

    OSA_INFO("SENSOR PDM Master Driver Initialized.\n");
    return 0;

err_master_unregister:
    pdm_master_unregister(sensor_master);
err_master_free:
    pdm_master_free(sensor_master);
    return status;
}

/**
 * @brief 退出模板 PDM 主设备驱动
 *
 * 该函数用于退出模板 PDM 主设备驱动，注销驱动和主设备，释放相关资源。
 */
void pdm_master_sensor_driver_exit(void)
{
    pdm_bus_unregister_driver(&pdm_master_sensor_driver);
    pdm_master_unregister(sensor_master);
    pdm_master_free(sensor_master);
    OSA_INFO("SENSOR PDM Master Driver Exited.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("SENSOR PDM Master Driver");
