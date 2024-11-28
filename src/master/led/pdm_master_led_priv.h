#ifndef _PDM_MASTER_LED_PRIV_H_
#define _PDM_MASTER_LED_PRIV_H_


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
    int (*turn_on)(struct pdm_device *pdmdev);      /**< 开灯 */
    int (*turn_off)(struct pdm_device *pdmdev);     /**< 关灯 */
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
    int index;
    struct pdm_device_led_operations *ops;  /**< 操作函数回调 */
};


int pdm_master_led_gpio_init(struct pdm_device *pdmdev);
int pdm_master_led_pwm_init(struct pdm_device *pdmdev);

#endif /* _PDM_MASTER_LED_PRIV_H_ */
