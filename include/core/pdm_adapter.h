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
    char name[PDM_DEVICE_NAME_SIZE];        /**< Adapter名称 */
    struct list_head entry;                 /**< 链表节点句柄 */
    struct list_head client_list;           /**< 子设备列表 */
    struct mutex client_list_mutex_lock;    /**< 用于保护子设备列表的互斥锁 */
    struct idr device_idr;                  /**< 用于给client分配ID的IDR */
    struct mutex idr_mutex_lock;            /**< 用于保护IDR的互斥锁 */
    struct device dev;                      /**< 设备结构体 */
    struct rw_semaphore rwlock;             /**< 读写锁，用于sysfs读写主控制器属性时使用 */
};


/**
 * @brief 将 device 转换为 pdm_adapter
 *
 * 该宏用于将 device 转换为 pdm_adapter
 *
 * @param __dev device 结构体指针
 * @return pdm_adapter 结构体指针
 */
#define dev_to_pdm_adapter(__dev) container_of(__dev, struct pdm_adapter, dev)

/**
 * @brief 获取 PDM Adapter 的私有数据
 *
 * 该函数用于获取 PDM Adapter 的私有数据。
 *
 * @param adapter PDM Adapter 结构体指针
 * @return 私有数据指针
 */
void *pdm_adapter_devdata_get(struct pdm_adapter *adapter);

/**
 * @brief 设置 PDM Adapter 的私有数据
 *
 * 该函数用于设置 PDM Adapter 的私有数据。
 *
 * @param adapter PDM Adapter 结构体指针
 * @param data 私有数据指针
 */
void pdm_adapter_devdata_set(struct pdm_adapter *adapter, void *data);

/**
 * @brief 获取 PDM Adapter 的引用
 *
 * 该函数用于获取 PDM Adapter 的引用。
 *
 * @param adapter PDM Adapter 结构体指针
 * @return 成功返回 PDM Adapter 结构体指针，失败返回 NULL
 */
struct pdm_adapter *pdm_adapter_get(struct pdm_adapter *adapter);


/**
 * @brief 分配PDM设备ID
 *
 * 该函数用于分配一个唯一的ID给PDM设备。
 *
 * @param adapter PDM Adapter
 * @param pdmdev PDM设备
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_adapter_id_alloc(struct pdm_adapter *adapter, struct pdm_client *client);

/**
 * @brief 释放 PDM Client 的ID
 *
 * 该函数用于释放 PDM Client 的ID。
 *
 * @param adapter PDM Adapter
 * @param client PDM Client
 */
void pdm_adapter_id_free(struct pdm_adapter *adapter, struct pdm_client *client);

/**
 * @brief 释放 PDM Adapter 的引用
 *
 * 该函数用于释放 PDM Adapter 的引用。
 *
 * @param adapter PDM Adapter 结构体指针
 */
void pdm_adapter_put(struct pdm_adapter *adapter);

/**
 * @brief 注册 PDM  Adapter
 *
 * 该函数用于注册 PDM  Adapter。
 *
 * @param adapter PDM  Adapter结构体指针
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_adapter_register(struct pdm_adapter *adapter, const char *name);

/**
 * @brief 注销 PDM  Adapter
 *
 * 该函数用于注销 PDM  Adapter。
 *
 * @param adapter PDM  Adapter结构体指针
 */
void pdm_adapter_unregister(struct pdm_adapter *adapter);

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
