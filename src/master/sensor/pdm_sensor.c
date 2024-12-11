#include "pdm.h"
#include "pdm_sensor_ioctl.h"
#include "pdm_sensor_priv.h"

static struct pdm_master *sensor_master = NULL;

/**
 * @brief 查找指定索引的 PDM 设备
 *
 * @param index 设备索引
 * @return 找到的 PDM 设备指针，未找到返回 NULL
 */
static struct pdm_device *pdm_master_sensor_find_pdmdev(int index)
{
    struct pdm_device *pdmdev;
    int found_dev = 0;

    list_for_each_entry(pdmdev, &sensor_master->client_list, entry) {
        if (pdmdev->client_index == index) {
            OSA_INFO("Found target Sensor device.\n");
            found_dev = 1;
            break;
        }
    }

    if (!found_dev) {
        OSA_ERROR("Cannot find target Sensor device\n");
        return NULL;
    }

    return pdmdev;
}

/**
 * @brief 获取指定索引的 PDM 设备的电压值
 *
 * @param index 设备索引
 * @param value 存储电压值的指针
 * @return 成功返回 0，失败返回负错误码
 */
static int pdm_master_sensor_get_voltage(int index, int *value)
{
    struct pdm_device_sensor_priv *sensor_priv;
    struct pdm_device *pdmdev;
    int status;

    mutex_lock(&sensor_master->client_list_mutex_lock);

    pdmdev = pdm_master_sensor_find_pdmdev(index);
    if (!pdmdev) {
        status = -EINVAL;
        goto err_unlock;
    }

    sensor_priv = pdm_device_devdata_get(pdmdev);
    if (!sensor_priv || !sensor_priv->ops || !sensor_priv->ops->get_voltage) {
        status = -EINVAL;
        goto err_unlock;
    }

    status = sensor_priv->ops->get_voltage(pdmdev, value);
    if (status) {
        OSA_ERROR("PDM Sensor get_voltage failed.\n");
        goto err_unlock;
    }

    mutex_unlock(&sensor_master->client_list_mutex_lock);
    return 0;

err_unlock:
    mutex_unlock(&sensor_master->client_list_mutex_lock);
    return status;
}

/**
 * @brief 获取指定索引的 PDM 设备的电流值
 *
 * @param index 设备索引
 * @param value 存储电流值的指针
 * @return 成功返回 0，失败返回负错误码
 */
static int pdm_master_sensor_get_current(int index, int *value)
{
    struct pdm_device_sensor_priv *sensor_priv;
    struct pdm_device *pdmdev;
    int status;

    mutex_lock(&sensor_master->client_list_mutex_lock);

    pdmdev = pdm_master_sensor_find_pdmdev(index);
    if (!pdmdev) {
        status = -EINVAL;
        goto err_unlock;
    }

    sensor_priv = pdm_device_devdata_get(pdmdev);
    if (!sensor_priv || !sensor_priv->ops || !sensor_priv->ops->get_current) {
        status = -EINVAL;
        goto err_unlock;
    }

    status = sensor_priv->ops->get_current(pdmdev, value);
    if (status) {
        OSA_ERROR("PDM Sensor get_current failed.\n");
        goto err_unlock;
    }

    mutex_unlock(&sensor_master->client_list_mutex_lock);
    return 0;

err_unlock:
    mutex_unlock(&sensor_master->client_list_mutex_lock);
    return status;
}

/**
 * @brief 处理 IOCTL 命令
 *
 * @param file 文件指针
 * @param cmd 命令
 * @param arg 参数
 * @return 成功返回 0，失败返回负错误码
 */
static long pdm_master_sensor_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int status;
    int value;
    int32_t index;

    OSA_DEBUG("ioctl, cmd=0x%02x, arg=0x%02lx\n", cmd, arg);

    switch (cmd) {
        case PDM_MASTER_SENSOR_GET_VOLTAGE: {
            if (copy_from_user(&index, (int32_t __user *)arg, sizeof(int32_t))) {
                return -EFAULT;
            }

            printk(KERN_INFO "PDM_MASTER_SENSOR GET VOLTAGE with index: %d\n", index);
            status = pdm_master_sensor_get_voltage(index, &value);
            break;
        }

        case PDM_MASTER_SENSOR_GET_CURRENT: {
            if (copy_from_user(&index, (int32_t __user *)arg, sizeof(int32_t))) {
                return -EFAULT;
            }

            printk(KERN_INFO "PDM_MASTER_SENSOR GET CURRENT with index: %d\n", index);
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

/**
 * @brief 处理写操作
 *
 * @param filp 文件指针
 * @param buf 用户空间缓冲区
 * @param count 缓冲区大小
 * @param ppos 文件偏移量
 * @return 成功返回写入的字节数，失败返回负错误码
 */
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
        OSA_ERROR("Failed to copy data from user space: %zd\n", bytes_read);
        return -EFAULT;
    }

    if (sscanf(kernel_buf, "%d", &index) != 1) {
        OSA_ERROR("Invalid data: %s\n", kernel_buf);
        return -EINVAL;
    }

    OSA_INFO("Target Sensor index: %d\n", index);
    if (pdm_master_sensor_get_current(index, &value)) {
        OSA_ERROR("pdm_master_sensor_status_set failed.\n");
    }
    return count;
}

