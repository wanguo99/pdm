#ifndef _PDM_SENSOR_IOCTL_H_
#define _PDM_SENSOR_IOCTL_H_

#define PDM_SENSOR_IOC_MAGIC	's'

enum pdm_sensor_type {
	PDM_SENSOR_TYPE_NULL	= 0x00,
	PDM_SENSOR_TYPE_IR	= 0x01,
	PDM_SENSOR_TYPE_ALS	= 0x02,
	PDM_SENSOR_TYPE_PS	= 0x03,
	PDM_SENSOR_TYPE_INVALID	= 0xFFFF
};

struct pdm_sensor_ioctl_data {
	enum pdm_sensor_type type;
	unsigned int value;
};

/* IOCTL commands */
#define PDM_SENSOR_READ_REG	_IOW(PDM_SENSOR_IOC_MAGIC, 0, struct pdm_sensor_ioctl_data *)

#endif /* _PDM_SENSOR_IOCTL_H_ */
