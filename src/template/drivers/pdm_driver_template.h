#ifndef _PDM_TEMPLATE_DRIVER_H_
#define _PDM_TEMPLATE_DRIVER_H_

/**
 * @brief 初始化 I2C 驱动
 *
 * 该函数用于初始化 I2C 驱动，注册 I2C 驱动到系统。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_template_i2c_driver_init(void);

/**
 * @brief 退出 I2C 驱动
 *
 * 该函数用于退出 I2C 驱动，注销 I2C 驱动。
 */
void pdm_template_i2c_driver_exit(void);


/**
 * @brief 初始化 PLATFORM 驱动
 *
 * 该函数用于初始化 PLATFORM 驱动，注册 PLATFORM 驱动到系统。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_template_platform_driver_init(void);

/**
 * @brief 退出 PLATFORM 驱动
 *
 * 该函数用于退出 PLATFORM 驱动，注销 PLATFORM 驱动。
 */
 void pdm_template_platform_driver_exit(void);

/**
 * @brief 初始化 SPI 驱动
 *
 * 该函数用于初始化 SPI 驱动，注册 SPI 驱动到系统。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_template_spi_driver_init(void);

/**
 * @brief 退出 SPI 驱动
 *
 * 该函数用于退出 SPI 驱动，注销 SPI 驱动。
 */
void pdm_template_spi_driver_exit(void);

#endif /* _PDM_TEMPLATE_DRIVER_H_ */
