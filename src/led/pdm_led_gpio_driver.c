#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/version.h>

#include "pdm.h"
#include "pdm_led.h"

static int pdm_led_gpio_probe(struct platform_device *pdev) {
    struct pdm_led_device *led_dev;
    int ret;
    int gpio_num;

    printk(KERN_INFO "LED GPIO Device probed\n");

    ret = pdm_led_device_alloc(&led_dev);
    if (ret) {
        return ret;
    }

    // Get the GPIO number from the platform data or device tree
    gpio_num = of_get_named_gpio(pdev->dev.of_node, "led-gpio", 0);
    if (!gpio_is_valid(gpio_num)) {
        dev_err(&pdev->dev, "Invalid GPIO number\n");
        pdm_led_device_free(led_dev);
        return -ENODEV;
    }

    // Request the GPIO
    ret = devm_gpio_request_one(&pdev->dev, gpio_num, GPIOF_OUT_INIT_LOW, "pdm_led");
    if (ret) {
        dev_err(&pdev->dev, "Failed to request GPIO\n");
        pdm_led_device_free(led_dev);
        return ret;
    }

    // Set the GPIO in the LED device structure
    led_dev->control.gpio = gpio_to_desc(gpio_num);

    // Register the device
    ret = pdm_led_device_register(led_dev);
    if (ret) {
        pdm_led_device_free(led_dev);
        return ret;
    }

    platform_set_drvdata(pdev, led_dev);

    return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 10, 0)
static int pdm_led_gpio_remove(struct platform_device *pdev) {
#else
static void pdm_led_gpio_remove(struct platform_device *pdev) {
#endif
    struct pdm_led_device *led_dev = platform_get_drvdata(pdev);

    printk(KERN_INFO "LED GPIO Device removed\n");

    pdm_led_device_unregister(led_dev);
    pdm_led_device_free(led_dev);

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 10, 0)
    return 0;
#endif
}


#if 0
// dts 配置示意:
/ {
    led_gpio: led_gpio {
        compatible = "pdm,led-gpio";
        led-gpio = <&gpio1 17 0>; // 假设GPIO 17是LED的引脚
    };
};
#endif

static const struct of_device_id pdm_led_gpio_of_match[] = {
    { .compatible = "pdm,led-gpio", },
    { /* end of list */ }
};
MODULE_DEVICE_TABLE(of, pdm_led_gpio_of_match);

static struct platform_driver pdm_led_gpio_driver = {
    .driver = {
        .name = "pdm_led_gpio",
        .owner = THIS_MODULE,
        .of_match_table = pdm_led_gpio_of_match,
    },
    .probe = pdm_led_gpio_probe,
    .remove = pdm_led_gpio_remove,
};

int pdm_led_gpio_driver_init(void) {
    int ret;

    printk(KERN_INFO "LED GPIO Driver initialized\n");

    ret = platform_driver_register(&pdm_led_gpio_driver);
    if (ret) {
        printk(KERN_ERR "Failed to register LED GPIO driver\n");
    }

    return ret;
}

void pdm_led_gpio_driver_exit(void) {
    printk(KERN_INFO "LED GPIO Driver exited\n");

    platform_driver_unregister(&pdm_led_gpio_driver);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wanguo");
MODULE_DESCRIPTION("PDM LED GPIO Driver.");
