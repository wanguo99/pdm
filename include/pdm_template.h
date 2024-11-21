#ifndef _PDM_TEMPLATE_H_
#define _PDM_TEMPLATE_H_

#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/spi/spi.h>
#include "pdm.h"

struct pdm_template_operations {
    int (*read_reg)(int addr, int *value);
    int (*write_reg)(int addr, int value);
};

struct pdm_template_master_priv {
};

struct pdm_template_device_priv {

    struct pdm_template_operations *ops;
};

struct pdm_device *pdm_template_master_get_pdmdev_of_real_device(void *real_device);
int pdm_template_master_add_device(struct pdm_device *pdmdev);
int pdm_template_master_del_device(struct pdm_device *pdmdev);


// Master initialization and exit functions
int pdm_template_master_init(void);
void pdm_template_master_exit(void);

// Driver initialization and exit functions
int pdm_template_i2c_driver_init(void);
void pdm_template_i2c_driver_exit(void);

#endif /* _PDM_TEMPLATE_H_ */
