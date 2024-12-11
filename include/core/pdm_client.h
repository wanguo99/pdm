#ifndef _PDM_CLIENT_H_
#define _PDM_CLIENT_H_


/**
 * @brief PDM Client 名称的最大长度
 */
#define PDM_CLIENT_NAME_SIZE (64)


/**
 * @brief PDM Client 结构体
 *
 * 该结构体定义了 PDM Client 的基本信息。
 */
struct pdm_client {
    char name[PDM_CLIENT_NAME_SIZE];            /**< 设备名称 */
    struct pdm_device *pdmdev;                  /**< pdm_deivce句柄 */
    bool force_dts_id;                          /**< 是否强制从dts内指定ID */
    int index;                                  /**< Adapter分配的 Client ID */
    struct pdm_adapter *adapter;                /**< 指向所属的 PDM 主控制器 */
    struct device *dev;                         /**< 设备句柄 */
    dev_t devno;                                /**< 设备号 */
    struct cdev cdev;                           /**< 字符设备结构体 */
    struct file_operations fops;                /**< 文件操作结构体，每个client单独实现一套文件操作 */
    struct list_head entry;                     /**< 设备链表节点 */
};


/**
 * @brief 注册 PDM 设备
 *
 * 该函数用于注册 PDM 设备。
 *
 * @param client PDM 设备结构体指针
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_client_register(struct pdm_adapter *adapter, struct pdm_client *client);

/**
 * @brief 注销 PDM 设备
 *
 * 该函数用于注销 PDM 设备。
 *
 * @param client PDM 设备结构体指针
 */
void pdm_client_unregister(struct pdm_adapter *adapter, struct pdm_client *client);

/**
 * @brief 分配 PDM 设备结构体
 *
 * 该函数用于分配新的 PDM 设备结构体。
 *
 * @return 分配的 PDM 设备结构体指针，失败返回 NULL
 */
struct pdm_client *pdm_client_alloc(unsigned int data_size);

/**
 * @brief 释放 PDM 设备结构体
 *
 * 该函数用于释放 PDM 设备结构体。
 *
 * @param client PDM 设备结构体指针
 */
void pdm_client_free(struct pdm_client *client);

#endif /* _PDM_CLIENT_H_ */
