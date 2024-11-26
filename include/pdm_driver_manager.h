#ifndef _PDM_DRIVER_MANAGER_H_
#define _PDM_DRIVER_MANAGER_H_

#include "pdm.h"

/**
 * @file pdm_driver_manager.h
 * @brief PDM 驱动管理接口
 *
 * 本文件定义了 PDM 驱动管理模块的接口和结构体，用于注册和注销驱动。
 */

/**
 * @struct pdm_subdriver
 * @brief PDM 子驱动结构体定义
 *
 * 该结构体用于定义 PDM 子驱动的基本信息和操作函数。
 */
struct pdm_subdriver {
    bool status;                /**< 驱动是否加载 */
    const char *name;           /**< 子驱动的名称 */
    int (*init)(void);          /**< 子驱动的初始化函数 */
    void (*exit)(void);         /**< 子驱动的退出函数 */
    struct list_head list;      /**< 用于链表管理的节点 */
};

/**
 * @brief 子驱动注册参数结构体
 *
 * 该结构体用于封装子驱动注册所需的所有参数。
 */
struct pdm_subdriver_register_params {
    struct pdm_subdriver *drivers;      /**< 要注册的子驱动数组 */
    int count;                          /**< 子驱动数组的长度 */
    bool ignore_failures;               /**< 是否忽略某些驱动初始化失败 */
    struct list_head *list;             /**< 子驱动链表头指针 */
};

/**
 * @brief 卸载链表中所有的驱动
 *
 * 该函数用于卸载所有已注册的 PDM 子驱动，依次调用每个子驱动的退出函数。
 *
 * @param list 子驱动链表头指针
 */
void pdm_subdriver_unregister(struct list_head *list);

/**
 * @brief 注册数组中所有的驱动并保存至链表
 *
 * 该函数用于注册所有 PDM 子驱动，依次调用每个子驱动的初始化函数。
 *
 * @param params 子驱动注册参数结构体指针
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_subdriver_register(struct pdm_subdriver_register_params *params);

#endif /* _PDM_DRIVER_MANAGER_H_ */
