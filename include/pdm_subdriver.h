#ifndef _PDM_SUBMODULE_H_
#define _PDM_SUBMODULE_H_


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
 * @param drivers 要注册的子驱动数组
 * @param count 子驱动数组的长度
 * @param list 子驱动链表头指针
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_subdriver_register(struct pdm_subdriver *drivers, int count, struct list_head *list);

#endif /* _PDM_SUBMODULE_H_ */
