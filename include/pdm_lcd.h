#ifndef _PDM_LCD_H_
#define _PDM_LCD_H_

#include <linux/device.h>
#include <linux/i2c.h>
#include "pdm.h"

struct pdm_lcd_device {
    struct pdm_device *pdm_dev;
    struct i2c_client *i2c_client;
    // Add any CPLD-specific data here
};

// Device allocation and free functions
int pdm_lcd_device_alloc(struct pdm_lcd_device **lcd_dev);
void pdm_lcd_device_free(struct pdm_lcd_device *lcd_dev);

// Device registration and unregistration functions
int pdm_lcd_device_register(struct pdm_lcd_device *lcd_dev);
void pdm_lcd_device_unregister(struct pdm_lcd_device *lcd_dev);

// Master initialization and exit functions
int pdm_lcd_master_init(void);
void pdm_lcd_master_exit(void);

// Driver initialization and exit functions
int pdm_lcd_i2c_driver_init(void);
void pdm_lcd_i2c_driver_exit(void);

#endif /* _PDM_LCD_H_ */
