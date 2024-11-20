#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/version.h>

#include "pdm.h"
#include "pdm_cpld.h"

static struct pdm_cpld_master *g_pstCpldMaster;

static long pdc_cpld_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    pr_info("-------------------------\n");
    pr_info("pdc_cpld_ioctl.\n");
    pr_info("-------------------------\n");

    return 0;

    switch (cmd)
    {
        default:
        {
            return -ENOTTY;
        }
    }

    return 0;
}

int pdm_cpld_master_add_device(struct pdm_cpld_device *cpld_dev)
{
    return pdm_master_add_device(g_pstCpldMaster->master, cpld_dev->pdmdev);
}

int pdm_cpld_master_del_device(struct pdm_cpld_device *cpld_dev)
{
    return pdm_master_delete_device(g_pstCpldMaster->master, cpld_dev->pdmdev);
}

int pdm_cpld_master_init(void)
{
	int status = 0;
	struct pdm_master *master;

	master = pdm_master_alloc(sizeof(struct pdm_cpld_master));  // 申请pdm_master和私有数据内存
	if (master == NULL)
	{
		printk(KERN_ERR "master allocation failed\n");
		return -ENOMEM;
	}

	g_pstCpldMaster = pdm_master_get_devdata(master);   // 获取pdm_master私有数据
    if (status < 0)
    {
        printk(KERN_ERR "pdm_master_get_devdata failed.\n");
		goto err_master_put;
    }

	g_pstCpldMaster->master = pdm_master_get(master);   // 引用计数加一
    if (status < 0)
    {
        printk(KERN_ERR "pdm_master_get failed.\n");
		goto err_master_put;
    }


    // 设置master名称
    strcpy(g_pstCpldMaster->master->name, "cpld");
	status = pdm_master_register(master);           // 注册pdm_master
	if (status < 0)
	{
        printk(KERN_ERR "pdm_master_register failed.\n");
        goto err_master_put;
	}

    g_pstCpldMaster->master->fops.unlocked_ioctl = pdc_cpld_ioctl;
    printk(KERN_INFO "CPLD Master initialized OK.\n");

    return 0;

err_master_put:
    pdm_master_put(g_pstCpldMaster->master);

	return status;
}

void pdm_cpld_master_exit(void)
{
    if (!g_pstCpldMaster)
    {
        printk(KERN_ERR "CPLD Master exit called with g_pstCpldMaster as NULL\n");
        return;
    }
    pdm_master_unregister(g_pstCpldMaster->master);
    pdm_master_put(g_pstCpldMaster->master);
    printk(KERN_INFO "CPLD Master exited\n");
}


MODULE_LICENSE("GPL");
MODULE_AUTHOR("wanguo");
MODULE_DESCRIPTION("PDM CPLD Master Module.");

