#include "pdm.h"
#include "pdm_component.h"
#include "pdm_adapter_drivers.h"

/**
 * @brief PDM Adapter 驱动列表
 *
 * 该列表用于存储所有注册的 PDM Adapter 驱动。
 */
static struct list_head pdm_adapter_driver_list;

/**
 * @brief PDM Adapter 驱动数组
 *
 * 该数组包含所有需要注册的 PDM Adapter 驱动。
 */
static struct pdm_component pdm_adapter_drivers[] = {
    {
        .name = "LED Adapter",
        .status = true,
        .ignore_failures = true,
        .init = pdm_led_driver_init,
        .exit = pdm_led_driver_exit,
    },
    {}  /* end of array */
};

/**
 * @brief 注册 PDM Adapter 驱动
 *
 * 该函数用于初始化 PDM Adapter 列表，并注册所有 PDM Adapter 驱动。
 *
 * @return 0 - 成功
 *         负值 - 失败
 */
int pdm_adapter_drivers_register(void)
{
    int status;
    struct pdm_component_params params;

    INIT_LIST_HEAD(&pdm_adapter_driver_list);
    params.components = pdm_adapter_drivers;
    params.count = ARRAY_SIZE(pdm_adapter_drivers);
    params.list = &pdm_adapter_driver_list;
    status = pdm_component_register(&params);
    if (status < 0) {
        OSA_ERROR("Failed to register PDM Adapter Drivers, error: %d.\n", status);
        return status;
    }

    OSA_DEBUG("PDM Adapter Drivers Registered OK.\n");
    return 0;
}

/**
 * @brief 卸载 PDM Adapter 驱动
 *
 * 该函数用于卸载所有 PDM Adapter 驱动。
 */
void pdm_adapter_drivers_unregister(void)
{
    pdm_component_unregister(&pdm_adapter_driver_list);

    OSA_DEBUG("PDM Adapter Drivers Unregistered.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Adapter Drivers");
