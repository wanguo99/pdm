#ifndef _PDM_MASTER_LED_H_
#define _PDM_MASTER_LED_H_

/**
 * @brief 初始化 PDM 模板主设备
 *
 * 该函数用于初始化 PDM 模板主设备。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_master_led_driver_init(void);

/**
 * @brief 退出 PDM 模板主设备
 *
 * 该函数用于退出 PDM 模板主设备，释放相关资源。
 */
void pdm_master_led_driver_exit(void);

#endif /* _PDM_MASTER_LED_H_ */
