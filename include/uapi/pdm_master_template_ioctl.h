#ifndef _PDM_MASTER_TEMPLATE_IOCTL_H_
#define _PDM_MASTER_TEMPLATE_IOCTL_H_

/**
 * @file pdm_master_template_ioctl.h
 * @brief PDM 模板 IOCTL 命令定义
 *
 * 本文件定义了 PDM 模板设备的 IOCTL 命令。
 */

/**
 * @def PDM_MASTER_TEMPLATE_STATE_OFF
 * @brief 模板关闭状态
 */
#define PDM_MASTER_TEMPLATE_STATE_OFF 0

/**
 * @def PDM_MASTER_TEMPLATE_STATE_ON
 * @brief 模板开启状态
 */
#define PDM_MASTER_TEMPLATE_STATE_ON 1

/**
 * @def PDM_MASTER_TEMPLATE_IOC_MAGIC
 * @brief IOCTL 命令的魔术数
 */
#define PDM_MASTER_TEMPLATE_IOC_MAGIC 't'

/**
 * @def PDM_MASTER_TEMPLATE_GET_VOLTAGE
 * @brief 获取电压值的 IOCTL 命令
 */
#define PDM_MASTER_TEMPLATE_GET_VOLTAGE _IOR(PDM_MASTER_TEMPLATE_IOC_MAGIC, 0, int32_t)

/**
 * @def PDM_MASTER_TEMPLATE_GET_CURRENT
 * @brief 获取电流值的 IOCTL 命令
 */
#define PDM_MASTER_TEMPLATE_GET_CURRENT _IOR(PDM_MASTER_TEMPLATE_IOC_MAGIC, 1, int32_t)

#endif /* _PDM_MASTER_TEMPLATE_IOCTL_H_ */
