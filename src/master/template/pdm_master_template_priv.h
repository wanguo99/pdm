#ifndef _PDM_MASTER_TEMPLATE_PRIV_H_
#define _PDM_MASTER_TEMPLATE_PRIV_H_

/**
 * @file pdm_template.h
 * @brief PDM 模板驱动接口
 *
 * 本文件定义了 PDM 模板驱动的结构体和相关函数，用于管理和操作 PDM 模板设备。
 */

#define PDM_MASTER_TEMPLATE_NAME        "template"      /* 控制器名字 */

/**
 * @struct pdm_template_operations
 * @brief PDM 模板操作结构体
 *
 * 该结构体定义了 PDM 模板设备的操作函数，包括读取和写入寄存器。
 */
struct pdm_device_template_operations {
    int (*read_reg)(int addr, int *value);  /**< 读取寄存器的函数 */
    int (*write_reg)(int addr, int value);  /**< 写入寄存器的函数 */
};

/**
 * @struct pdm_template_master_priv
 * @brief PDM 模板主设备私有数据结构体
 *
 * 该结构体用于存储 PDM 模板主设备的私有数据。
 */
struct pdm_master_template_priv {
    // 可以根据需要添加master的私有数据
};

/**
 * @struct pdm_template_device_priv
 * @brief PDM 模板设备私有数据结构体
 *
 * 该结构体用于存储 PDM 模板设备的私有数据，包括操作函数指针。
 */
struct pdm_device_template_priv {
    // 可以根据需要添加device的私有数据
    struct pdm_device_template_operations *ops;  /**< 操作函数回调 */
};


#endif /* _PDM_MASTER_TEMPLATE_PRIV_H_ */
