#include "pdm.h"
#include "pdm_component.h"
#include "pdm_adapter_drivers.h"

/**
 * @brief PDM Adapter 列表
 *
 * 该列表用于存储所有注册的 PDM Adapter 。
 */
static struct list_head pdm_adapter_list;

/**
 * @brief 保护 PDM Adapter 列表的互斥锁
 *
 * 该互斥锁用于同步对 PDM Adapter 列表的访问，防止并发访问导致的数据竞争。
 */
static struct mutex pdm_adapter_list_mutex_lock;

/**
 * @brief 分配PDM Adapter结构
 *
 * 该函数用于分配PDM Adapter结构及其私有数据。
 *
 * @param data_size 私有数据的大小
 * @return 指向分配的PDM Adapter的指针，或NULL（失败）
 */
struct pdm_adapter *pdm_adapter_alloc(unsigned int data_size)
{
    struct pdm_adapter *adapter;
    size_t adapter_size = sizeof(struct pdm_adapter);

    adapter = kzalloc(adapter_size + data_size, GFP_KERNEL);
    if (!adapter) {
        OSA_ERROR("Failed to allocate memory for pdm_adapter.\n");
        return NULL;
    }

    INIT_LIST_HEAD(&adapter->client_list);
    mutex_init(&adapter->client_list_mutex_lock);
    adapter->data = (void *)adapter + adapter_size;

    return adapter;
}

/**
 * @brief 释放PDM Adapter结构
 *
 * 该函数用于释放PDM Adapter结构。
 *
 * @param adapter PDM Adapter
 */
void pdm_adapter_free(struct pdm_adapter *adapter)
{
    if (adapter) {
        kfree(adapter);
    }
}

/**
 * @brief 注册 PDM  Adapter
 *
 * 该函数用于注册 PDM  Adapter，并将其添加到 Adapter列表中。
 *
 * @param adapter PDM  Adapter指针
 * @return 0 - 成功
 *         -EINVAL - 参数无效
 *         -EEXIST -  Adapter名称已存在
 *         其他负值 - 其他错误码
 */
int pdm_adapter_register(struct pdm_adapter *adapter)
{
    struct pdm_adapter *existing_adapter;

    if (!adapter || !strlen(adapter->name)) {
        OSA_ERROR("Invalid input parameters (adapter: %p, name: %s).\n", adapter, adapter ? adapter->name : "NULL");
        return -EINVAL;
    }

    mutex_lock(&pdm_adapter_list_mutex_lock);
    list_for_each_entry(existing_adapter, &pdm_adapter_list, entry) {
        if (!strcmp(existing_adapter->name, adapter->name)) {
            OSA_ERROR("Adapter already exists: %s.\n", adapter->name);
            mutex_unlock(&pdm_adapter_list_mutex_lock);
            return -EEXIST;
        }
    }
    mutex_unlock(&pdm_adapter_list_mutex_lock);

    mutex_lock(&pdm_adapter_list_mutex_lock);
    list_add_tail(&adapter->entry, &pdm_adapter_list);
    mutex_unlock(&pdm_adapter_list_mutex_lock);

    mutex_init(&adapter->idr_mutex_lock);
    idr_init(&adapter->device_idr);

    adapter->init_done = true;
    OSA_DEBUG("PDM Adapter Registered: %s.\n", adapter->name);

    return 0;
}

/**
 * @brief 注销 PDM  Adapter
 *
 * 该函数用于注销 PDM  Adapter，并从 Adapter列表中移除。
 *
 * @param adapter PDM  Adapter指针
 */
void pdm_adapter_unregister(struct pdm_adapter *adapter)
{
    if (!adapter) {
        OSA_ERROR("Invalid input parameters (adapter: %p).\n", adapter);
        return;
    }

    OSA_INFO("PDM Adapter unregistered: %s.\n", adapter->name);
    adapter->init_done = false;

    mutex_lock(&pdm_adapter_list_mutex_lock);
    list_del(&adapter->entry);
    mutex_unlock(&pdm_adapter_list_mutex_lock);
}

/**
 * @brief 初始化 PDM  Adapter模块
 *
 * 该函数用于初始化 PDM  Adapter模块，包括注册类和驱动。
 *
 * @return 0 - 成功
 *         负值 - 失败
 */
int pdm_adapter_init(void)
{
    int status;

    INIT_LIST_HEAD(&pdm_adapter_list);
    mutex_init(&pdm_adapter_list_mutex_lock);

    status = pdm_adapter_drivers_register();
    if (status < 0) {
        OSA_ERROR("Failed to register PDM Adapter Drivers, error: %d.\n", status);
        return status;
    }

    OSA_DEBUG("Initialize PDM Adapter OK.\n");
    return 0;
}

/**
 * @brief 卸载 PDM  Adapter模块
 *
 * 该函数用于卸载 PDM  Adapter模块，包括注销类和驱动。
 */
void pdm_adapter_exit(void)
{
    pdm_adapter_drivers_unregister();
    OSA_DEBUG("PDM Adapter unregistered.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Adapter Module");
