#ifndef _PDM_ADAPTER_PRIV_H_
#define _PDM_ADAPTER_PRIV_H_

int pdm_switch_driver_init(void);
void pdm_switch_driver_exit(void);

int pdm_dimmer_driver_init(void);
void pdm_dimmer_driver_exit(void);

int pdm_nvmem_driver_init(void);
void pdm_nvmem_driver_exit(void);

#endif /* _PDM_ADAPTER_PRIV_H_ */
