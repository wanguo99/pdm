#ifndef _PDM_ADAPTER_H_
#define _PDM_ADAPTER_H_

/**
 * @file pdm_adapter.h
 * @brief PDM  Adapter模块头文件
 *
 * 本文件定义了 PDM  Adapter模块的公共数据类型、结构体和函数声明。
 */
#define PDM_ADAPTER_CLIENT_IDR_END          (1024)

/**
 * @brief PDM  Adapter结构体
 */
struct pdm_adapter {
    char name[PDM_DEVICE_NAME_SIZE];     /**<  Adapter名称 */
    struct device *dev;                  /**< 字符设备结构体 */
    dev_t devno;                         /**< 设备号 */
    struct cdev cdev;                    /**< 字符设备结构体 */
    struct file_operations fops;         /**< 文件操作结构体，每个 Adapter内部单独实现一套文件操作 */
    struct rw_semaphore rwlock;          /**< 读写锁，用于sysfs读写 Adapter属性时使用 */
    unsigned int init_done : 1;          /**< 初始化标志 */
    struct list_head entry;              /**< 挂载到bus的链表节点 */
    struct list_head client_list;        /**< 子设备列表 */
    struct mutex client_list_mutex_lock; /**< 用于保护子设备列表的互斥锁 */
    struct idr device_idr;               /**< 用于给client分配ID的IDR */
    struct mutex idr_mutex_lock;         /**< 用于保护IDR的互斥锁 */
    void *data;                          /**< 私有数据 */
};

/**
 * @brief 显示所有已注册的 PDM 设备列表
 *
 * 该函数用于显示当前已注册的所有 PDM 设备的名称。
 * 如果主设备未初始化，则会返回错误。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_adapter_client_show(struct pdm_adapter *adapter);

/**
 * @brief 添加 PDM 设备
 *
 * 该函数用于添加 PDM 设备。
 *
 * @param adapter PDM  Adapter结构体指针
 * @param pdmdev PDM 设备结构体指针
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_adapter_client_add(struct pdm_adapter *adapter, struct pdm_device *pdmdev);

/**
 * @brief 删除 PDM 设备
 *
 * 该函数用于删除 PDM 设备。
 *
 * @param adapter PDM  Adapter结构体指针
 * @param pdmdev PDM 设备结构体指针
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_adapter_client_delete(struct pdm_adapter *adapter, struct pdm_device *pdmdev);

struct pdm_adapter *dev_to_pdm_adapter(struct device *dev);


/**
 * @brief 获取 PDM  Adapter的私有数据
 *
 * 该函数用于获取 PDM  Adapter的私有数据。
 *
 * @param adapter PDM  Adapter结构体指针
 * @return 私有数据指针
 */
void *pdm_adapter_priv_data_get(struct pdm_adapter *adapter);

/**
 * @brief 分配 PDM  Adapter结构体
 *
 * 该函数用于分配新的 PDM  Adapter结构体。
 *
 * @param size 私有数据区域的大小
 * @return 分配的 PDM  Adapter结构体指针，失败返回 NULL
 */
struct pdm_adapter *pdm_adapter_alloc(unsigned int size);

/**
 * @brief 释放 PDM  Adapter结构体
 *
 * 该函数用于释放 PDM  Adapter结构体。
 *
 * @param adapter PDM  Adapter结构体指针
 */
void pdm_adapter_free(struct pdm_adapter *adapter);

/**
 * @brief 注册 PDM  Adapter
 *
 * 该函数用于注册 PDM  Adapter。
 *
 * @param adapter PDM  Adapter结构体指针
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_adapter_register(struct pdm_adapter *adapter);

/**
 * @brief 注销 PDM  Adapter
 *
 * 该函数用于注销 PDM  Adapter。
 *
 * @param adapter PDM  Adapter结构体指针
 */
void pdm_adapter_unregister(struct pdm_adapter *adapter);

/**
 * @brief 初始化 PDM  Adapter
 *
 * 该函数用于初始化 PDM  Adapter。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_adapter_init(void);

/**
 * @brief 退出 PDM  Adapter
 *
 * 该函数用于退出 PDM  Adapter。
 */
void pdm_adapter_exit(void);

#endif /* _PDM_ADAPTER_H_ */