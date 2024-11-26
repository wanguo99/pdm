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

#include "osa/osa_log.h"
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
 * @brief PDM 主控制器的ID范围
 */
#define PDM_BUS_DEVICE_IDR_START 0
#define PDM_BUS_DEVICE_IDR_END 1024

/**
 * @brief PDM BUS结构体
 *
 * 保存PDM BUS私有数据。
 */
struct pdm_bus {
    struct idr device_idr;               /**< 用于给子设备分配ID的IDR */
    struct mutex idr_mutex_lock;         /**< 用于保护IDR的互斥锁 */
};

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
    if (head == NULL)
    {
        return false;
    }
    return true;
}

/**
 * @brief 为PDM设备分配ID
 * @master: PDM主控制器
 * @pdmdev: PDM设备
 *
 * 返回值:
 * 0 - 成功
 * -EINVAL - 参数无效
 * -EBUSY - 没有可用的ID
 * 其他负值 - 其他错误码
 */
int pdm_bus_device_id_alloc(struct pdm_device *pdmdev);


/**
 * @brief 释放PDM设备的ID
 * @master: PDM主控制器
 * @pdmdev: PDM设备
 */
void pdm_bus_device_id_free(struct pdm_device *pdmdev);

/**
 * @brief 遍历 pdm_bus_type 总线上的所有设备
 *
 * 该函数用于遍历 `pdm_bus_type` 总线上的所有设备，并对每个设备调用指定的回调函数。
 *
 * @param data 传递给回调函数的数据
 * @param fn 回调函数指针，用于处理每个设备
 * @return 返回遍历结果，0 表示成功，非零值表示失败
 */
int pdm_bus_for_each_dev(void *data, int (*fn)(struct device *dev, void *data));

/**
 * @brief 查找与给定物理信息匹配的 PDM 设备
 *
 * 该函数用于查找与给定物理信息匹配的 PDM 设备。
 *
 * @param physical_info 要匹配的物理设备信息
 * @return 返回匹配的设备指针，如果没有找到则返回 NULL
 */
struct pdm_device *pdm_bus_physical_info_match_pdm_device(struct pdm_device_physical_info *physical_info);

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

extern struct pdm_bus pdm_bus_instance;

#endif /* _PDM_H_ */
