#ifndef _PDM_MASTER_H_
#define _PDM_MASTER_H_

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

/**
 * @file pdm_master.h
 * @brief PDM 主控制器模块头文件
 *
 * 本文件定义了 PDM 主控制器模块的公共数据类型、结构体和函数声明。
 */

/**
 * @brief PDM 主控制器的ID范围
 */
#define PDM_MASTER_CLIENT_IDR_START 0
#define PDM_MASTER_CLIENT_IDR_END 1024

/**
 * @brief PDM 主控制器结构体
 */
struct pdm_master {
    char name[PDM_DEVICE_NAME_SIZE];     /**< 主控制器名称 */
    struct device dev;                   /**< 设备结构体 */
    dev_t devno;                         /**< 设备号 */
    struct cdev cdev;                    /**< 字符设备结构体 */
    struct file_operations fops;         /**< 文件操作结构体，每个主控制器内部单独实现一套文件操作 */
    struct idr device_idr;               /**< 用于给子设备分配ID的IDR */
    struct mutex idr_mutex_lock;         /**< 用于保护IDR的互斥锁 */
    struct rw_semaphore rwlock;          /**< 读写锁，用于sysfs读写主控制器属性时使用 */
    unsigned int init_done : 1;          /**< 初始化标志 */
    struct list_head entry;              /**< 挂载到bus的链表节点 */
    struct list_head client_list;        /**< 子设备列表 */
    struct mutex client_list_mutex_lock; /**< 用于保护子设备列表的互斥锁 */
};

/**
 * @brief 分配 PDM 设备ID
 *
 * 该函数用于分配 PDM 设备ID。
 *
 * @param master PDM 主控制器结构体指针
 * @param pdmdev PDM 设备结构体指针
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_master_client_id_alloc(struct pdm_master *master, struct pdm_device *pdmdev);

/**
 * @brief 释放 PDM 设备ID
 *
 * 该函数用于释放 PDM 设备ID。
 *
 * @param master PDM 主控制器结构体指针
 * @param pdmdev PDM 设备结构体指针
 */
void pdm_master_client_id_free(struct pdm_master *master, struct pdm_device *pdmdev);

/**
 * @brief 显示所有已注册的 PDM 设备列表
 *
 * 该函数用于显示当前已注册的所有 PDM 设备的名称。
 * 如果主设备未初始化，则会返回错误。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_master_client_show(struct pdm_master *master);

/**
 * @brief 根据 real_device 和 interface 查找 PDM 设备
 *
 * 该函数遍历 PDM 主控制器的客户设备链表，根据指定的 real_device 和 interface 找到对应的 PDM 设备。
 *
 * @param master 指向 PDM 主控制器的指针
 * @param real_device 指向实际设备结构体的指针
 * @param interface 设备物理接口类型
 * @return 成功返回找到的 PDM 设备指针，失败返回 NULL
 */
struct pdm_device *pdm_master_client_find(struct pdm_master *master, void *real_device, int interface);

/**
 * @brief 添加 PDM 设备
 *
 * 该函数用于添加 PDM 设备。
 *
 * @param master PDM 主控制器结构体指针
 * @param pdmdev PDM 设备结构体指针
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_master_client_add(struct pdm_master *master, struct pdm_device *pdmdev);

/**
 * @brief 删除 PDM 设备
 *
 * 该函数用于删除 PDM 设备。
 *
 * @param master PDM 主控制器结构体指针
 * @param pdmdev PDM 设备结构体指针
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_master_client_delete(struct pdm_master *master, struct pdm_device *pdmdev);

/**
 * @brief 将 device 转换为 pdm_master
 *
 * 该宏用于将 device 转换为 pdm_master。
 *
 * @param __dev device 结构体指针
 * @return pdm_master 结构体指针
 */
#define dev_to_pdm_master(__dev) container_of(__dev, struct pdm_master, dev)

/**
 * @brief 获取 PDM 主控制器的私有数据
 *
 * 该函数用于获取 PDM 主控制器的私有数据。
 *
 * @param master PDM 主控制器结构体指针
 * @return 私有数据指针
 */
void *pdm_master_devdata_get(struct pdm_master *master);

/**
 * @brief 设置 PDM 主控制器的私有数据
 *
 * 该函数用于设置 PDM 主控制器的私有数据。
 *
 * @param master PDM 主控制器结构体指针
 * @param data 私有数据指针
 */
void pdm_master_devdata_set(struct pdm_master *master, void *data);

/**
 * @brief 获取 PDM 主控制器的引用
 *
 * 该函数用于获取 PDM 主控制器的引用。
 *
 * @param master PDM 主控制器结构体指针
 * @return 成功返回 PDM 主控制器结构体指针，失败返回 NULL
 */
struct pdm_master *pdm_master_get(struct pdm_master *master);

/**
 * @brief 释放 PDM 主控制器的引用
 *
 * 该函数用于释放 PDM 主控制器的引用。
 *
 * @param master PDM 主控制器结构体指针
 */
void pdm_master_put(struct pdm_master *master);

/**
 * @brief 分配 PDM 主控制器结构体
 *
 * 该函数用于分配新的 PDM 主控制器结构体。
 *
 * @param size 私有数据区域的大小
 * @return 分配的 PDM 主控制器结构体指针，失败返回 NULL
 */
struct pdm_master *pdm_master_alloc(unsigned int size);

/**
 * @brief 释放 PDM 主控制器结构体
 *
 * 该函数用于释放 PDM 主控制器结构体。
 *
 * @param master PDM 主控制器结构体指针
 */
void pdm_master_free(struct pdm_master *master);

/**
 * @brief 注册 PDM 主控制器
 *
 * 该函数用于注册 PDM 主控制器。
 *
 * @param master PDM 主控制器结构体指针
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_master_register(struct pdm_master *master);

/**
 * @brief 注销 PDM 主控制器
 *
 * 该函数用于注销 PDM 主控制器。
 *
 * @param master PDM 主控制器结构体指针
 */
void pdm_master_unregister(struct pdm_master *master);

/**
 * @brief 初始化 PDM 主控制器
 *
 * 该函数用于初始化 PDM 主控制器。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_master_init(void);

/**
 * @brief 退出 PDM 主控制器
 *
 * 该函数用于退出 PDM 主控制器。
 */
void pdm_master_exit(void);

#endif /* _PDM_MASTER_H_ */
