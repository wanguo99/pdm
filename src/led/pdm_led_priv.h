#ifndef _PDM_LED_PRIV_H_
#define _PDM_LED_PRIV_H_

/**
 * @file pdm_led.h
 * @brief PDM 模板驱动接口
 *
 * 本文件定义了 PDM 模板驱动的结构体和相关函数，用于管理和操作 PDM 模板设备。
 */

#define PDM_LED_NAME        "led"      /* 控制器名字 */

/**
 * @struct pdm_led_operations
 * @brief PDM LED 设备操作结构体
 *
 * 该结构体定义了 PDM LED 设备的操作函数，包括开灯和关灯。
 */
struct pdm_led_operations {
    int (*set_state)(struct pdm_client *client, int state);      /**< 设置开关灯 */
};

/**
 * @struct pdm_led_priv
 * @brief PDM LED 设备私有数据结构体
 *
 * 该结构体用于存储 PDM LED 设备的私有数据，包括操作函数指针。
 */
struct pdm_led_priv {
    struct pdm_led_operations *ops;  /**< 操作函数回调 */
};

int pdm_led_gpio_setup(struct pdm_client *client);
int pdm_led_pwm_setup(struct pdm_client *client);

#endif /* _PDM_LED_PRIV_H_ */
