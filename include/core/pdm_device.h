#ifndef _PDM_DEVICE_H_
#define _PDM_DEVICE_H_

#include <linux/i2c.h>
#include <linux/i3c/master.h>
#include <linux/spi/spi.h>
#include <linux/platform_device.h>

/**
 * @brief Maximum length of device names.
 */
#define PDM_DEVICE_NAME_SIZE (64)

/**
 * @struct pdm_device
 * @brief Structure defining a PDM device.
 *
 * Contains essential information about a PDM device, including its ID,
 * device structure, and client handle.
 */
struct pdm_device {
    struct device dev;          /**< Device structure. */
    struct pdm_client *client;  /**< PDM Client handle. */
};

/**
 * @def dev_to_pdm_device
 * @brief Converts a `device` pointer to a `pdm_device` pointer.
 *
 * This macro casts a `device` structure pointer to a `pdm_device` structure pointer.
 *
 * @param __dev Pointer to a `device` structure.
 * @return Pointer to a `pdm_device` structure.
 */
#define dev_to_pdm_device(__dev) container_of(__dev, struct pdm_device, dev)

/**
 * @brief pdm_device_get.
 *
 * @param pdmdev Pointer to the PDM device structure.
 * @return Pointer to the PDM device structure, or NULL on failure.
 */
static inline struct pdm_device *pdm_device_get(struct pdm_device *pdmdev)
{
	return (pdmdev && get_device(&pdmdev->dev)) ? pdmdev : NULL;
}

/**
 * @brief pdm_device_put.
 *
 * @param pdmdev Pointer to the PDM device structure.
 */
static inline void pdm_device_put(struct pdm_device *pdmdev)
{
	if (pdmdev)
		put_device(&pdmdev->dev);
}

/**
 * @brief Allocates a new PDM device structure.
 *
 * @return Pointer to the allocated PDM device structure, or NULL on failure.
 */
struct pdm_device *pdm_device_alloc(struct device *dev);

/**
 * @brief Frees a PDM device structure.
 *
 * @param pdmdev Pointer to the PDM device structure.
 */
void pdm_device_free(struct pdm_device *pdmdev);

/**
 * @brief Registers a PDM device with the system.
 *
 * @param pdmdev Pointer to the PDM device structure.
 * @return 0 on success, negative error code on failure.
 */
int pdm_device_register(struct pdm_device *pdmdev);

/**
 * @brief Unregisters a PDM device from the system.
 *
 * @param pdmdev Pointer to the PDM device structure.
 */
void pdm_device_unregister(struct pdm_device *pdmdev);

/**
 * @brief Initializes the PDM device module.
 *
 * @return 0 on success, negative error code on failure.
 */
int pdm_device_init(void);

/**
 * @brief Cleans up and unregisters the PDM device module.
 */
void pdm_device_exit(void);

#endif /* _PDM_DEVICE_H_ */
