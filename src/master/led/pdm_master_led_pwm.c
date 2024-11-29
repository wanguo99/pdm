#include "pdm.h"
#include "pdm_master_led_priv.h"

static int pdm_master_led_pwm_turn_on(struct pdm_device *pdmdev)
{
    OSA_INFO("PWM LED Turn ON\n");
    return 0;
}

static int pdm_master_led_pwm_turn_off(struct pdm_device *pdmdev)
{
    OSA_INFO("PWM LED Turn OFF\n");
    return 0;
}

static struct pdm_device_led_operations pdm_device_led_ops_pwm = {
    .turn_on = pdm_master_led_pwm_turn_on,
    .turn_off = pdm_master_led_pwm_turn_off,
};

int pdm_master_led_pwm_setup(struct pdm_device *pdmdev)
{
    struct pdm_device_led_priv *data;
    int status;

    OSA_INFO("pdm_master_led_pwm_setup\n");
    data = (struct pdm_device_led_priv *)pdm_device_devdata_get(pdmdev);
    if (!data)
    {
        OSA_ERROR("Get Device Private Data Failed, status=%d.\n", status);
        return -ENOMEM;
    }
    data->ops = &pdm_device_led_ops_pwm;
    return 0;
}
