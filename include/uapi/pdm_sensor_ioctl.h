#ifndef _PDM_SENSOR_IOCTL_H_
#define _PDM_SENSOR_IOCTL_H_

#define PDM_SENSOR_IOC_MAGIC	'n'

struct pdm_sensor_ioctl_data {
	unsigned int type;
	unsigned int value;
};

/* IOCTL commands */
#define PDM_SENSOR_READ_REG	_IOW(PDM_SENSOR_IOC_MAGIC, 0, struct pdm_sensor_ioctl_data *)

#endif /* _PDM_SENSOR_IOCTL_H_ */
