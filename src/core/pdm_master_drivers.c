#include "pdm.h"
#include "pdm_driver_manager.h"
#include "pdm_master_drivers.h"

/**
 * @brief PDM 主驱动程序列表
 *
 * 该列表用于存储所有注册的 PDM 主驱动程序。
 */
static struct list_head pdm_master_driver_list;

/**
 * @brief PDM 主驱动程序数组
 *
 * 该数组包含所有需要注册的 PDM 主驱动程序。
 */
static struct pdm_subdriver pdm_master_drivers[] = {
    {
        .name = "Template Master",
        .status = true,
        .ignore_failures = true,
        .init = pdm_master_template_driver_init,
        .exit = pdm_master_template_driver_exit,
    },
    {
        .name = "LED Master",
        .status = true,
        .ignore_failures = true,
        .init = pdm_master_led_driver_init,
        .exit = pdm_master_led_driver_exit,
    },
    {
        .name = "Sensor Master",
        .status = true,
        .ignore_failures = true,
        .init = pdm_master_sensor_driver_init,
        .exit = pdm_master_sensor_driver_exit,
    },
};

/**
 * @brief 初始化 PDM 主控制器模块
 *
 * 该函数用于初始化 PDM 主控制器模块，并注册所有 PDM 主驱动程序。
 *
 * @return 0 - 成功
 *         负值 - 失败
 */
int pdm_master_drivers_register(void)
{
    int status;
    struct pdm_subdriver_register_params params;

    INIT_LIST_HEAD(&pdm_master_driver_list);
    params.drivers = pdm_master_drivers;
    params.count = ARRAY_SIZE(pdm_master_drivers);
    params.list = &pdm_master_driver_list;
    status = pdm_subdriver_register(&params);
    if (status < 0) {
        OSA_ERROR("Failed to register PDM Master Drivers, error: %d.\n", status);
        return status;
    }

    OSA_DEBUG("PDM Master Drivers Registered OK.\n");
    return 0;
}

/**
 * @brief 卸载 PDM 主控制器模块
 *
 * 该函数用于卸载 PDM 主控制器模块，并注销所有 PDM 主驱动程序。
 */
void pdm_master_drivers_unregister(void)
{
    pdm_subdriver_unregister(&pdm_master_driver_list);

    OSA_DEBUG("PDM Master Drivers Unregistered.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Master Module");
