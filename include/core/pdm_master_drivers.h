#ifndef _PDM_MASTER_DRIVERS_H_
#define _PDM_MASTER_DRIVERS_H_

/**
 * @brief 初始化 PDM 主控制器驱动
 *
 * 该函数用于初始化所有 PDM 主控制器驱动程序。
 *
 * @return 0 - 成功
 *         负值 - 失败
 */
int pdm_master_drivers_register(void);

/**
 * @brief 卸载 PDM 主控制器驱动
 *
 * 该函数用于卸载所有 PDM 主控制器驱动程序。
 */
void pdm_master_drivers_unregister(void);

/**
 * @brief 初始化 LED 主设备驱动
 *
 * 该函数用于初始化 LED 主设备驱动程序。
 *
 * @return 0 - 成功
 *         负值 - 失败
 */
int pdm_master_led_driver_init(void);

/**
 * @brief 退出 LED 主设备驱动
 *
 * 该函数用于退出 LED 主设备驱动程序，并释放相关资源。
 */
void pdm_master_led_driver_exit(void);

/**
 * @brief 初始化传感器主设备驱动
 *
 * 该函数用于初始化传感器主设备驱动程序。
 *
 * @return 0 - 成功
 *         负值 - 失败
 */
int pdm_master_sensor_driver_init(void);

/**
 * @brief 退出传感器主设备驱动
 *
 * 该函数用于退出传感器主设备驱动程序，并释放相关资源。
 */
void pdm_master_sensor_driver_exit(void);

/**
 * @brief 初始化模板主设备驱动
 *
 * 该函数用于初始化模板主设备驱动程序。
 *
 * @return 0 - 成功
 *         负值 - 失败
 */
int pdm_master_template_driver_init(void);

/**
 * @brief 退出模板主设备驱动
 *
 * 该函数用于退出模板主设备驱动程序，并释放相关资源。
 */
void pdm_master_template_driver_exit(void);

#endif /* _PDM_MASTER_DRIVERS_H_ */
