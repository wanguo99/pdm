#ifndef _PDM_SUBMODULE_H_
#define _PDM_SUBMODULE_H_

#include <linux/list.h>

/**
 * @file pdm_submodule.h
 * @brief PDM 子驱动管理接口
 *
 * 本文件定义了 PDM 子驱动的结构体和相关管理函数，用于注册和注销子驱动。
 */

/**
 * @struct pdm_subdriver
 * @brief PDM 子驱动结构体定义
 *
 * 该结构体用于定义 PDM 子驱动的基本信息和操作函数。
 */
struct pdm_subdriver {
    const char *name;           /**< 子驱动的名称 */
    int (*init)(void);          /**< 子驱动的初始化函数 */
    void (*exit)(void);         /**< 子驱动的退出函数 */
    struct list_head list;      /**< 用于链表管理的节点 */
};

/**
 * @brief 注册所有 PDM 子驱动
 *
 * 该函数用于注册所有 PDM 子驱动，调用每个子驱动的初始化函数。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_submodule_register_drivers(void);

/**
 * @brief 注销所有 PDM 子驱动
 *
 * 该函数用于注销所有 PDM 子驱动，调用每个子驱动的退出函数。
 */
void pdm_submodule_unregister_drivers(void);

#endif /* _PDM_SUBMODULE_H_ */
