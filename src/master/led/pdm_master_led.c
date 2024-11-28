#include "pdm.h"
#include "pdm_master_led.h"
#include "pdm_master_led_priv.h"
#include "pdm_master_led_ioctl.h"

static struct pdm_master *led_master = NULL;

static int pdm_master_led_status_set(int index, int state)
{
    struct pdm_device_led_priv *led_priv;
    struct pdm_device *pdmdev;
    int status;
    int found_dev;

    mutex_lock(&led_master->client_list_mutex_lock);

    found_dev = 0;
    list_for_each_entry(pdmdev, &led_master->client_list, entry) {
        led_priv = pdm_device_devdata_get(pdmdev);
        if ((led_priv) && (led_priv->index == index)) {
            OSA_INFO("Found target Led device.\n");
            found_dev = 1;
            break;
        }
    }
    if (!found_dev) {
        OSA_ERROR("Cannot find target LED device");
        mutex_unlock(&led_master->client_list_mutex_lock);
        return -ENODEV;
    }

    switch (state) {
        case PDM_MASTER_LED_STATE_OFF:
            status = led_priv->ops->turn_off(pdmdev);
            break;
        case PDM_MASTER_LED_STATE_ON:
            status = led_priv->ops->turn_on(pdmdev);
            break;
        default:
            status = -EINVAL;
    }

    if (status) {
        OSA_ERROR("PDM Led Turn ON Failed.\n");
    }
    mutex_unlock(&led_master->client_list_mutex_lock);
    return 0;
}


static long pdm_master_led_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int status;

    OSA_INFO("Called pdm_master_led_ioctl\n");

    dev_dbg(&led_master->dev, "ioctl, cmd=0x%02x, arg=0x%02lx\n", cmd, arg);
    switch (cmd) {
        case PDM_MASTER_LED_TURN_ON: {
            int32_t index;
            if (copy_from_user(&index, (int32_t __user *)arg, sizeof(int32_t))) {
                return -EFAULT;
            }

            printk(KERN_INFO "PDM_MASTER_LED TURN ON with value: %d\n", index);
            status = pdm_master_led_status_set(index, PDM_MASTER_LED_STATE_ON);
            break;
        }

        case PDM_MASTER_LED_TURN_OFF: {
            int32_t index;
            if (copy_from_user(&index, (int32_t __user *)arg, sizeof(int32_t))) {
                return -EFAULT;
            }

            printk(KERN_INFO "PDM_MASTER_LED TURN OFF with value: %d\n", index);
            status = pdm_master_led_status_set(index, PDM_MASTER_LED_STATE_ON);
            break;
        }

        default:
            return -ENOTTY;
    }
    if (status) {
        OSA_ERROR("pdm_master_led_ioctl error.\n");
    }

    return status;
}

