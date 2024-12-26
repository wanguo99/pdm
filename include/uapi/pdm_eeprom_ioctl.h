#ifndef _PDM_EEPROM_IOCTL_H_
#define _PDM_EEPROM_IOCTL_H_

#define PDM_EEPROM_IOC_MAGIC       'e'

struct pdm_eeprom_ioctl_data {
    unsigned char addr;
    unsigned char value;
};

/* IOCTL commands */
#define PDM_EEPROM_READ_REG             _IOW(PDM_EEPROM_IOC_MAGIC, 0, struct pdm_eeprom_ioctl_data *)
#define PDM_EEPROM_WRITE_REG            _IOW(PDM_EEPROM_IOC_MAGIC, 1, struct pdm_eeprom_ioctl_data *)

#endif /* _PDM_EEPROM_IOCTL_H_ */
