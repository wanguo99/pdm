#ifndef _PDM_H_
#define _PDM_H_

#include <linux/version.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/idr.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/mod_devicetable.h>

#include "osa_log.h"
#include "pdm_device.h"
#include "pdm_adapter.h"
#include "pdm_bus.h"
#include "pdm_client.h"

/**
 * @file pdm.h
 * @brief PDM 模块公共头文件
 *
 * 本文件定义了 PDM 模块的公共数据类型、结构体和函数声明。
 */

/**
 * @brief DebugFS 和 ProcFS 目录名称
 */
#define PDM_DEBUG_FS_DIR_NAME    "pdm"         /**< debugfs 和 procfs 目录名 */


#endif /* _PDM_H_ */
