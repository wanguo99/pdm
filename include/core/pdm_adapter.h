#ifndef _PDM_ADAPTER_H_
#define _PDM_ADAPTER_H_

/**
 * @file pdm_adapter.h
 * @brief PDM Adapter module header file.
 *
 * This file defines public data types, structures, and function declarations for the PDM Adapter module.
 */

#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/idr.h>
#include <linux/device.h>
#include <linux/rwsem.h>

/**
 * @brief Maximum number of clients that can be registered with an adapter.
 */
#define PDM_ADAPTER_CLIENT_IDR_END          (1024)

/**
 * @brief PDM Adapter structure.
 *
 * This structure represents a PDM Adapter, which manages multiple PDM Client devices.
 */
struct pdm_adapter {
    char name[PDM_DEVICE_NAME_SIZE];        /**< Adapter name */
    struct list_head entry;                 /**< List node handle */
    struct list_head client_list;           /**< List of child devices */
    struct mutex client_list_mutex_lock;    /**< Mutex to protect the client list */
    struct idr device_idr;                  /**< IDR for allocating unique IDs to clients */
    struct mutex idr_mutex_lock;            /**< Mutex to protect the IDR */
    struct device dev;                      /**< Kernel device structure */
    struct rw_semaphore rwlock;             /**< Read-write semaphore for sysfs attribute access */
};

/**
 * @brief Macro to convert a device pointer to a pdm_adapter pointer.
 *
 * @param __dev Pointer to the device structure.
 * @return Pointer to the pdm_adapter structure.
 */
#define dev_to_pdm_adapter(__dev) container_of(__dev, struct pdm_adapter, dev)

/**
 * @brief Gets private data from a PDM Adapter.
 *
 * This function retrieves the private data associated with a PDM Adapter.
 *
 * @param adapter Pointer to the PDM Adapter structure.
 * @return Pointer to the private data.
 */
void *pdm_adapter_devdata_get(struct pdm_adapter *adapter);

/**
 * @brief Sets private data for a PDM Adapter.
 *
 * This function sets the private data associated with a PDM Adapter.
 *
 * @param adapter Pointer to the PDM Adapter structure.
 * @param data Pointer to the private data.
 */
void pdm_adapter_devdata_set(struct pdm_adapter *adapter, void *data);

/**
 * @brief Gets a reference to a PDM Adapter.
 *
 * This function increments the reference count of a PDM Adapter.
 *
 * @param adapter Pointer to the PDM Adapter structure.
 * @return Pointer to the PDM Adapter structure, or NULL on failure.
 */
struct pdm_adapter *pdm_adapter_get(struct pdm_adapter *adapter);

/**
 * @brief Allocates a unique ID for a PDM Client.
 *
 * This function allocates a unique ID for a PDM Client managed by the specified adapter.
 *
 * @param adapter Pointer to the PDM Adapter structure.
 * @param client Pointer to the PDM Client structure.
 * @return 0 on success, negative error code on failure.
 */
int pdm_adapter_id_alloc(struct pdm_adapter *adapter, struct pdm_client *client);

/**
 * @brief Frees the ID allocated for a PDM Client.
 *
 * This function frees the ID previously allocated for a PDM Client.
 *
 * @param adapter Pointer to the PDM Adapter structure.
 * @param client Pointer to the PDM Client structure.
 */
void pdm_adapter_id_free(struct pdm_adapter *adapter, struct pdm_client *client);

/**
 * @brief Releases a reference to a PDM Adapter.
 *
 * This function decrements the reference count of a PDM Adapter.
 *
 * @param adapter Pointer to the PDM Adapter structure.
 */
void pdm_adapter_put(struct pdm_adapter *adapter);

/**
 * @brief Registers a PDM Adapter.
 *
 * This function registers a new PDM Adapter with the system.
 *
 * @param adapter Pointer to the PDM Adapter structure.
 * @param name Name of the adapter.
 * @return 0 on success, negative error code on failure.
 */
int pdm_adapter_register(struct pdm_adapter *adapter, const char *name);

/**
 * @brief Unregisters a PDM Adapter.
 *
 * This function unregisters a PDM Adapter from the system and cleans up resources.
 *
 * @param adapter Pointer to the PDM Adapter structure.
 */
void pdm_adapter_unregister(struct pdm_adapter *adapter);

/**
 * @brief Allocates a new PDM Adapter structure.
 *
 * This function allocates and initializes a new PDM Adapter structure.
 *
 * @param size Size of the private data area to allocate.
 * @return Pointer to the allocated PDM Adapter structure, or NULL on failure.
 */
struct pdm_adapter *pdm_adapter_alloc(unsigned int size);

/**
 * @brief Frees a PDM Adapter structure.
 *
 * This function frees an allocated PDM Adapter structure and its associated resources.
 *
 * @param adapter Pointer to the PDM Adapter structure.
 */
void pdm_adapter_free(struct pdm_adapter *adapter);

/**
 * @brief Initializes the PDM Adapter module.
 *
 * This function initializes the PDM Adapter module, including registering necessary drivers and setting initial states.
 *
 * @return 0 on success, negative error code on failure.
 */
int pdm_adapter_init(void);

/**
 * @brief Cleans up the PDM Adapter module.
 *
 * This function exits the PDM Adapter module, including unregistering drivers and cleaning up all resources.
 */
void pdm_adapter_exit(void);

#endif /* _PDM_ADAPTER_H_ */
