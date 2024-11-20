#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/version.h>

#include "pdm.h"
#include "pdm_cpld.h"

static struct pdm_cpld_master *g_pstCpldMaster;


// CPLD Master的ioctl实现
static long pdc_cpld_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    pr_info("pdc_cpld_ioctl.\n");

    switch (cmd)
    {
        default:
            return -ENOTTY;
    }

    return 0;
}

int pdm_cpld_master_init(void)
{
    int ret;

    // 分配内存
    g_pstCpldMaster = kzalloc(sizeof(struct pdm_cpld_master), GFP_KERNEL);
    if (!g_pstCpldMaster)
    {
        printk(KERN_ERR "Memory allocation failed\n");
        return -ENOMEM;
    }

    // 设置master名称
    strcpy(g_pstCpldMaster->master.name, "cpld");

    // 注册主设备
    ret = pdm_master_register(&g_pstCpldMaster->master);
    if (ret)
    {
        printk(KERN_ERR "CPLD Master register failed. ret: %d.\n", ret);
        kfree(g_pstCpldMaster);
        return ret;
    }

    // 重新指定ioctl指针
    g_pstCpldMaster->master.fops.unlocked_ioctl = pdc_cpld_ioctl;

    printk(KERN_INFO "CPLD Master initialized OK.\n");

    return 0;
}


void pdm_cpld_master_exit(void)
{
    if (g_pstCpldMaster) {
        // 注销主设备
        pdm_master_unregister(&g_pstCpldMaster->master);

        // 释放内存
        kfree(g_pstCpldMaster);
        g_pstCpldMaster = NULL;  // 防止悬挂指针

        printk(KERN_INFO "CPLD Master exited\n");
    } else {
        printk(KERN_ERR "CPLD Master exit called with g_pstCpldMaster as NULL\n");
    }
}


MODULE_LICENSE("GPL");
MODULE_AUTHOR("wanguo");
MODULE_DESCRIPTION("PDM CPLD Master Module.");

