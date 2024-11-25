#ifndef _PDM_TEMPLATE_DRIVER_H_
#define _PDM_TEMPLATE_DRIVER_H_

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
 * @brief 初始化 PDM 模板 PLATFORM 驱动
 *
 * 该函数用于初始化 PDM 模板 PLATFORM 驱动。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_template_platform_driver_init(void);

/**
 * @brief 退出 PDM 模板 PLATFORM 驱动
 *
 * 该函数用于退出 PDM 模板 PLATFORM 驱动，释放相关资源。
 */
void pdm_template_platform_driver_exit(void);

/**
 * @brief 初始化 PDM 模板 SPI 驱动
 *
 * 该函数用于初始化 PDM 模板 SPI 驱动。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_template_spi_driver_init(void);

/**
 * @brief 退出 PDM 模板 SPI 驱动
 *
 * 该函数用于退出 PDM 模板 SPI 驱动，释放相关资源。
 */
void pdm_template_spi_driver_exit(void);

#endif /* _PDM_TEMPLATE_DRIVER_H_ */
