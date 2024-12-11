#ifndef _PDM_SENSOR_IOCTL_H_
#define _PDM_SENSOR_IOCTL_H_

/**
 * @file pdm_master_sensor_ioctl.h
 * @brief PDM 传感器 IOCTL 命令定义
 *
 * 本文件定义了 PDM 传感器设备的 IOCTL 命令。
 */

/**
 * @def PDM_MASTER_SENSOR_STATE_OFF
 * @brief 传感器关闭状态
 */
#define PDM_MASTER_SENSOR_STATE_OFF (0)

/**
 * @def PDM_MASTER_SENSOR_STATE_ON
 * @brief 传感器开启状态
 */
#define PDM_MASTER_SENSOR_STATE_ON (1)

/**
 * @def PDM_MASTER_SENSOR_IOC_MAGIC
 * @brief IOCTL 命令的魔术数
 */
#define PDM_MASTER_SENSOR_IOC_MAGIC 's'

/**
 * @def PDM_MASTER_SENSOR_GET_VOLTAGE
 * @brief 获取电压值的 IOCTL 命令
 */
#define PDM_MASTER_SENSOR_GET_VOLTAGE _IOR(PDM_MASTER_SENSOR_IOC_MAGIC, 0, int32_t)

/**
 * @def PDM_MASTER_SENSOR_GET_CURRENT
 * @brief 获取电流值的 IOCTL 命令
 */
#define PDM_MASTER_SENSOR_GET_CURRENT _IOR(PDM_MASTER_SENSOR_IOC_MAGIC, 1, int32_t)

#endif /* _PDM_SENSOR_IOCTL_H_ */
