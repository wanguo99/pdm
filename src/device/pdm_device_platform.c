#include <linux/platform_device.h>

#include "pdm.h"

/**
 * @brief PLATFORM 设备平台数据结构体
 *
 * 该结构体用于存储 PLATFORM 设备的平台数据。
 */
struct pdm_device_platform_data {
    PDM_DEVICE_INTERFACE_TYPE type;  /**< 设备接口类型 */
};

/**
 * @brief PLATFORM 设备平台数据实例
 *
 * 这些变量定义了不同类型的 PLATFORM 设备的平台数据。
 */
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
 * @brief 获取 PLATFORM 设备类型
 *
 * 该函数从设备树或平台数据中获取 PLATFORM 设备的类型。
 *
 * @param pdev 指向 PLATFORM 设备的指针
 * @return 返回设备类型
 */
static int pdm_device_platform_get_dev_type(struct platform_device *pdev) {
    struct pdm_device_platform_data *pdata;
    int dev_type;

    pdata = dev_get_platdata(&pdev->dev);
    if (pdata) {
        return pdata->type;
    }

    // 后续需要删掉，pdm_device_platform不对外支持compatible匹配
    if (of_device_is_compatible(pdev->dev.of_node, "led,pdm-device-gpio")) {
        dev_type = PDM_DEVICE_INTERFACE_TYPE_GPIO;
    } else if (of_device_is_compatible(pdev->dev.of_node, "led,pdm-device-pwm")) {
        dev_type = PDM_DEVICE_INTERFACE_TYPE_PWM;
    } else {
        dev_type = PDM_DEVICE_INTERFACE_TYPE_UNDEFINED;
    }
    return dev_type;
}

/**
 * @brief PLATFORM 设备探测函数
 *
 * 该函数在 PLATFORM 设备被探测到时调用，负责初始化和注册 PDM 设备。
 *
 * @param pdev 指向 PLATFORM 设备的指针
 * @return 成功返回 0，失败返回负错误码
 */
static int pdm_device_platform_probe(struct platform_device *pdev) {
    struct pdm_device *pdmdev;
    int status;

    OSA_DEBUG("PDM Device PLATFORM Device Probe.\n");

    pdmdev = pdm_device_alloc();
    if (!pdmdev) {
        OSA_ERROR("Failed to allocate pdm_device.\n");
        return -ENOMEM;
    }

    pdmdev->physical_info.type = pdm_device_platform_get_dev_type(pdev);
    pdmdev->physical_info.device.pdev = pdev;
    pdmdev->physical_info.of_node = pdev->dev.of_node;
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
    struct pdm_device *pdmdev;

    pdmdev = pdm_bus_find_device_by_of_node(pdev->dev.of_node);
    if (!pdmdev) {
        OSA_ERROR("Failed to find pdm device from bus.\n");
        return -ENODEV;
    } else {
        OSA_DEBUG("Found SPI PDM Device: %s", dev_name(&pdmdev->dev));
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
    int status;
    status = pdm_device_platform_real_remove(pdev);
    if (status) {
        OSA_ERROR("pdm_device_platform_real_remove failed.\n");
    }
}
#endif

/**
 * @brief PLATFORM 设备 ID 表
 *
 * 该表定义了支持的 PLATFORM 设备 ID。
 */
static const struct platform_device_id pdm_device_platform_ids[] = {
    { .name = "pdm-device-platform", .driver_data = (kernel_ulong_t)&pdm_device_platform_data_plat, },
    { .name = "pdm-device-gpio", .driver_data = (kernel_ulong_t)&pdm_device_platform_data_gpio, },
    { .name = "pdm-device-pwm", .driver_data = (kernel_ulong_t)&pdm_device_platform_data_pwm, },
    { .name = "pdm-device-tty", .driver_data = (kernel_ulong_t)&pdm_device_platform_data_tty, },
    { },  // 终止符
};
MODULE_DEVICE_TABLE(platform, pdm_device_platform_ids);

/**
 * @brief DEVICE_TREE 匹配表
 *
 * 该表定义了支持的 DEVICE_TREE 兼容性字符串。
 */
static const struct of_device_id pdm_device_platform_of_match[] = {
    { .compatible = "led,pdm-device-gpio" },
    { .compatible = "led,pdm-device-pwm" },
    { },  // 终止符
};
MODULE_DEVICE_TABLE(of, pdm_device_platform_of_match);

/**
 * @brief PLATFORM 驱动结构体
 *
 * 该结构体定义了 PLATFORM 驱动的基本信息和操作函数。
 */
static struct platform_driver pdm_device_platform_driver = {
    .probe = pdm_device_platform_probe,
    .remove = pdm_device_platform_remove,
    .driver = {
        .name = "pdm-device-platform",
        .of_match_table = pdm_device_platform_of_match,
    },
    .id_table = pdm_device_platform_ids,
};

/**
 * @brief 初始化 PLATFORM 驱动
 *
 * 该函数用于初始化 PLATFORM 驱动，并将其注册到系统中。
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
 * 该函数用于退出 PLATFORM 驱动，并将其从系统中注销。
 */
void pdm_device_platform_driver_exit(void) {
    platform_driver_unregister(&pdm_device_platform_driver);
    OSA_DEBUG("PDM Device PLATFORM Driver Exited.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Device PLATFORM Driver");
