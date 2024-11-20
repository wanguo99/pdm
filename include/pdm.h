#ifndef _PDM_H_
#define _PDM_H_

#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/idr.h>
#include <linux/mod_devicetable.h>
#include <linux/version.h>



/*                                                                              */
/*                                公共数据类型声明                                      */
/*                                                                              */

extern const  struct device_type pdm_device_type;

/*                                                                              */
/*                                公共数据类型定义                                      */
/*                                                                              */
#define PDM_DEVICE_NAME_SIZE    (64)

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
    struct file_operations fops;            // 每个master内部单独实现一套文件操作
    struct idr device_idr;                  // 给子设备分配ID使用
    struct rw_semaphore rwlock;             // 读写锁, sysfs读写master属性时使用
    unsigned int init_done : 1;             // 初始化标志
    struct list_head node;                  // bus挂载点
    struct list_head clients;               // 子设备列表
    struct mutex client_list_mutex_lock;    // 子设备列表锁
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

/*
    common
*/

static inline struct pdm_driver *drv_to_pdmdrv(struct device_driver *drv)
{
    return container_of(drv, struct pdm_driver, driver);
}



const struct pdm_device_id *pdm_match_id(const struct pdm_device_id *id, struct pdm_device *pdmdev);

/*
    pdm_device
*/
#define dev_to_pdmdev(__dev)    container_of(__dev, struct pdm_device, dev)
struct pdm_device *pdm_device_alloc(struct pdm_master *master);
void pdm_device_free(struct pdm_device *pdmdev);
void pdm_device_unregister(struct pdm_device *pdmdev);


/*
    pdm_master
*/
#define dev_to_pdm_master(__dev) container_of(__dev, struct pdm_master, dev)

static inline void *pdm_master_get_devdata(struct pdm_master *master)
{
    return dev_get_drvdata(&master->dev);
}

static inline void pdm_master_set_devdata(struct pdm_master *master, void *data)
{
    dev_set_drvdata(&master->dev, data);
}

struct pdm_master *pdm_master_alloc(unsigned int size);
struct pdm_master *pdm_master_get(struct pdm_master *master);
void pdm_master_put(struct pdm_master *master);
int  pdm_master_register(struct pdm_master *master);
void pdm_master_unregister(struct pdm_master *master);
int  pdm_master_init(void);
void pdm_master_exit(void);
int pdm_master_add_device(struct pdm_master *master, struct pdm_device *pdmdev);
int pdm_master_delete_device(struct pdm_master *master, struct pdm_device *pdmdev);



/*                                                                              */
/*                                全局变量声明                                        */
/*                                                                              */

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 16, 0)
extern struct bus_type             pdm_bus_type;
#else
extern const struct bus_type       pdm_bus_type;
#endif


#endif /* _PDM_H_ */
