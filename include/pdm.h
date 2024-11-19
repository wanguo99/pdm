#ifndef _PDM_H_
#define _PDM_H_

#include <linux/device.h>
#include <linux/idr.h>
#include <linux/mod_devicetable.h>


/*                                                                              */
/*                                公共数据类型声明                                      */
/*                                                                              */

extern const  struct device_type pdm_device_type;
extern const  struct bus_type pdm_bus_type;

/*                                                                              */
/*                                公共数据类型定义                                      */
/*                                                                              */

struct pdm_device {
    int id;
    const char *name;
    struct pdm_master *master;
    struct device dev;
    struct list_head node;
};

struct pdm_master {
    const char *name;
    struct device dev;
    struct list_head slaves;
    struct list_head node;
    struct idr device_idr;
};

#define PDM_NAME_SIZE	20

struct pdm_device_id {
	char name[PDM_NAME_SIZE];
	kernel_ulong_t driver_data;
};

struct pdm_driver {
    struct device_driver driver;
    const struct pdm_device_id *id_table;
    int (*probe)(struct pdm_device *dev);
    void (*remove)(struct pdm_device *dev);
};


struct pdm_bus {
    int id;

    struct bus_type bus_type;
    struct device *dev;
    struct list_head masters;
};


/*                                                                              */
/*                                    函数声明                                      */
/*                                                                              */

static inline struct pdm_driver *drv_to_pdmdrv(struct device_driver *drv)
{
	return container_of(drv, struct pdm_driver, driver);
}

struct device *pdmdev_to_dev(struct pdm_device *pdmdev);

/**
 * dev_to_pdmdev() - Returns the I3C device containing @dev
 * @__dev: device object
 *
 * Return: a pointer to an PDM device object.
 */
#define dev_to_pdmdev(__dev)	container_of_const(__dev, struct pdm_device, dev)




// driver
int pdm_driver_register(struct pdm_driver *driver);
void pdm_driver_unregister(struct pdm_driver *driver);

// device
int pdm_device_alloc(struct pdm_device **device, struct pdm_master *master);
void pdm_device_free(struct pdm_device *device);
int pdm_device_register(struct pdm_device *device);
void pdm_device_unregister(struct pdm_device *device);
const struct pdm_device_id *pdm_match_id(const struct pdm_device_id *id, struct pdm_device *pdmdev);

// master
int pdm_master_alloc(struct pdm_master **master, const char *name);
void pdm_master_free(struct pdm_master *master);
int pdm_master_register(struct pdm_master *master);
void pdm_master_unregister(struct pdm_master *master);


/*                                                                              */
/*                              私有公共数据类型定义                                      */
/*                                                                              */

// 定义 PDM 子驱动结构体
struct pdm_subdriver {
    int (*init)(void);          // 初始化函数指针
    void (*exit)(void);         // 退出函数指针
    struct list_head list;      // 用于链表管理
};



#endif /* _PDM_H_ */
