#ifndef _PDM_H_
#define _PDM_H_

/**
 * @file pdm.h
 * @brief PDM Module Public Header File.
 *
 * This file defines public data types, structures, and function declarations for the PDM module.
 */

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

#ifndef KBUILD_MODNAME
#define KBUILD_MODNAME
#endif

#ifndef MODULE_BUILD_TIME
#define MODULE_BUILD_TIME
#endif

#ifndef MODULE_VERSIONS
#define MODULE_VERSIONS
#endif

#define PDM_MODULE_NAME         KBUILD_MODNAME
#define PDM_MODULE_BUILD_TIME   MODULE_BUILD_TIME
#define PDM_MODULE_VERSIONS     MODULE_VERSIONS

#endif /* _PDM_H_ */
