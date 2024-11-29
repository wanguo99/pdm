#ifndef _PDM_DEVICE_DRIVERS_H_
#define _PDM_DEVICE_DRIVERS_H_


/**
 * pdm_device_init - 初始化PDM设备
 *
 * 返回值:
 * 0 - 成功
 * 负值 - 失败
 */
int pdm_device_drivers_register(void);

/**
 * pdm_device_exit - 卸载PDM设备
 */
void pdm_device_drivers_unregister(void);

/**
 * @brief 初始化 I2C 驱动
 *
 * 该函数用于初始化 I2C 驱动，注册 I2C 驱动到系统。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_device_i2c_driver_init(void);

/**
 * @brief 退出 I2C 驱动
 *
 * 该函数用于退出 I2C 驱动，注销 I2C 驱动。
 */
void pdm_device_i2c_driver_exit(void);

/**
 * @brief 初始化 PLATFORM 驱动
 *
 * 该函数用于初始化 PLATFORM 驱动，注册 PLATFORM 驱动到系统。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_device_platform_driver_init(void);

/**
 * @brief 退出 PLATFORM 驱动
 *
 * 该函数用于退出 PLATFORM 驱动，注销 PLATFORM 驱动。
 */
 void pdm_device_platform_driver_exit(void);

/**
 * @brief 初始化 SPI 驱动
 *
 * 该函数用于初始化 SPI 驱动，注册 SPI 驱动到系统。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_device_spi_driver_init(void);

/**
 * @brief 退出 SPI 驱动
 *
 * 该函数用于退出 SPI 驱动，注销 SPI 驱动。
 */
void pdm_device_spi_driver_exit(void);

#endif /* _PDM_DEVICE_DRIVERS_H_ */
