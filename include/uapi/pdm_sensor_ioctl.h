#ifndef _PDM_SENSOR_IOCTL_H_
#define _PDM_SENSOR_IOCTL_H_

#define PDM_SENSOR_IOC_MAGIC	'n'

struct pdm_sensor_ioctl_data {
	unsigned char addr;
	unsigned char value;
};

/* IOCTL commands */
#define PDM_SENSOR_READ_REG	_IOW(PDM_SENSOR_IOC_MAGIC, 0, struct pdm_sensor_ioctl_data *)
#define PDM_SENSOR_WRITE_REG	_IOW(PDM_SENSOR_IOC_MAGIC, 1, struct pdm_sensor_ioctl_data *)

#endif /* _PDM_SENSOR_IOCTL_H_ */
