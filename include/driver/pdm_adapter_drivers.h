#ifndef _PDM_ADAPTER_DRIVERS_H_
#define _PDM_ADAPTER_DRIVERS_H_

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
int pdm_led_driver_init(void);

/**
 * @brief 退出 LED 主设备驱动
 *
 * 该函数用于退出 LED 主设备驱动程序，并释放相关资源。
 */
void pdm_led_driver_exit(void);

#endif /* _PDM_ADAPTER_DRIVERS_H_ */
