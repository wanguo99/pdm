#include <linux/platform_device.h>
#include "pdm.h"

struct pdm_device_platform_data {
    PDM_DEVICE_INTERFACE_TYPE type;
};

static struct pdm_device_platform_data pdm_device_platform_data_plat = {
    .type = PDM_DEVICE_INTERFACE_TYPE_PLATFORM,
};
static struct pdm_device_platform_data pdm_device_platform_data_gpio = {
    .type = PDM_DEVICE_INTERFACE_TYPE_GPIO,
};
static struct pdm_device_platform_data pdm_device_platform_data_pwm = {
    .type = PDM_DEVICE_INTERFACE_TYPE_PWM,
};
static struct pdm_device_platform_data pdm_device_platform_data_tty = {
    .type = PDM_DEVICE_INTERFACE_TYPE_TTY,
};

/**
 * @brief PLATFORM 设备探测函数
 *
 * 该函数在 PLATFORM 设备被探测到时调用，负责初始化和注册 PDM 设备。
 *
 * @param pdev 指向 PLATFORM 设备的指针
 * @return 成功返回 0，失败返回负错误码
 */
static int pdm_device_platform_probe(struct platform_device *pdev) {
    struct pdm_device_physical_info physical_info;
    struct pdm_device *pdmdev;
    int status;
    struct pdm_device_platform_data *pdata = dev_get_platdata(&pdev->dev);

    pdmdev = pdm_device_alloc();
    if (!pdmdev) {
        OSA_ERROR("Failed to allocate pdm_device.\n");
        return -ENOMEM;
    }

    physical_info.type = pdata ? pdata->type : PDM_DEVICE_INTERFACE_TYPE_UNDEFINED;
    physical_info.device = pdev;
    physical_info.of_node = pdev->dev.of_node;
    status = pdm_device_physical_info_set(pdmdev, &physical_info);
    if (status) {
        OSA_ERROR("Failed to set pdm device physical info, status=%d.\n", status);
        goto free_pdmdev;
    }

    status = pdm_device_register(pdmdev);
    if (status) {
        OSA_ERROR("Failed to register pdm device, status=%d.\n", status);
        goto free_pdmdev;
    }

    OSA_DEBUG("PDM Device PLATFORM Device Probed.\n");
    return 0;

free_pdmdev:
    pdm_device_free(pdmdev);
    return status;
}

/**
 * @brief PLATFORM 设备移除函数
 *
 * 该函数在 PLATFORM 设备被移除时调用，负责注销和释放 PDM 设备。
 *
 * @param pdev 指向 PLATFORM 设备的指针
 * @return 成功返回 0，失败返回负错误码
 */
static int pdm_device_platform_real_remove(struct platform_device *pdev) {
    struct pdm_device_physical_info physical_info;
    struct pdm_device *pdmdev;

    physical_info.type = PDM_DEVICE_INTERFACE_TYPE_PLATFORM;
    physical_info.device= pdev;
    pdmdev = pdm_device_physical_info_match(&physical_info);
    if (!pdmdev) {
        OSA_ERROR("Failed to find pdm device from bus.\n");
        return -ENODEV;
    }

    pdm_device_unregister(pdmdev);

    OSA_DEBUG("PDM Device PLATFORM Device Removed.\n");
    return 0;
}

/**
 * @brief 兼容旧内核版本的 PLATFORM 移除函数
 *
 * 该函数用于兼容 Linux 内核版本低于 6.10.0 的情况。
 *
 * @param pdev PLATFORM 设备指针
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 10, 0)
static int pdm_device_platform_remove(struct platform_device *pdev) {
    return pdm_device_platform_real_remove(pdev);
}
#else
static void pdm_device_platform_remove(struct platform_device *pdev) {
    (void)pdm_device_platform_real_remove(pdev);
}
#endif

static const struct platform_device_id pdm_device_platform_ids[] = {
    { .name = "pdm-device-platform", .driver_data = (kernel_ulong_t)&pdm_device_platform_data_plat, },
    { .name = "pdm-device-gpio", .driver_data = (kernel_ulong_t)&pdm_device_platform_data_gpio, },
    { .name = "pdm-device-pwm", .driver_data = (kernel_ulong_t)&pdm_device_platform_data_pwm, },
    { .name = "pdm-device-tty", .driver_data = (kernel_ulong_t)&pdm_device_platform_data_tty, },
    {},
};
MODULE_DEVICE_TABLE(platform, pdm_device_platform_ids);

static struct platform_driver pdm_device_platform_driver = {
    .probe = pdm_device_platform_probe,
    .remove = pdm_device_platform_remove,
    .driver = {
        .name = "pdm-device-platform",
    },
    .id_table = pdm_device_platform_ids,
};

/**
 * @brief 初始化 PLATFORM 驱动
 *
 * 该函数用于初始化 PLATFORM 驱动，注册 PLATFORM 驱动到系统。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_device_platform_driver_init(void) {
    int status;

    status = platform_driver_register(&pdm_device_platform_driver);
    if (status) {
        OSA_ERROR("Failed to register PDM Device PLATFORM Driver, status=%d.\n", status);
        return status;
    }
    OSA_DEBUG("PDM Device PLATFORM Driver Initialized.\n");
    return 0;
}

/**
 * @brief 退出 PLATFORM 驱动
 *
 * 该函数用于退出 PLATFORM 驱动，注销 PLATFORM 驱动。
 */
void pdm_device_platform_driver_exit(void) {
    platform_driver_unregister(&pdm_device_platform_driver);
    OSA_DEBUG("PDM Device PLATFORM Driver Exited.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Device PLATFORM Driver");
