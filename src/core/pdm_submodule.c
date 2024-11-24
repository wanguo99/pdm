#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/list.h>

#include "pdm.h"
#include "pdm_submodule.h"
#include "pdm_template.h"

/**
 * @brief PDM 子驱动链表头
 *
 * 该链表用于管理所有注册的 PDM 子驱动。
 */
static LIST_HEAD(pdm_submodule_driver_list);

/**
 * @brief PDM 子驱动数组
 *
 * 该数组定义了所有需要注册的 PDM 子驱动。
 */
static struct pdm_subdriver sub_drivers[] = {
    { .name = "Template Master", .init = pdm_template_master_init, .exit = pdm_template_master_exit },
    // 可以按照需要添加更多的驱动，需注意驱动之间的依赖关系
    { .name = "Template I2C Driver", .init = pdm_template_i2c_driver_init, .exit = pdm_template_i2c_driver_exit },
    { .name = "Template GPIO Driver", .init = pdm_template_platform_driver_init, .exit = pdm_template_platform_driver_exit },
    { }  // 末尾的空项用于标记数组结束
};

/**
 * @brief 注册单个 PDM 子驱动
 *
 * 该函数用于注册单个 PDM 子驱动，调用其初始化函数并将其添加到子驱动链表中。
 *
 * @param driver 要注册的子驱动结构体指针
 * @return 成功返回 0，失败返回负错误码
 */
static int pdm_submodule_register_driver(struct pdm_subdriver *driver) {
    int ret = 0;
    if (driver->init) {
        ret = driver->init();
        if (ret) {
            OSA_ERROR("Failed to register driver %s, ret = %d.\n", driver->name ? driver->name : "Unknown", ret);
            return ret;
        }
    }
    list_add_tail(&driver->list, &pdm_submodule_driver_list);
    return 0;
}

/**
 * @brief 注销单个 PDM 子驱动
 *
 * 该函数用于注销单个 PDM 子驱动，调用其退出函数并将其从子驱动链表中移除。
 *
 * @param driver 要注销的子驱动结构体指针
 */
static void pdm_submodule_unregister_driver(struct pdm_subdriver *driver) {
    if (driver->exit) {
        driver->exit();
    }
    list_del(&driver->list);
}

/**
 * @brief 注册所有 PDM 子驱动
 *
 * 该函数用于注册所有 PDM 子驱动，依次调用每个子驱动的初始化函数。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_submodule_register_drivers(void) {
    int i, ret = 0;

    for (i = 0; sub_drivers[i].name; i++) {
        ret = pdm_submodule_register_driver(&sub_drivers[i]);
        if (ret) {
            OSA_ERROR("Failed to register driver %s at index %d\n", sub_drivers[i].name, i);
            pdm_submodule_unregister_drivers();  // 注册失败时，注销已注册的驱动
            return ret;
        }
    }
    return 0;
}

/**
 * @brief 注销所有 PDM 子驱动
 *
 * 该函数用于注销所有 PDM 子驱动，依次调用每个子驱动的退出函数。
 */
void pdm_submodule_unregister_drivers(void) {
    struct pdm_subdriver *driver, *tmp;
    list_for_each_entry_safe_reverse(driver, tmp, &pdm_submodule_driver_list, list) {
        pdm_submodule_unregister_driver(driver);
    }
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Submodule Driver.");
