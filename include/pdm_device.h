#ifndef _PDM_DEVICE_H_
#define _PDM_DEVICE_H_

#include <linux/i2c.h>
#include <linux/i3c/master.h>
#include <linux/spi/spi.h>

#define PDM_DEVICE_NAME_SIZE        (64)        // 定义设备名称的最大长度

typedef enum tagPDM_DEVICE_INTERFACE_TYPE
{
    PDM_DEVICE_INTERFACE_TYPE_UNDEFINED     = 0x00,
    PDM_DEVICE_INTERFACE_TYPE_I2C           = 0x02,
    PDM_DEVICE_INTERFACE_TYPE_I3C           = 0x03,
    PDM_DEVICE_INTERFACE_TYPE_SPI           = 0x04,
    PDM_DEVICE_INTERFACE_TYPE_GPIO          = 0x05,
    PDM_DEVICE_INTERFACE_TYPE_PWM           = 0x06,
    PDM_DEVICE_INTERFACE_TYPE_TTY           = 0x07,
    PDM_DEVICE_INTERFACE_TYPE_PLATFORM      = 0x08,
    PDM_DEVICE_INTERFACE_TYPE_INVALID       = 0xFF,
}PDM_DEVICE_INTERFACE_TYPE;

struct pdm_device_physical_info {
    int type;                              /**< 设备物理接口类型, PDM_DEVICE_INTERFACE_TYPE */
    void *device;                      /**< 指向实际的设备结构体 */
};

/**
 * @brief PDM 设备结构体
 */
struct pdm_device {
    int id;                                     /**< 设备ID */
    char compatible[PDM_DEVICE_NAME_SIZE];      /**< 设备兼容字符串 */
    struct device dev;                          /**< 设备结构体 */
    struct pdm_master *master;                  /**< 指向所属的PDM主控制器 */
    struct list_head entry;                     /**< 设备链表节点 */
    struct pdm_device_physical_info physical_info;     /**< 物理设备信息 */
};



/*
 * PDM device 相关函数
 */

/**
 * @brief 将 PDM 设备转换为实际的设备指针
 *
 * 该函数根据 PDM 设备的接口类型，返回相应的实际设备指针。
 *
 * @param pdmdev 指向 PDM 设备的指针
 * @return 成功返回实际的设备指针，失败返回 NULL
 */
struct device *pdm_device_to_physical_dev(struct pdm_device *pdmdev);

/**
 * @brief 将 device 转换为 pdm_device
 *
 * 该宏用于将 device 转换为 pdm_device。
 *
 * @param __dev device 结构体指针
 * @return pdm_device 结构体指针
 */
#define dev_to_pdm_device(__dev) container_of(__dev, struct pdm_device, dev)

/**
 * @brief 获取 PDM 设备的私有数据
 *
 * 该函数用于获取 PDM 设备的私有数据。
 *
 * @param pdmdev PDM 设备结构体指针
 * @return 私有数据指针
 */
void *pdm_device_devdata_get(struct pdm_device *pdmdev);

/**
 * @brief 设置 PDM 设备的私有数据
 *
 * 该函数用于设置 PDM 设备的私有数据。
 *
 * @param pdmdev PDM 设备结构体指针
 * @param data 私有数据指针
 */
void pdm_device_devdata_set(struct pdm_device *pdmdev, void *data);

/**
 * @brief 分配 PDM 设备结构体
 *
 * 该函数用于分配新的 PDM 设备结构体。
 *
 * @param data_size 私有数据区域的大小
 * @return 分配的 PDM 设备结构体指针，失败返回 NULL
 */
struct pdm_device *pdm_device_alloc(unsigned int data_size);

/**
 * @brief 释放 PDM 设备结构体
 *
 * 该函数用于释放 PDM 设备结构体。
 *
 * @param pdmdev PDM 设备结构体指针
 */
void pdm_device_free(struct pdm_device *pdmdev);

/**
 * @brief 查找与给定物理信息匹配的 PDM 设备
 *
 * 该函数用于查找与给定物理信息匹配的 PDM 设备。
 *
 * @param physical_info 要匹配的物理设备信息
 * @return 返回匹配的设备指针，如果没有找到则返回 NULL
 */
struct pdm_device *pdm_device_match_physical_info(struct pdm_device_physical_info *physical_info);

/**
 * @brief 注册 PDM 设备
 *
 * 该函数用于注册 PDM 设备。
 *
 * @param pdmdev PDM 设备结构体指针
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_device_register(struct pdm_device *pdmdev);

/**
 * @brief 注销 PDM 设备
 *
 * 该函数用于注销 PDM 设备。
 *
 * @param pdmdev PDM 设备结构体指针
 */
void pdm_device_unregister(struct pdm_device *pdmdev);

/**
 * pdm_device_init - 初始化PDM设备
 *
 * 返回值:
 * 0 - 成功
 * 负值 - 失败
 */
int pdm_device_init(void);

/**
 * pdm_device_exit - 卸载PDM设备
 */
void pdm_device_exit(void);



/**
 * @brief 初始化 I2C 驱动
 *
 * 该函数用于初始化 I2C 驱动，注册 I2C 驱动到系统。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_device_i2c_driver_init(void);

/**
 * @brief 退出 I2C 驱动
 *
 * 该函数用于退出 I2C 驱动，注销 I2C 驱动。
 */
void pdm_device_i2c_driver_exit(void);


/**
 * @brief 初始化 PLATFORM 驱动
 *
 * 该函数用于初始化 PLATFORM 驱动，注册 PLATFORM 驱动到系统。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_device_platform_driver_init(void);

/**
 * @brief 退出 PLATFORM 驱动
 *
 * 该函数用于退出 PLATFORM 驱动，注销 PLATFORM 驱动。
 */
 void pdm_device_platform_driver_exit(void);

/**
 * @brief 初始化 SPI 驱动
 *
 * 该函数用于初始化 SPI 驱动，注册 SPI 驱动到系统。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_device_spi_driver_init(void);

/**
 * @brief 退出 SPI 驱动
 *
 * 该函数用于退出 SPI 驱动，注销 SPI 驱动。
 */
void pdm_device_spi_driver_exit(void);

#endif /* _PDM_DEVICE_H_ */
