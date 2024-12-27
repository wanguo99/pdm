#ifndef _PDM_NVMEM_IOCTL_H_
#define _PDM_NVMEM_IOCTL_H_

#define PDM_NVMEM_IOC_MAGIC       'n'

struct pdm_nvmem_ioctl_data {
    unsigned char addr;
    unsigned char value;
};

/* IOCTL commands */
#define PDM_NVMEM_READ_REG             _IOW(PDM_NVMEM_IOC_MAGIC, 0, struct pdm_nvmem_ioctl_data *)
#define PDM_NVMEM_WRITE_REG            _IOW(PDM_NVMEM_IOC_MAGIC, 1, struct pdm_nvmem_ioctl_data *)

#endif /* _PDM_NVMEM_IOCTL_H_ */
