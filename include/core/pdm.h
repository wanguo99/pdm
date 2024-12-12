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

#endif /* _PDM_H_ */
