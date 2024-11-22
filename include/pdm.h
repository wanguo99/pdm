#ifndef _PDM_H_
#define _PDM_H_

#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/idr.h>
#include <linux/mod_devicetable.h>
#include <linux/version.h>
#include <linux/string.h>

#include "pdm_osa.h"

// 获取设备引用计数
#define DEVICE_REF_COUNT(dev) (kref_read(&(dev)->kobj.kref))

/*
 * 公共数据类型声明
 */

// PDM 设备类型
extern const struct device_type pdm_device_type;

/*
 * 公共数据类型定义
 */

// 定义设备名称的最大长度
#define PDM_DEVICE_NAME_SIZE (64)

// PDM 设备结构体
struct pdm_device {
    int id;                       // 设备ID
    const char *compatible;       // 设备兼容字符串
    struct device dev;            // 设备结构体
    struct pdm_master *master;    // 指向所属的PDM主控制器
    struct list_head entry;       // 设备链表节点
    void *real_device;            // 指向实际的设备结构体
};

// 定义PDM主控制器的ID范围
#define PDM_MASTER_IDR_START 0
#define PDM_MASTER_IDR_END 1024

// PDM 主控制器结构体
struct pdm_master {
    char name[PDM_DEVICE_NAME_SIZE];     // 主控制器名称
    struct device dev;                   // 设备结构体
    dev_t devno;                         // 设备号
    struct cdev cdev;                    // 字符设备结构体
    struct file_operations fops;         // 文件操作结构体，每个主控制器内部单独实现一套文件操作
    struct idr device_idr;               // 用于给子设备分配ID的IDR
    struct mutex idr_mutex_lock;         // 用于保护IDR的互斥锁
    struct rw_semaphore rwlock;          // 读写锁，用于sysfs读写主控制器属性时使用
    unsigned int init_done : 1;          // 初始化标志
    struct list_head entry;              // 挂载到bus的链表节点
    struct list_head client_list;        // 子设备列表
    struct mutex client_list_mutex_lock; // 用于保护子设备列表的互斥锁
};

// PDM 设备ID结构体
struct pdm_device_id {
    char compatible[PDM_DEVICE_NAME_SIZE]; // 驱动匹配字符串
    kernel_ulong_t driver_data;            // 驱动私有数据
};

// PDM 驱动结构体
struct pdm_driver {
    struct device_driver driver;           // 设备驱动结构体
    const struct pdm_device_id id_table;   // 设备ID表
    int (*probe)(struct pdm_device *dev);  // 探测函数
    void (*remove)(struct pdm_device *dev); // 移除函数
};

/*
 * 函数声明
 */

/*
 * 常用函数
 */

// 将device_driver转换为pdm_driver
static inline struct pdm_driver *drv_to_pdmdrv(struct device_driver *drv)
{
    return container_of(drv, struct pdm_driver, driver);
}

// 匹配设备ID
const struct pdm_device_id *pdm_match_id(const struct pdm_device_id *id, struct pdm_device *pdmdev);

/*
 * PDM 设备相关函数
 */

// 将device转换为pdm_device
#define dev_to_pdmdev(__dev) container_of(__dev, struct pdm_device, dev)

// 获取PDM设备的私有数据
void *pdm_device_get_devdata(struct pdm_device *pdmdev);

// 设置PDM设备的私有数据
void pdm_device_set_devdata(struct pdm_device *pdmdev, void *data);

// 分配PDM设备结构体
struct pdm_device *pdm_device_alloc(unsigned int data_size);

// 释放PDM设备结构体
void pdm_device_free(struct pdm_device *pdmdev);

// 注册PDM设备
int pdm_device_register(struct pdm_device *pdmdev);

// 注销PDM设备
void pdm_device_unregister(struct pdm_device *pdmdev);

/*
 * PDM 主控制器相关函数
 */

// 将device转换为pdm_master
#define dev_to_pdm_master(__dev) container_of(__dev, struct pdm_master, dev)

// 获取PDM主控制器的私有数据
void *pdm_master_get_devdata(struct pdm_master *master);

// 设置PDM主控制器的私有数据
void pdm_master_set_devdata(struct pdm_master *master, void *data);

// 分配PDM主控制器结构体
struct pdm_master *pdm_master_alloc(unsigned int size);

// 释放PDM主控制器结构体
void pdm_master_free(struct pdm_master *master);

// 获取PDM主控制器的引用
struct pdm_master *pdm_master_get(struct pdm_master *master);

// 释放PDM主控制器的引用
void pdm_master_put(struct pdm_master *master);

// 注册PDM主控制器
int pdm_master_register(struct pdm_master *master);

// 注销PDM主控制器
void pdm_master_unregister(struct pdm_master *master);

// 初始化PDM主控制器
int pdm_master_init(void);

// 退出PDM主控制器
void pdm_master_exit(void);

// 查找PDM设备
struct pdm_device *pdm_master_find_pdmdev(struct pdm_master *master, void *real_device);

// 分配PDM设备ID
int pdm_master_id_alloc(struct pdm_master *master, struct pdm_device *pdmdev);

// 释放PDM设备ID
void pdm_master_id_free(struct pdm_master *master, struct pdm_device *pdmdev);

// 添加PDM设备
int pdm_master_add_device(struct pdm_master *master, struct pdm_device *pdmdev);

// 删除PDM设备
int pdm_master_delete_device(struct pdm_master *master, struct pdm_device *pdmdev);

/*
 * 全局变量声明
 */

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 16, 0)
extern struct bus_type pdm_bus_type;
#else
extern const struct bus_type pdm_bus_type;
#endif

#endif /* _PDM_H_ */
