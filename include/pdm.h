#ifndef _PDM_H_
#define _PDM_H_

#include <linux/version.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/idr.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/mod_devicetable.h>

#include "osa_log.h"
#include "pdm_device.h"
#include "pdm_master.h"

/**
 * @file pdm.h
 * @brief PDM 模块公共头文件
 *
 * 本文件定义了 PDM 模块的公共数据类型、结构体和函数声明。
 */

/**
 * @brief PDM 设备类型
 *
 * 该变量定义了 PDM 设备的类型。
 */
extern const struct device_type pdm_device_type;

/**
 * @brief DebugFS 和 ProcFS 目录名称
 */
#define PDM_DEBUG_FS_DIR_NAME       "pdm"       /**< debugfs和procfs目录名 */

/**
 * @brief PDM 设备ID结构体
 */
struct pdm_device_id {
    char compatible[PDM_DEVICE_NAME_SIZE];      /**< 驱动匹配字符串 */
    kernel_ulong_t driver_data;                 /**< 驱动私有数据 */
};

/**
 * @brief PDM 驱动结构体
 */
struct pdm_driver {
    struct device_driver driver;                /**< 设备驱动结构体 */
    const struct pdm_device_id *id_table;       /**< 设备ID表 */
    int (*probe)(struct pdm_device *dev);       /**< 探测函数 */
    void (*remove)(struct pdm_device *dev);     /**< 移除函数 */
};

/**
 * @brief 判断 list_head 是否已经初始化
 *
 * 该函数检查 list_head 的 next 和 prev 指针是否都指向自身。
 *
 * @param head 要检查的 list_head 结构体指针
 * @return 如果已经初始化，返回 true；否则返回 false
 */
static inline bool is_list_valid(const struct list_head *head) {
    return (head->next == head) && (head->prev == head);
}

/**
 * @brief 注册 PDM 驱动
 *
 * 该函数用于注册 PDM 驱动。
 *
 * @param owner 模块指针
 * @param driver 驱动结构体指针
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_register_driver(struct module *owner, struct pdm_driver *driver);

/**
 * @brief 注销 PDM 驱动
 *
 * 该函数用于注销 PDM 驱动。
 *
 * @param driver 驱动结构体指针
 */
void pdm_unregister_driver(struct pdm_driver *driver);

/**
 * @brief PDM 总线类型
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 2, 0)
extern struct bus_type pdm_bus_type;
#else
extern const struct bus_type pdm_bus_type;
#endif

#endif /* _PDM_H_ */
