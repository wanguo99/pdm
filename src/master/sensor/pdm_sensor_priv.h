#ifndef _PDM_SENSOR_PRIV_H_
#define _PDM_SENSOR_PRIV_H_

/**
 * @file pdm_master_sensor_priv.h
 * @brief PDM 传感器主设备私有数据结构体定义
 *
 * 本文件定义了 PDM 传感器主设备和设备的私有数据结构体及相关的操作函数。
 */

/**
 * @def PDM_MASTER_SENSOR_NAME
 * @brief 控制器名字
 */
#define PDM_MASTER_SENSOR_NAME "sensor"

/**
 * @struct pdm_device_sensor_operations
 * @brief PDM 传感器设备操作结构体
 *
 * 该结构体定义了 PDM 传感器设备的操作函数，包括读取电流和电压。
 */
struct pdm_device_sensor_operations {
    int (*get_current)(struct pdm_device *pdmdev, int *value);     /**< 读取电流 */
    int (*get_voltage)(struct pdm_device *pdmdev, int *value);     /**< 读取电压 */
};

/**
 * @struct pdm_master_sensor_priv
 * @brief PDM 传感器主设备私有数据结构体
 *
 * 该结构体用于存储 PDM 传感器主设备的私有数据。
 */
struct pdm_master_sensor_priv {
    // 可以根据需要添加主设备的私有数据
};

/**
 * @struct pdm_device_sensor_priv
 * @brief PDM 传感器设备私有数据结构体
 *
 * 该结构体用于存储 PDM 传感器设备的私有数据，包括操作函数指针。
 */
struct pdm_device_sensor_priv {
    struct pdm_device_sensor_operations *ops;  /**< 操作函数回调 */
};

/**
 * @brief 初始化 I2C 设置
 *
 * 该函数用于初始化指定 PDM 设备的 I2C 设置。
 *
 * @param pdmdev PDM 设备指针
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_master_sensor_i2c_setup(struct pdm_device *pdmdev);

/**
 * @brief 初始化 ADC 设置
 *
 * 该函数用于初始化指定 PDM 设备的 ADC 设置。
 *
 * @param pdmdev PDM 设备指针
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_master_sensor_adc_setup(struct pdm_device *pdmdev);

#endif /* _PDM_SENSOR_PRIV_H_ */
