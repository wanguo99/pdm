#include "pdm.h"
#include "pdm_component.h"

/**
 * @brief 注册一个组件
 *
 * 该函数用于注册单个 PDM 组件，调用其初始化函数并将其添加到组件链表中。
 *
 * @param driver 要注册的组件结构体指针
 * @param list 组件链表头指针
 * @return 成功返回 0，失败返回负错误码
 */
static int pdm_component_register_single(struct pdm_component *driver, struct list_head *list) {
    int status = 0;

    if (driver->status && driver->init) {
        status = driver->init();
        if (status) {
            if (driver->ignore_failures) {
                OSA_WARN("Failed to register driver %s, status = %d.\n", driver->name ? driver->name : "Unknown", status);
            } else {
                OSA_ERROR("Failed to register driver %s, status = %d.\n", driver->name ? driver->name : "Unknown", status);
                return status;
            }
        }
    }

    list_add_tail(&driver->list, list);
    return 0;
}

/**
 * @brief 卸载一个组件
 *
 * 该函数用于卸载单个 PDM 组件，调用其退出函数并将其从组件链表中移除。
 *
 * @param driver 要卸载的组件结构体指针
 */
static void pdm_component_unregister_single(struct pdm_component *driver) {
    if (driver->status && driver->exit) {
        driver->exit();
    }
    list_del(&driver->list);
}

/**
 * @brief 卸载链表中所有的驱动
 *
 * 该函数用于卸载所有已注册的 PDM 组件，依次调用每个组件的退出函数。
 *
 * @param list 组件链表头指针
 */
void pdm_component_unregister(struct list_head *list) {
    struct pdm_component *driver, *tmp;

    if (!list) {
        OSA_ERROR("Invalid or uninitialized list pointer.\n");
        return;
    }

    list_for_each_entry_safe_reverse(driver, tmp, list, list) {
        pdm_component_unregister_single(driver);
    }
}

/**
 * @brief 注册数组中所有的驱动并保存至链表
 *
 * 该函数用于注册所有 PDM 组件，依次调用每个组件的初始化函数。
 * 根据 `ignore_failures` 参数决定是否忽略某些驱动初始化失败的情况。
 *
 * @param params 组件注册参数结构体指针
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_component_register(struct pdm_component_params *params) {
    int i, status = 0;

    if (!params || !params->drivers || params->count <= 0 || !params->list) {
        OSA_ERROR("Invalid input parameters.\n");
        return -EINVAL;
    }

    for (i = 0; i < params->count; i++) {
        status = pdm_component_register_single(&params->drivers[i], params->list);
        if (status) {
            OSA_ERROR("Failed to register driver %s at index %d, status = %d.\n",
                      params->drivers[i].name ? params->drivers[i].name : "Unknown", i, status);
            pdm_component_unregister(params->list);
            return status;

        }
    }

    OSA_DEBUG("PDM Subdriver Register OK.\n");
    return 0;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Driver Manager Module.");
