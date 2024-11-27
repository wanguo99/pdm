#include "pdm.h"
#include "pdm_master_led.h"
#include "pdm_master_led_priv.h"

static int pdm_master_led_gpio_turn_on(void)
{
    OSA_INFO("GPIO LED Turn ON\n");
    return 0;
}

static int pdm_master_led_gpio_turn_off(void)
{
    OSA_INFO("GPIO LED Turn OFF\n");
    return 0;
}

static struct pdm_device_led_operations pdm_device_led_ops_gpio = {
    .turn_on = pdm_master_led_gpio_turn_on,
    .turn_off = pdm_master_led_gpio_turn_off,
};

int pdm_master_led_gpio_init(struct pdm_device *pdmdev)
{
    struct pdm_device_led_priv *data;
    int status;

    data = (struct pdm_device_led_priv *)pdm_device_devdata_get(pdmdev);
    if (!data)
    {
        OSA_ERROR("Get Device Private Data Failed, status=%d.\n", status);
        return -ENOMEM;
    }
    data->ops = &pdm_device_led_ops_gpio;
    return 0;
}

