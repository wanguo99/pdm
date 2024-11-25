#include <linux/spi/spi.h>

#include "pdm.h"

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

    pdmdev = pdm_device_alloc(sizeof(void*));
    if (!pdmdev) {
        OSA_ERROR("Failed to allocate pdm_device.\n");
        return -ENOMEM;
    }

    pdmdev->interface = PDM_DEVICE_INTERFACE_TYPE_SPI;
    pdmdev->real_device.spi = spi;
    status = pdm_device_register(pdmdev);
    if (status) {
        OSA_ERROR("Failed to register pdm device, status=%d.\n", status);
        goto free_pdmdev;
    }

    OSA_INFO("PDM SPI Device Probed.\n");
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
static void pdm_device_spi_remove(struct spi_device *spi) {
#if 0
    struct pdm_device *pdmdev = pdm_master_client_find(spi);
    if (NULL == pdmdev) {
        OSA_ERROR("Failed to find pdm device from master.\n");
        return;
    }

    pdm_device_unregister(pdmdev);
    pdm_device_free(pdmdev);
#endif

    OSA_INFO("PDM SPI Device Removed.\n");
    return;
}

static const struct spi_device_id pdm_device_spi_ids[] = {
    { .name = "pdm-device-spi" },
    { }
};
MODULE_DEVICE_TABLE(spi, pdm_device_spi_ids);

static struct spi_driver pdm_device_spi_driver = {
    .probe = pdm_device_spi_probe,
    .remove = pdm_device_spi_remove,
    .driver = {
        .name = "pdm-device-spi",
    },
    .id_table = pdm_device_spi_ids,
};

/**
 * @brief 初始化 SPI 驱动
 *
 * 该函数用于初始化 SPI 驱动，注册 SPI 驱动到系统。
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
    OSA_INFO("PDM Device SPI Driver Initialized.\n");
    return 0;
}

/**
 * @brief 退出 SPI 驱动
 *
 * 该函数用于退出 SPI 驱动，注销 SPI 驱动。
 */
void pdm_device_spi_driver_exit(void) {
    spi_unregister_driver(&pdm_device_spi_driver);
    OSA_INFO("PDM Device SPI Driver Exited.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Device SPI Driver");
