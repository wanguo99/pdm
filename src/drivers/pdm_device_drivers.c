#include "pdm.h"
#include "pdm_component.h"
#include "pdm_device_drivers.h"

/**
 * @brief PDM Device 驱动列表
 *
 * 该列表用于存储所有注册的 PDM Device 驱动。
 */
static struct list_head pdm_device_driver_list;

/**
 * @brief PDM Device 驱动数组
 *
 * 该数组包含所有需要注册的 PDM Device 驱动。每个 `pdm_component` 结构体包含驱动程序的名称、初始化函数和退出函数。
 */
static struct pdm_component pdm_device_drivers[] = {
    {
        .name = "SPI PDM Device",
        .status = true,
        .ignore_failures = true,
        .init = pdm_device_spi_driver_init,
        .exit = pdm_device_spi_driver_exit
    },
    {
        .name = "I2C PDM Device",
        .status = true,
        .ignore_failures = true,
        .init = pdm_device_i2c_driver_init,
        .exit = pdm_device_i2c_driver_exit
    },
    {
        .name = "PLATFORM PDM Device",
        .status = true,
        .ignore_failures = true,
        .init = pdm_device_platform_driver_init,
        .exit = pdm_device_platform_driver_exit
    },
};

/**
 * @brief 初始化 PDM Device 驱动
 *
 * 该函数用于初始化 PDM Device 驱动。
 * 它会执行以下操作：
 * - 初始化 PDM Device 驱动列表
 * - 注册 PDM Device 驱动
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_device_drivers_register(void)
{
    struct pdm_component_params params;
    int status;

    INIT_LIST_HEAD(&pdm_device_driver_list);

    params.drivers = pdm_device_drivers;
    params.count = ARRAY_SIZE(pdm_device_drivers);
    params.list = &pdm_device_driver_list;
    status = pdm_component_register(&params);
    if (status < 0) {
        OSA_ERROR("Failed to register PDM Device Drivers, error: %d.\n", status);
        return status;
    }

    OSA_DEBUG("Initialize PDM Device Drivers OK.\n");
    return 0;
}

/**
 * @brief 卸载 PDM Device 驱动
 *
 * 该函数用于卸载 PDM Device，包括注销设备驱动。
 * 它会执行以下操作：
 * - 注销 PDM Device驱动
 *
 * @note 在调用此函数之前，请确保所有相关的设备已经注销。
 */
void pdm_device_drivers_unregister(void)
{
    pdm_component_unregister(&pdm_device_driver_list);
    OSA_DEBUG("PDM Device Drivers Exited.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Device Drivers");
