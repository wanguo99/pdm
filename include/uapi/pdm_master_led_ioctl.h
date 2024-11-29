#ifndef _PDM_MASTER_LED_IOCTL_H_
#define _PDM_MASTER_LED_IOCTL_H_

#define PDM_MASTER_LED_STATE_OFF       (0)
#define PDM_MASTER_LED_STATE_ON        (1)

/* IOCTL commands */

#define PDM_MASTER_LED_IOC_MAGIC		'l'


struct pdm_master_led_ioctl_args {
    int index;
    int state;
};

#define PDM_MASTER_LED_SET_STATE		_IOW(PDM_MASTER_LED_IOC_MAGIC, 0, struct pdm_master_led_ioctl_args *)


#endif /* _PDM_MASTER_LED_IOCTL_H_ */
