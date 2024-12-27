#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/of_gpio.h>
#include <linux/pwm.h>

#include "pdm.h"
#include "pdm_device_priv.h"

/**
 * @brief Initializes GPIO settings for a PDM device.
 *
 * This function initializes the GPIO settings for the specified PDM device and sets up the operation functions.
 *
 * @param client Pointer to the PDM client structure.
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_device_gpio_setup(struct pdm_device *pdmdev)
{
    struct pdm_device_priv *pdmdev_priv;
    struct device_node *np;
    struct gpio_desc *gpiod;
    const char *default_state;
    bool is_active_low;
    int status;

    if (!pdmdev) {
        OSA_ERROR("Invalid pdmdev\n");
        return -EINVAL;
    }

    pdmdev_priv = pdm_device_get_private_data(pdmdev);
    if (!pdmdev_priv) {
        OSA_ERROR("Get PDM Device DrvData Failed\n");
        return -ENOMEM;
    }

    np = pdm_device_get_of_node(pdmdev);
    if (!np) {
        OSA_ERROR("No DT node found\n");
        return -EINVAL;
    }

    status = of_property_read_string(np, "default-state", &default_state);
    if (status) {
        OSA_INFO("No default-state property found, using defaults as off\n");
        default_state = "off";
    }

    gpiod = gpiod_get_index(pdmdev->dev.parent, NULL, 0, GPIOD_OUT_LOW);
    if (IS_ERR(gpiod)) {
        OSA_ERROR("Failed to get GPIO\n");
        return PTR_ERR(gpiod);
    }

    is_active_low = gpiod_is_active_low(gpiod);
    if (!strcmp(default_state, "on")) {
        gpiod_set_value_cansleep(gpiod, is_active_low ? 1 : 0);
    } else if (!strcmp(default_state, "off")) {
        gpiod_set_value_cansleep(gpiod, is_active_low ? 0 : 1);
    } else {
        OSA_INFO("Unknown default-state: %s, using off\n", default_state);
        gpiod_set_value_cansleep(gpiod, is_active_low ? 0 : 1);
    }

    pdmdev_priv->hardware.gpio.gpiod = gpiod;
    return 0;
}

static void pdm_device_gpio_cleanup(struct pdm_device *pdmdev)
{
    struct pdm_device_priv *pdmdev_priv;

    if (!pdmdev) {
        return;
    }

    pdmdev_priv = pdm_device_get_private_data(pdmdev);
    if (pdmdev_priv && !IS_ERR_OR_NULL(pdmdev_priv->hardware.gpio.gpiod)) {
        gpiod_put(pdmdev_priv->hardware.gpio.gpiod);
    }
}

static int pdm_device_pwm_setup(struct pdm_device *pdmdev)
{
    struct pdm_device_priv *pdmdev_priv;
    struct device_node *np;
    unsigned int default_level;
    struct pwm_device *pwmdev;
    int status;

    if (!pdmdev) {
        OSA_ERROR("Invalid pdmdev\n");
        return -EINVAL;
    }

    pdmdev_priv = pdm_device_get_private_data(pdmdev);
    if (!pdmdev_priv) {
        OSA_ERROR("Get PDM Device DrvData Failed\n");
        return -ENOMEM;
    }

    np = pdm_device_get_of_node(pdmdev);
    if (!np) {
        OSA_ERROR("No DT node found\n");
        return -EINVAL;
    }

    status = of_property_read_u32(np, "default-level", &default_level);
    if (status) {
        OSA_INFO("No default-state property found, using defaults as off\n");
        default_level = 0;
    }

    pwmdev = pwm_get(pdmdev->dev.parent, NULL);
    if (IS_ERR(pwmdev)) {
        OSA_ERROR("Failed to get PWM\n");
        return PTR_ERR(pwmdev);
    }

    pdmdev_priv->hardware.pwm.pwmdev = pwmdev;
    return 0;

}

static void pdm_device_pwm_cleanup(struct pdm_device *pdmdev)
{
    struct pdm_device_priv *pdmdev_priv;

    if (!pdmdev) {
        return;
    }

    pdmdev_priv = pdm_device_get_private_data(pdmdev);
    if (pdmdev_priv && !IS_ERR_OR_NULL(pdmdev_priv->hardware.pwm.pwmdev)) {
        pwm_put(pdmdev_priv->hardware.pwm.pwmdev);
    }
}

/**
 * @brief Probes the PLATFORM device and initializes the PDM device.
 *
 * This function is called when a PLATFORM device is detected, responsible for initializing and registering the PDM device.
 *
 * @param pdev Pointer to the PLATFORM device structure.
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_device_platform_probe(struct platform_device *pdev) {
    struct pdm_device *pdmdev;
    int status;

    pdmdev = pdm_device_alloc(&pdev->dev, sizeof(struct pdm_device_priv));
    if (IS_ERR(pdmdev)) {
        OSA_ERROR("Failed to allocate pdm_device\n");
        return PTR_ERR(pdmdev);
    }

    status = pdm_device_register(pdmdev);
    if (status) {
        OSA_ERROR("Failed to register pdm device, status=%d\n", status);
        goto err_pdmdev_free;
    }

    status = pdm_device_setup(pdmdev);
    if (status) {
        OSA_ERROR("Failed to setup pdm device, status=%d\n", status);
        goto err_pdmdev_unregister;
    }

    return 0;

err_pdmdev_unregister:
    pdm_device_unregister(pdmdev);
err_pdmdev_free:
    pdm_device_free(pdmdev);
    return status;
}

/**
 * @brief Removes the PLATFORM device and unregisters the PDM device.
 *
 * This function is called when a PLATFORM device is removed, responsible for unregistering and freeing the PDM device.
 *
 * @param pdev Pointer to the PLATFORM device structure.
 * @return Returns 0 on success; negative error code on failure.
 */
