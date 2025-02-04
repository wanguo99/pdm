#ifndef _PDM_SWITCH_IOCTL_H_
#define _PDM_SWITCH_IOCTL_H_

#define PDM_SWITCH_STATE_OFF	(0)
#define PDM_SWITCH_STATE_ON	(1)

#define PDM_SWITCH_IOC_MAGIC	's'

/* IOCTL commands */
#define PDM_SWITCH_SET_STATE		_IOW(PDM_SWITCH_IOC_MAGIC, 0, int *)
#define PDM_SWITCH_GET_STATE		_IOW(PDM_SWITCH_IOC_MAGIC, 1, int *)

#endif /* _PDM_SWITCH_IOCTL_H_ */
