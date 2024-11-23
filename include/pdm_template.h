#ifndef _PDM_TEMPLATE_H_
#define _PDM_TEMPLATE_H_

#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/spi/spi.h>
#include "pdm.h"

/**
 * @file pdm_template.h
 * @brief PDM 模板驱动接口
 *
 * 本文件定义了 PDM 模板驱动的结构体和相关函数，用于管理和操作 PDM 模板设备。
 */

/**
 * @struct pdm_template_operations
 * @brief PDM 模板操作结构体
 *
 * 该结构体定义了 PDM 模板设备的操作函数，包括读取和写入寄存器。
 */
struct pdm_template_operations {
    int (*read_reg)(int addr, int *value);  /**< 读取寄存器的函数 */
    int (*write_reg)(int addr, int value);  /**< 写入寄存器的函数 */
};

/**
 * @struct pdm_template_master_priv
 * @brief PDM 模板主设备私有数据结构体
 *
 * 该结构体用于存储 PDM 模板主设备的私有数据。
 */
struct pdm_template_master_priv {
    // 可以根据需要添加主设备的私有数据
};

/**
 * @struct pdm_template_device_priv
 * @brief PDM 模板设备私有数据结构体
 *
 * 该结构体用于存储 PDM 模板设备的私有数据，包括操作函数指针。
 */
struct pdm_template_device_priv {
    struct pdm_template_operations *ops;  /**< 操作函数指针 */
};

/**
 * @brief 查找 PDM 设备
 *
 * 该函数用于查找与给定实际设备对应的 PDM 设备。
 *
 * @param real_device 实际设备指针
 * @return 成功返回 PDM 设备指针，失败返回 NULL
 */
struct pdm_device *pdm_template_master_find_pdmdev(void *real_device);

/**
 * @brief 注册 PDM 设备
 *
 * 该函数用于注册 PDM 设备，将其添加到设备管理器中。
 *
 * @param pdmdev PDM 设备指针
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_template_master_register_device(struct pdm_device *pdmdev);

/**
 * @brief 注销 PDM 设备
 *
 * 该函数用于注销 PDM 设备，将其从设备管理器中移除。
 *
 * @param pdmdev PDM 设备指针
 */
void pdm_template_master_unregister_device(struct pdm_device *pdmdev);

/**
 * @brief 初始化 PDM 模板主设备
 *
 * 该函数用于初始化 PDM 模板主设备。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_template_master_init(void);

/**
 * @brief 退出 PDM 模板主设备
 *
 * 该函数用于退出 PDM 模板主设备，释放相关资源。
 */
void pdm_template_master_exit(void);

/**
 * @brief 初始化 PDM 模板 I2C 驱动
 *
 * 该函数用于初始化 PDM 模板 I2C 驱动。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_template_i2c_driver_init(void);

/**
 * @brief 退出 PDM 模板 I2C 驱动
 *
 * 该函数用于退出 PDM 模板 I2C 驱动，释放相关资源。
 */
void pdm_template_i2c_driver_exit(void);

/**
 * @brief 初始化 PDM 模板 GPIO 驱动
 *
 * 该函数用于初始化 PDM 模板 GPIO 驱动。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_template_gpio_driver_init(void);

/**
 * @brief 退出 PDM 模板 GPIO 驱动
 *
 * 该函数用于退出 PDM 模板 GPIO 驱动，释放相关资源。
 */
void pdm_template_gpio_driver_exit(void);

#endif /* _PDM_TEMPLATE_H_ */
