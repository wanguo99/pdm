#ifndef _PDM_LED_H_
#define _PDM_LED_H_

/**
 * @file pdm_led.h
 * @brief PDM 模板驱动接口
 *
 * 本文件定义了 PDM 模板驱动的结构体和相关函数，用于管理和操作 PDM 模板设备。
 */

#define PDM_MASTER_LED_NAME        "led"      /* 控制器名字 */

/**
 * @struct pdm_led_operations
 * @brief PDM 模板操作结构体
 *
 * 该结构体定义了 PDM 模板设备的操作函数，包括读取和写入寄存器。
 */
struct pdm_device_led_operations {
    int (*turn_on)(int index);      /**< 开灯 */
    int (*turn_off)(int index);     /**< 关灯 */
};

/**
 * @struct pdm_led_master_priv
 * @brief PDM 模板主设备私有数据结构体
 *
 * 该结构体用于存储 PDM 模板主设备的私有数据。
 */
struct pdm_master_led_priv {
    // 可以根据需要添加master的私有数据
};

/**
 * @struct pdm_led_device_priv
 * @brief PDM 模板设备私有数据结构体
 *
 * 该结构体用于存储 PDM 模板设备的私有数据，包括操作函数指针。
 */
struct pdm_device_led_priv {
    // 可以根据需要添加device的私有数据
    struct pdm_device_led_operations *ops;  /**< 操作函数回调 */
};

/**
 * @brief 注册 PDM 设备
 *
 * 该函数用于注册 PDM 设备，将其添加到设备管理器中。
 *
 * @param pdmdev PDM 设备指针
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_led_master_register_device(struct pdm_device *pdmdev);

/**
 * @brief 注销 PDM 设备
 *
 * 该函数用于注销 PDM 设备，将其从设备管理器中移除。
 *
 * @param pdmdev PDM 设备指针
 */
void pdm_led_master_unregister_device(struct pdm_device *pdmdev);

/**
 * @brief 初始化 PDM 模板主设备
 *
 * 该函数用于初始化 PDM 模板主设备。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_master_led_driver_init(void);

/**
 * @brief 退出 PDM 模板主设备
 *
 * 该函数用于退出 PDM 模板主设备，释放相关资源。
 */
void pdm_master_led_driver_exit(void);

#endif /* _PDM_LED_H_ */
