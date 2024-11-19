#ifndef _PDM_CPLD_H_
#define _PDM_CPLD_H_

#include <linux/device.h>
#include <linux/i2c.h>
#include "pdm.h"


struct cpld_ops {
    int (*read)(int addr, int *value);
    int (*write)(int addr, int value);
};


struct pdm_cpld_master {
    struct pdm_master master;

    // Add any CPLD-specific data here
    union {
        struct spi_device *spi_device;
        struct i2c_client *i2c_client;
    } client;

    struct cpld_ops *ops;
};

struct pdm_cpld_device {
    struct pdm_device *pdm_dev;

};

// Device allocation and free functions
int pdm_cpld_device_alloc(struct pdm_cpld_device **cpld_dev);
void pdm_cpld_device_free(struct pdm_cpld_device *cpld_dev);

// Device registration and unregistration functions
int pdm_cpld_device_register(struct pdm_cpld_device *cpld_dev);
void pdm_cpld_device_unregister(struct pdm_cpld_device *cpld_dev);

// Master initialization and exit functions
int pdm_cpld_master_init(void);
void pdm_cpld_master_exit(void);

// Driver initialization and exit functions
int pdm_cpld_i2c_driver_init(void);
void pdm_cpld_i2c_driver_exit(void);

#endif /* _PDM_CPLD_H_ */
