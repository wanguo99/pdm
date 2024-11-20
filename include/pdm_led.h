#ifndef _PDM_LED_H_
#define _PDM_LED_H_

#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/pwm.h>
#include "pdm.h"

struct pdm_led_device {
    struct pdm_device *pdm_dev;
    union {
        struct gpio_desc *gpio;
        struct pwm_device *pwm;
    } control;
    // Add any LED-specific data here
};

// Device allocation and free functions
int pdm_led_device_alloc(struct pdm_led_device **led_dev);
void pdm_led_device_free(struct pdm_led_device *led_dev);

// Device registration and unregistration functions
int pdm_led_device_register(struct pdm_led_device *led_dev);
void pdm_led_device_unregister(struct pdm_led_device *led_dev);

// Master initialization and exit functions
int pdm_led_master_init(void);
void pdm_led_master_exit(void);

// Driver initialization and exit functions (if needed)
int pdm_led_gpio_driver_init(void);
void pdm_led_gpio_driver_exit(void);

#endif /* _PDM_LED_H_ */
