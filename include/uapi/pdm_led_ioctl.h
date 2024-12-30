#ifndef _PDM_LED_IOCTL_H_
#define _PDM_LED_IOCTL_H_

#define PDM_LED_STATE_OFF	(0)
#define PDM_LED_STATE_ON	(1)

#define PDM_LED_IOC_MAGIC	'l'

/* IOCTL commands */
#define PDM_LED_SET_STATE		_IOW(PDM_LED_IOC_MAGIC, 0, int *)
#define PDM_LED_GET_STATE		_IOW(PDM_LED_IOC_MAGIC, 1, int *)
#define PDM_LED_SET_BRIGHTNESS		_IOW(PDM_LED_IOC_MAGIC, 2, int *)
#define PDM_LED_GET_BRIGHTNESS		_IOW(PDM_LED_IOC_MAGIC, 3, int *)

#endif /* _PDM_LED_IOCTL_H_ */
