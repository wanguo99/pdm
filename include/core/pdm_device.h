#ifndef _PDM_DEVICE_H_
#define _PDM_DEVICE_H_

#include <linux/i2c.h>
#include <linux/i3c/master.h>
#include <linux/spi/spi.h>
#include <linux/platform_device.h>


/**
 * @brief PDM 设备类型
 *
 * 该变量定义了 PDM 设备的类型。
 */
extern const struct device_type pdm_device_type;

/**
 * @brief 设备名称的最大长度
 */
#define PDM_DEVICE_NAME_SIZE (64)

/**
 * @brief PDM 设备结构体
 *
 * 该结构体定义了 PDM 设备的基本信息。
 */
struct pdm_device {
    int id;                                     /**< 总线分配的设备ID */
    struct device dev;                          /**< 设备结构体 */
    struct pdm_client *client;                  /**< PDM Client句柄 */
};


/**
 * @brief 将 device 转换为 pdm_device
 *
 * 该宏用于将 `device` 结构体指针转换为 `pdm_device` 结构体指针。
 *
 * @param __dev `device` 结构体指针
 * @return `pdm_device` 结构体指针
 */
#define dev_to_pdm_device(__dev) container_of(__dev, struct pdm_device, dev)

/**
 * @brief 分配 PDM 设备结构体
 *
 * 该函数用于分配新的 PDM 设备结构体。
 *
 * @return 分配的 PDM 设备结构体指针，失败返回 NULL
 */
struct pdm_device *pdm_device_alloc(void);

/**
 * @brief 释放 PDM 设备结构体
 *
 * 该函数用于释放 PDM 设备结构体。
 *
 * @param pdmdev PDM 设备结构体指针
 */
void pdm_device_free(struct pdm_device *pdmdev);

/**
 * @brief 注册 PDM 设备
 *
 * 该函数用于注册 PDM 设备。
 *
 * @param pdmdev PDM 设备结构体指针
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_device_register(struct pdm_device *pdmdev);

/**
 * @brief 注销 PDM 设备
 *
 * 该函数用于注销 PDM 设备。
 *
 * @param pdmdev PDM 设备结构体指针
 */
void pdm_device_unregister(struct pdm_device *pdmdev);

/**
 * @brief 初始化 PDM 设备模块
 *
 * 该函数用于初始化 PDM 设备模块。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_device_init(void);

/**
 * @brief 卸载 PDM 设备模块
 *
 * 该函数用于卸载 PDM 设备模块。
 */
void pdm_device_exit(void);

#endif /* _PDM_DEVICE_H_ */
