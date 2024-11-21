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

struct pdm_template_private_data {
    void *Data_1;
    void *Data_2;
    /* ----------------------- */
};

struct pdm_template_device {
    struct pdm_device *pdmdev;
    union {
        struct i2c_client *i2cdev;
        struct spi_device *spidev;
    }client;
    struct pdm_template_operations ops;
};

int pdm_template_master_add_device(struct pdm_template_device *template_dev);
int pdm_template_master_del_device(struct pdm_template_device *template_dev);


// Master initialization and exit functions
int pdm_template_master_init(void);
void pdm_template_master_exit(void);

// Driver initialization and exit functions
int pdm_template_i2c_driver_init(void);
void pdm_template_i2c_driver_exit(void);

#endif /* _PDM_TEMPLATE_H_ */
