#include <linux/spi/spi.h>

#include "pdm.h"
#include "pdm_device_drivers.h"

/**
 * @brief SPI 设备探测函数
 *
 * 该函数在 SPI 设备被探测到时调用，负责初始化和注册 PDM 设备。
 *
 * @param spi 指向 SPI 设备的指针
 * @return 成功返回 0，失败返回负错误码
 */
static int pdm_device_spi_probe(struct spi_device *spi) {
    struct pdm_device *pdmdev;
    int status;

    pdmdev = pdm_device_alloc();
    if (!pdmdev) {
        OSA_ERROR("Failed to allocate pdm_device.\n");
        return -ENOMEM;
    }

    pdmdev->dev.parent = &spi->dev;
    status = pdm_device_register(pdmdev);
    if (status) {
        OSA_ERROR("Failed to register pdm device, status=%d.\n", status);
        goto free_pdmdev;
    }

    OSA_DEBUG("PDM SPI Device Probed.\n");
    return 0;

free_pdmdev:
    pdm_device_free(pdmdev);
    return status;
}

/**
 * @brief SPI 设备移除函数
 *
 * 该函数在 SPI 设备被移除时调用，负责注销和释放 PDM 设备。
 *
 * @param spi 指向 SPI 设备的指针
 */
static int pdm_device_spi_real_remove(struct spi_device *spi) {
    struct pdm_device *pdmdev;

    pdmdev = pdm_bus_find_device_by_parent(&spi->dev);
    if (!pdmdev) {
        OSA_ERROR("Failed to find pdm device from bus.\n");
        return -ENODEV;
    } else {
        OSA_DEBUG("Found SPI PDM Device: %s", dev_name(&pdmdev->dev));
    }

    pdm_device_unregister(pdmdev);

    OSA_DEBUG("PDM SPI Device Removed.\n");
    return 0;
}

/**
 * @brief 兼容旧内核版本的 SPI 移除函数
 *
 * 该函数用于兼容 Linux 内核版本低于 5.17.0 的情况。
 *
 * 该函数在 SPI 设备被移除时调用，负责注销和释放 PDM 设备。
 *
 * @param spi 指向 SPI 设备的指针
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 17, 0)
static int pdm_device_spi_remove(struct spi_device *spi) {
    return pdm_device_spi_real_remove(spi);
}
#else
static void pdm_device_spi_remove(struct spi_device *spi) {
    if (pdm_device_spi_real_remove(spi)) {
        OSA_ERROR("pdm_device_spi_real_remove failed.\n");
    }
    return;
}
#endif

/**
 * @brief SPI 设备 ID 表
 *
 * 该表定义了支持的 SPI 设备 ID。
 */
static const struct spi_device_id pdm_device_spi_ids[] = {
    { .name = "pdm-device-spi" },
    { }  // 终止符
};
MODULE_DEVICE_TABLE(spi, pdm_device_spi_ids);

/**
 * @brief SPI 驱动结构体
 *
 * 该结构体定义了 SPI 驱动的基本信息和操作函数。
 */
static struct spi_driver pdm_device_spi_driver = {
    .probe = pdm_device_spi_probe,
    .remove = pdm_device_spi_remove,
    .driver = {
        .name = "pdm-device-spi",
    },
    .id_table = pdm_device_spi_ids,
};

/**
 * @brief 初始化 PDM 设备 SPI 驱动
 *
 * 该函数用于初始化 PDM 设备 SPI 驱动，并将其注册到系统中。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_device_spi_driver_init(void) {
    int status;

    status = spi_register_driver(&pdm_device_spi_driver);
    if (status) {
        OSA_ERROR("Failed to register PDM Device SPI Driver, status=%d.\n", status);
        return status;
    }
    OSA_DEBUG("PDM Device SPI Driver Initialized.\n");
    return 0;
}

/**
 * @brief 退出 PDM 设备 SPI 驱动
 *
 * 该函数用于退出 PDM 设备 SPI 驱动，并将其从系统中注销。
 */
void pdm_device_spi_driver_exit(void) {
    spi_unregister_driver(&pdm_device_spi_driver);
    OSA_DEBUG("PDM Device SPI Driver Exited.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Device SPI Driver");
