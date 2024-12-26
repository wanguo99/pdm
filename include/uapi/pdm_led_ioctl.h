#ifndef _PDM_LED_IOCTL_H_
#define _PDM_LED_IOCTL_H_

#define PDM_LED_STATE_OFF       (0)
#define PDM_LED_STATE_ON        (1)

/* IOCTL commands */

#define PDM_LED_IOC_MAGIC		'l'


struct pdm_led_ioctl_args {
    bool state;
};

#define PDM_LED_SET_STATE		_IOW(PDM_LED_IOC_MAGIC, 0, struct pdm_led_ioctl_args *)


#endif /* _PDM_LED_IOCTL_H_ */
