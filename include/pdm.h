#ifndef _PDM_H_
#define _PDM_H_

#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/idr.h>
#include <linux/mod_devicetable.h>



/*                                                                              */
/*                                公共数据类型声明                                      */
/*                                                                              */

extern const  struct device_type pdm_device_type;

/*                                                                              */
/*                                公共数据类型定义                                      */
/*                                                                              */
#define PDM_DEVICE_NAME_SIZE	32

struct pdm_device {
    int id;                 // 根据[master->name && name && id]可以唯一确定一个pdm设备
    const char *compatible; // pdm_driver_match使用
    struct device dev;
    struct pdm_master *master;
    struct list_head node;
};

struct pdm_master {
    char   name[PDM_DEVICE_NAME_SIZE];
    struct device dev;
    dev_t devno;
    struct cdev cdev;
    struct file_operations fops;    // 每个master内部单独实现一套文件操作
    struct idr device_idr;          // 给子设备分配ID使用
    struct rw_semaphore rwlock;     // 读写锁
    unsigned int init_done : 1;     // 初始化标志
    struct list_head node;          // bus挂载点
    struct list_head clients;       // 子设备列表
};


struct pdm_device_id {
	char compatible[PDM_DEVICE_NAME_SIZE];  // 驱动匹配字符串
	kernel_ulong_t driver_data;             // 驱动私有数据
};

struct pdm_driver {
    struct device_driver driver;
    const struct pdm_device_id id_table;
    int (*probe)(struct pdm_device *dev);
    void (*remove)(struct pdm_device *dev);
};


/*                                                                              */
/*                                    函数声明                                      */
/*                                                                              */

static inline struct pdm_driver *drv_to_pdmdrv(struct device_driver *drv)
{
	return container_of(drv, struct pdm_driver, driver);
}

#define dev_to_pdmdev(__dev)	container_of(__dev, struct pdm_device, dev)
#define dev_to_pdm_master(__dev) container_of(__dev, struct pdm_master, dev)


const struct pdm_device_id *pdm_match_id(const struct pdm_device_id *id, struct pdm_device *pdmdev);

int pdm_master_register(struct pdm_master *master);
void pdm_master_unregister(struct pdm_master *master);


/*                                                                              */
/*                              私有公共数据类型定义                                      */
/*                                                                              */

#endif /* _PDM_H_ */
