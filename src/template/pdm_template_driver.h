#ifndef _PDM_TEMPLATE_DRIVER_H_
#define _PDM_TEMPLATE_DRIVER_H_


int pdm_template_i2c_driver_init(void);
void pdm_template_i2c_driver_exit(void);


int pdm_template_platform_driver_init(void);
void pdm_template_platform_driver_exit(void);


int pdm_template_spi_driver_init(void);
void pdm_template_spi_driver_exit(void);

#endif /* _PDM_TEMPLATE_DRIVER_H_ */