static ssize_t pdm_master_led_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
    int index;
    char kernel_buf[5];
    ssize_t bytes_read;

    OSA_INFO("Called pdm_master_led_write.\n");

    if (count > sizeof(kernel_buf) - 1) {
        count = sizeof(kernel_buf) - 1;
    }

    if ((bytes_read = copy_from_user(kernel_buf, buf, count)) != 0) {
        OSA_ERROR("Failed to copy data from user space: %zd\n", bytes_read);
        return -EFAULT;
    }

    OSA_INFO("Received data: %s\n", kernel_buf);
    if (sscanf(kernel_buf, "%d", &index) != 1) {
        OSA_ERROR("Invalid data: %s\n", kernel_buf);
        return -EINVAL;
    }

    OSA_INFO("Target LED index: %d\n", index);
    if (pdm_master_led_status_set(index, PDM_MASTER_LED_STATE_ON)) {
        OSA_ERROR("pdm_master_led_status_set failed.\n");
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
static int pdm_master_led_device_probe(struct pdm_device *pdmdev)
{
    int status;
    struct pdm_device_led_priv *data;

    status = pdm_master_client_add(led_master, pdmdev);
    if (status) {
        OSA_ERROR("LED Master Add Device Failed, status=%d.\n", status);
        return status;
    }

    status = pdm_device_devdata_alloc(pdmdev, sizeof(struct pdm_device_led_priv));
    if (status) {
        OSA_ERROR("Alloc Device Private Data Failed, status=%d.\n", status);
        goto err_client_del;
    }

    switch (pdmdev->physical_info.type) {
        case PDM_DEVICE_INTERFACE_TYPE_GPIO: {
            OSA_INFO("pdmdev->physical_info.type: %d\n", pdmdev->physical_info.type);
            status = pdm_master_led_gpio_setup(pdmdev);
            break;
        }
        case PDM_DEVICE_INTERFACE_TYPE_PWM: {
            OSA_INFO("pdmdev->physical_info.type: %d\n", pdmdev->physical_info.type);
            status = pdm_master_led_pwm_setup(pdmdev);
            break;
        }
        default: {
            OSA_ERROR("Unsupport LED Type: %d\n", pdmdev->physical_info.type);
            status = -ENOTSUPP;
            break;
        }
    }
    if (status) {
        OSA_ERROR("Setup Failed: %d\n", status);
        goto err_devdata_free;
    }

    data = (struct pdm_device_led_priv *)pdm_device_devdata_get(pdmdev);
    if (!data)
    {
        OSA_ERROR("Get Device Private Data Failed, status=%d.\n", status);
        goto err_devdata_free;
    }
    data->index = pdmdev->id;

    OSA_DEBUG("LED PDM Device Probed.\n");

    return 0;

err_devdata_free:
    pdm_device_devdata_free(pdmdev);

err_client_del:
    pdm_master_client_delete(led_master, pdmdev);
    OSA_DEBUG("LED PDM Device Probe Failed.\n");
    return status;
}

/**
 * @brief 模板 PDM 设备移除函数
 *
 * 该函数在 PDM 设备被移除时调用，负责将设备从模板主设备中删除。
 *
 * @param pdmdev PDM 设备指针
 */
static void pdm_master_led_device_remove(struct pdm_device *pdmdev)
{
    int status;

    pdm_device_devdata_free(pdmdev);

    status = pdm_master_client_delete(led_master, pdmdev);
    if (status) {
        OSA_ERROR("LED Master Delete Device Failed, status=%d.\n", status);
        return;
    }

    OSA_DEBUG("LED PDM Device Removed.\n");
}

/**
 * @brief 设备树匹配表
 *
 * 该表定义了支持的设备树兼容属性。
 */
static const struct of_device_id of_pdm_master_led_match[] = {
    { .compatible = "led,pdm-device-pwm", },
    { .compatible = "led,pdm-device-gpio", },
    {},
};
MODULE_DEVICE_TABLE(of, of_pdm_master_led_match);

/**
 * @brief 模板 PDM 驱动结构体
 *
 * 该结构体定义了模板 PDM 驱动的基本信息和操作函数。
 */
static struct pdm_driver pdm_master_led_driver = {
    .probe = pdm_master_led_device_probe,
    .remove = pdm_master_led_device_remove,
    .driver = {
        .name = "pdm-device-led",
        .of_match_table = of_pdm_master_led_match,
    },
};

/**
 * @brief 初始化模板 PDM 主设备驱动
 *
 * 该函数用于初始化模板 PDM 主设备驱动，分配和注册主设备及驱动。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_master_led_driver_init(void)
{
    int status;

    led_master = pdm_master_alloc(sizeof(struct pdm_master_led_priv));
    if (!led_master) {
        OSA_ERROR("Failed to allocate pdm_master.\n");
        return -ENOMEM;
    }

    strncpy(led_master->name, PDM_MASTER_LED_NAME, strlen(PDM_MASTER_LED_NAME));
    status = pdm_master_register(led_master);
    if (status) {
        OSA_ERROR("Failed to register LED PDM Master, status=%d.\n", status);
        goto err_master_free;
    }

    status = pdm_bus_register_driver(THIS_MODULE, &pdm_master_led_driver);
    if (status) {
        OSA_ERROR("Failed to register LED PDM Master Driver, status=%d.\n", status);
        goto err_master_unregister;
    }

    led_master->fops->unlocked_ioctl = pdm_master_led_ioctl;
    led_master->fops->write = pdm_master_led_write;

    OSA_INFO("LED PDM Master Driver Initialized.\n");
    return 0;

err_master_unregister:
    pdm_master_unregister(led_master);
err_master_free:
    pdm_master_free(led_master);
    return status;
}

/**
 * @brief 退出模板 PDM 主设备驱动
 *
 * 该函数用于退出模板 PDM 主设备驱动，注销驱动和主设备，释放相关资源。
 */
void pdm_master_led_driver_exit(void)
{
    pdm_bus_unregister_driver(&pdm_master_led_driver);
    pdm_master_unregister(led_master);
    pdm_master_free(led_master);
    OSA_INFO("LED PDM Master Driver Exited.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("LED PDM Master Driver");
