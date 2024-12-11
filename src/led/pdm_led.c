#include "pdm.h"
#include "pdm_adapter_priv.h"
#include "pdm_led_ioctl.h"
#include "pdm_led_priv.h"

static struct pdm_adapter *led_adapter = NULL;

#if 0
/**
 * @brief 查找指定索引的 PDM 设备
 *
 * @param index 设备索引
 * @return 找到的 PDM 设备指针，未找到返回 NULL
 */
static struct pdm_client *pdm_led_find_client(int index)
{
    struct pdm_client *client;
    int found_dev = 0;

    list_for_each_entry(client, &led_adapter->client_list, entry) {
        if (client->index == index) {
            OSA_INFO("Found target LED device.\n");
            found_dev = 1;
            break;
        }
    }

    if (!found_dev) {
        OSA_ERROR("Cannot find target LED device, index: %d\n", index);
        return NULL;
    }

    return client;
}

/**
 * @brief 设置指定索引的 PDM LED 设备的状态
 *
 * @param args 包含设备索引和状态的结构体
 * @return 成功返回 0，失败返回负错误码
 */
static int pdm_led_set_state(struct pdm_led_ioctl_args *args)
{
    struct pdm_client *client;
    int status;

    mutex_lock(&led_adapter->client_list_mutex_lock);

    client = pdm_led_find_client(args->index);
    if (!client) {
        status = -EINVAL;
        goto err_unlock;
    }

    mutex_unlock(&led_adapter->client_list_mutex_lock);
    return 0;

err_unlock:
    mutex_unlock(&led_adapter->client_list_mutex_lock);
    return status;
}

/**
 * @brief 处理 IOCTL 命令
 *
 * 该函数用于处理来自用户空间的 IOCTL 命令。
 *
 * @param file 文件描述符
 * @param cmd IOCTL 命令
 * @param arg 命令参数
 * @return 成功返回 0，失败返回负错误码
 */
static long pdm_led_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct pdm_led_ioctl_args args;
    int status;

    OSA_DEBUG("ioctl, cmd=0x%02x, arg=0x%02lx\n", cmd, arg);

    memset(&args, 0, sizeof(struct pdm_led_ioctl_args));
    switch (cmd) {
        case PDM_LED_SET_STATE: {
            if (copy_from_user(&args, (struct pdm_led_ioctl_args __user *)arg,
                                        sizeof(struct pdm_led_ioctl_args))) {
                return -EFAULT;
            }
            printk(KERN_INFO "PDM_LED_SET_STATE: index %d, state %d\n", args.index, args.state);
            status = pdm_led_set_state(&args);
            break;
        }
        default:
            return -ENOTTY;
    }

    if (status) {
        OSA_ERROR("pdm_led_ioctl error\n");
    }

    return status;
}

/**
 * @brief 处理写操作
 *
 * 该函数用于处理写操作，从用户空间复制数据并设置 LED 状态。
 *
 * @param filp 文件描述符
 * @param buf 用户空间缓冲区
 * @param count 缓冲区大小
 * @param ppos 文件偏移量
 * @return 写入的字节数，或负错误码
 */
static ssize_t pdm_led_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
    struct pdm_led_ioctl_args args;
    char kernel_buf[5];
    ssize_t bytes_read;

    OSA_INFO("Called pdm_led_write\n");

    if (count > sizeof(kernel_buf) - 1) {
        count = sizeof(kernel_buf) - 1;
    }

    if ((bytes_read = copy_from_user(kernel_buf, buf, count)) != 0) {
        OSA_ERROR("Failed to copy data from user space: %zd\n", bytes_read);
        return -EFAULT;
    }

    if (sscanf(kernel_buf, "%d %d", &args.index, &args.state) != 2) {
        OSA_ERROR("Invalid data: %s\n", kernel_buf);
        return -EINVAL;
    }

    if (pdm_led_set_state(&args)) {
        OSA_ERROR("pdm_led_set_state failed\n");
    }

    return count;
}
#endif

/**
 * @brief LED PDM 设备探测函数
 *
 * 该函数在 PDM 设备被探测到时调用，负责将设备添加到主设备中。
 *
 * @param pdmdev PDM 设备指针
 * @return 成功返回 0，失败返回负错误码
 */
static int pdm_led_device_probe(struct pdm_device *pdmdev)
{
    int status;
    struct pdm_client *client;

    client = pdm_client_alloc(sizeof(void *));
    if (client) {
        OSA_ERROR("LED Client Alloc Failed, status=%d\n", status);
        return -ENOMEM;
    }

    pdmdev->client = client;
    client->pdmdev = pdmdev;
    status = pdm_client_register(led_adapter, client);
    if (status) {
        OSA_ERROR("LED Adapter Add Device Failed, status=%d\n", status);
        return status;
    }

    OSA_DEBUG("LED PDM Device Probed\n");
    return 0;
}

/**
 * @brief LED PDM 设备移除函数
 *
 * 该函数在 PDM 设备被移除时调用，负责将设备从主设备中删除。
 *
 * @param pdmdev PDM 设备指针
 */
static void pdm_led_device_remove(struct pdm_device *pdmdev)
{
    pdm_client_unregister(led_adapter, pdmdev->client);
    OSA_DEBUG("LED PDM Device Removed\n");
}

/**
 * @brief 设备树匹配表
 *
 * 该表定义了支持的设备树兼容属性。
 */
static const struct of_device_id of_pdm_led_match[] = {
    { .compatible = "led,pdm-device-pwm", },
    { .compatible = "led,pdm-device-gpio", },
    {},
};
MODULE_DEVICE_TABLE(of, of_pdm_led_match);

/**
 * @brief LED PDM 驱动结构体
 *
 * 该结构体定义了 LED PDM 驱动的基本信息和操作函数。
 */
static struct pdm_driver pdm_led_driver = {
    .probe = pdm_led_device_probe,
    .remove = pdm_led_device_remove,
    .driver = {
        .name = "pdm-led",
        .of_match_table = of_pdm_led_match,
    },
};

/**
 * @brief 初始化 LED PDM 主设备驱动
 *
 * 该函数用于初始化 LED PDM 主设备驱动，分配和注册主设备及驱动。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_led_driver_init(void)
{
    int status;

    led_adapter = pdm_adapter_alloc(sizeof(void *));
    if (!led_adapter) {
        OSA_ERROR("Failed to allocate pdm_adapter\n");
        return -ENOMEM;
    }

    status = pdm_adapter_register(led_adapter, PDM_LED_NAME);
    if (status) {
        OSA_ERROR("Failed to register LED PDM Adapter, status=%d\n", status);
        goto err_adapter_free;
    }

    status = pdm_bus_register_driver(THIS_MODULE, &pdm_led_driver);
    if (status) {
        OSA_ERROR("Failed to register LED PDM Adapter Driver, status=%d\n", status);
        goto err_adapter_unregister;
    }

    OSA_INFO("LED PDM Adapter Driver Initialized\n");
    return 0;

err_adapter_unregister:
    pdm_adapter_unregister(led_adapter);
err_adapter_free:
    pdm_adapter_free(led_adapter);
    return status;
}

/**
 * @brief 退出 LED PDM 主设备驱动
 *
 * 该函数用于退出 LED PDM 主设备驱动，注销驱动和主设备，释放相关资源。
 */
void pdm_led_driver_exit(void)
{
    pdm_bus_unregister_driver(&pdm_led_driver);
    pdm_adapter_unregister(led_adapter);
    pdm_adapter_free(led_adapter);
    OSA_INFO("LED PDM Adapter Driver Exited\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("LED PDM Adapter Driver");