static void pdm_device_platform_real_remove(struct platform_device *pdev) {
    struct pdm_device *pdmdev = pdm_bus_find_device_by_parent(&pdev->dev);
    if (pdmdev) {
        pdm_device_cleanup(pdmdev);
        pdm_device_unregister(pdmdev);
        pdm_device_free(pdmdev);
    }
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 10, 0)
/**
 * @brief Compatibility remove function for older kernel versions.
 *
 * This function is used for Linux kernel versions below 6.10.0.
 *
 * @param pdev Pointer to the PLATFORM device structure.
 * @return Returns 0 on success; negative error code on failure.
 */
static int pdm_device_platform_remove(struct platform_device *pdev) {
    pdm_device_platform_real_remove(pdev);
    return 0;
}
#else
/**
 * @brief Removes the PLATFORM device and unregisters the PDM device.
 *
 * This function is called when a PLATFORM device is removed, responsible for unregistering and freeing the PDM device.
 *
 * @param pdev Pointer to the PLATFORM device structure.
 */
static void pdm_device_platform_remove(struct platform_device *pdev) {
    pdm_device_platform_real_remove(pdev);
}
#endif


/**
 * @brief Match data structure for initializing GPIO type devices.
 */
static const struct pdm_device_match_data pdm_device_gpio_match_data = {
    .setup = pdm_device_gpio_setup,
    .cleanup = pdm_device_gpio_cleanup,
};

static const struct pdm_device_match_data pdm_device_pwm_match_data = {
    .setup = pdm_device_pwm_setup,
    .cleanup = pdm_device_pwm_cleanup,
};

/**
 * @brief DEVICE_TREE match table.
 *
 * Defines the supported DEVICE_TREE compatibility strings.
 */
static const struct of_device_id pdm_device_platform_of_match[] = {
    { .compatible = "pdm,device-gpio",  .data = &pdm_device_gpio_match_data },
    { .compatible = "pdm,device-pwm",   .data = &pdm_device_pwm_match_data },
    { }
};
MODULE_DEVICE_TABLE(of, pdm_device_platform_of_match);

/**
 * @brief PLATFORM driver structure.
 *
 * Defines the basic information and operation functions of the PLATFORM driver.
 */
static struct platform_driver pdm_device_platform_driver = {
    .probe = pdm_device_platform_probe,
    .remove = pdm_device_platform_remove,
    .driver = {
        .name = "pdm-device-platform",
        .of_match_table = pdm_device_platform_of_match,
    },
};

/**
 * @brief Initializes the PLATFORM driver.
 *
 * Registers the PLATFORM driver with the system.
 *
 * @return Returns 0 on success; negative error code on failure.
 */
int pdm_device_platform_driver_init(void) {
    return platform_driver_register(&pdm_device_platform_driver);
}

/**
 * @brief Exits the PLATFORM driver.
 *
 * Unregisters the PLATFORM driver from the system.
 */
void pdm_device_platform_driver_exit(void) {
    platform_driver_unregister(&pdm_device_platform_driver);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Device PLATFORM Driver");