/**
 * @brief 传感器 PDM 设备探测函数
 *
 * 该函数在 PDM 设备被探测到时调用，负责将设备添加到传感器主设备中。
 *
 * @param pdmdev PDM 设备指针
 * @return 成功返回 0，失败返回负错误码
 */
static int pdm_master_sensor_device_probe(struct pdm_device *pdmdev)
{
    int status;

    status = pdm_master_client_add(sensor_master, pdmdev);
    if (status) {
        OSA_ERROR("Sensor Master Add Device Failed, status=%d.\n", status);
        return status;
    }

    status = pdm_device_devdata_alloc(pdmdev, sizeof(struct pdm_device_sensor_priv));
    if (status) {
        OSA_ERROR("Alloc Device Private Data Failed, status=%d.\n", status);
        goto err_client_del;
    }

    OSA_DEBUG("Sensor PDM Device Probed.\n");
    return 0;

err_client_del:
    pdm_master_client_delete(sensor_master, pdmdev);
    OSA_DEBUG("Sensor PDM Device Probe Failed.\n");
    return status;
}

/**
 * @brief 传感器 PDM 设备移除函数
 *
 * 该函数在 PDM 设备被移除时调用，负责将设备从传感器主设备中删除。
 *
 * @param pdmdev PDM 设备指针
 */
static void pdm_master_sensor_device_remove(struct pdm_device *pdmdev)
{
    int status;

    pdm_device_devdata_free(pdmdev);

    status = pdm_master_client_delete(sensor_master, pdmdev);
    if (status) {
        OSA_ERROR("Sensor Master Delete Device Failed, status=%d.\n", status);
        return;
    }

    OSA_DEBUG("Sensor PDM Device Removed.\n");
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
 * @brief 传感器 PDM 驱动结构体
 *
 * 该结构体定义了传感器 PDM 驱动的基本信息和操作函数。
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
 * @brief 初始化传感器 PDM 主设备驱动
 *
 * 该函数用于初始化传感器 PDM 主设备驱动，分配和注册主设备及驱动。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_master_sensor_driver_init(void)
{
    int status;

    sensor_master = pdm_master_alloc(sizeof(struct pdm_master_sensor_priv));
    if (!sensor_master) {
        OSA_ERROR("Failed to allocate pdm_master.\n");
        return -ENOMEM;
    }

    strncpy(sensor_master->name, PDM_MASTER_SENSOR_NAME, strlen(PDM_MASTER_SENSOR_NAME));
    status = pdm_master_register(sensor_master);
    if (status) {
        OSA_ERROR("Failed to register Sensor PDM Master, status=%d.\n", status);
        goto err_master_free;
    }

    status = pdm_bus_register_driver(THIS_MODULE, &pdm_master_sensor_driver);
    if (status) {
        OSA_ERROR("Failed to register Sensor PDM Master Driver, status=%d.\n", status);
        goto err_master_unregister;
    }

    sensor_master->fops.unlocked_ioctl = pdm_master_sensor_ioctl;
    sensor_master->fops.write = pdm_master_sensor_write;

    OSA_INFO("Sensor PDM Master Driver Initialized.\n");
    return 0;

err_master_unregister:
    pdm_master_unregister(sensor_master);
err_master_free:
    pdm_master_free(sensor_master);
    return status;
}

/**
 * @brief 退出传感器 PDM 主设备驱动
 *
 * 该函数用于退出传感器 PDM 主设备驱动，注销驱动和主设备，释放相关资源。
 */
void pdm_master_sensor_driver_exit(void)
{
    pdm_bus_unregister_driver(&pdm_master_sensor_driver);
    pdm_master_unregister(sensor_master);
    pdm_master_free(sensor_master);
    OSA_INFO("Sensor PDM Master Driver Exited.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("Sensor PDM Master Driver");
