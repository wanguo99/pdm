#include "pdm.h"
#include "pdm_master_sensor_priv.h"

static int pdm_master_sensor_adc_get_voltage(struct pdm_device *pdmdev, int *value)
{
    OSA_INFO("pdm_master_sensor_adc_get_voltage\n");
    return 0;
}

static int pdm_master_sensor_adc_get_current(struct pdm_device *pdmdev, int *value)
{
    OSA_INFO("pdm_master_sensor_adc_get_current\n");
    return 0;
}

static struct pdm_device_sensor_operations pdm_device_sensor_ops_adc = {
    .get_current = pdm_master_sensor_adc_get_current,
    .get_voltage = pdm_master_sensor_adc_get_voltage,
};

int pdm_master_sensor_adc_setup(struct pdm_device *pdmdev)
{
    struct pdm_device_sensor_priv *data;
    int status;

    OSA_INFO("pdm_master_sensor_adc_setup\n");
    data = (struct pdm_device_sensor_priv *)pdm_device_devdata_get(pdmdev);
    if (!data)
    {
        OSA_ERROR("Get Device Private Data Faisensor, status=%d.\n", status);
        return -ENOMEM;
    }
    data->ops = &pdm_device_sensor_ops_adc;
    return 0;
}
