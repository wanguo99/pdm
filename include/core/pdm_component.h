#ifndef _PDM_COMPONENT_H_
#define _PDM_COMPONENT_H_

/**
 * @file pdm_driver_manager.h
 * @brief PDM 驱动管理接口
 *
 * 本文件定义了 PDM 驱动管理模块的接口和结构体，用于注册和注销驱动。
 */

/**
 * @struct pdm_component
 * @brief PDM 组件结构体定义
 *
 * 该结构体用于定义 PDM 组件的基本信息和操作函数。
 */
struct pdm_component {
    bool status;                /**< 驱动是否加载，默认为false，设置为true后开启 */
    bool ignore_failures;       /**< 是否忽略驱动初始化失败 */
    const char *name;           /**< 组件的名称 */
    int (*init)(void);          /**< 组件的初始化函数 */
    void (*exit)(void);         /**< 组件的退出函数 */
    struct list_head list;      /**< 用于链表管理的节点 */
};

/**
 * @brief 组件注册参数结构体
 *
 * 该结构体用于封装组件注册所需的所有参数。
 */
struct pdm_component_params {
    struct pdm_component *drivers;      /**< 要注册的组件数组 */
    int count;                          /**< 组件数组的长度 */
    struct list_head *list;             /**< 组件链表头指针 */
};

/**
 * @brief 卸载链表中所有的驱动
 *
 * 该函数用于卸载所有已注册的 PDM 组件，依次调用每个组件的退出函数。
 *
 * @param list 组件链表头指针
 */
void pdm_component_unregister(struct list_head *list);

/**
 * @brief 注册数组中所有的驱动并保存至链表
 *
 * 该函数用于注册所有 PDM 组件，依次调用每个组件的初始化函数。
 *
 * @param params 组件注册参数结构体指针
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_component_register(struct pdm_component_params *params);

#endif /* _PDM_COMPONENT_H_ */
