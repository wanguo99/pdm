#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/version.h>

#include "pdm.h"
#include "pdm_template.h"

static struct pdm_template_master *g_pstTemplateMaster;

static long pdc_template_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct pdm_device *client;
    int index = 1;

    pr_info("-------------------------\n");
    printk(KERN_ERR "[WANGUO] (%s:%d) \n", __func__, __LINE__);
    pr_info("-------------------------\n\n");

    mutex_lock(&g_pstTemplateMaster->master->client_list_mutex_lock);
    pr_info("Device List:\n\n");
    list_for_each_entry(client, &g_pstTemplateMaster->master->clients, node)
    {
        printk(KERN_ERR "[%d] Client Name: %s \n", index++, dev_name(&client->dev));
    }
    mutex_unlock(&g_pstTemplateMaster->master->client_list_mutex_lock);

    printk(KERN_ERR "\n");
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

int pdm_template_master_add_device(struct pdm_template_device *template_dev)
{
    return pdm_master_add_device(g_pstTemplateMaster->master, template_dev->pdmdev);
}

int pdm_template_master_del_device(struct pdm_template_device *template_dev)
{
    return pdm_master_delete_device(g_pstTemplateMaster->master, template_dev->pdmdev);
}

int pdm_template_master_init(void)
{
	int status = 0;
	struct pdm_master *master;

	master = pdm_master_alloc(sizeof(struct pdm_template_master));  // 申请pdm_master和私有数据内存
	if (master == NULL)
	{
		printk(KERN_ERR "master allocation failed\n");
		return -ENOMEM;
	}

	g_pstTemplateMaster = pdm_master_get_devdata(master);
    if (NULL == g_pstTemplateMaster)
    {
        printk(KERN_ERR "pdm_master_get_devdata failed.\n");
		goto err_master_put;
    }

	g_pstTemplateMaster->master = pdm_master_get(master);
    if (status < 0)
    {
        printk(KERN_ERR "pdm_master_get failed.\n");
		goto err_master_put;
    }

    strcpy(g_pstTemplateMaster->master->name, "template");
	status = pdm_master_register(master);
	if (status < 0)
	{
        printk(KERN_ERR "pdm_master_register failed.\n");
        goto err_master_put;
	}

    g_pstTemplateMaster->master->fops.unlocked_ioctl = pdc_template_ioctl;

    printk(KERN_INFO "TEMPLATE Master initialized OK.\n");

    return 0;

err_master_put:
    pdm_master_put(master);

	return status;
}

void pdm_template_master_exit(void)
{
    if (!g_pstTemplateMaster)
    {
        printk(KERN_ERR "TEMPLATE Master exit called with g_pstTemplateMaster as NULL\n");
        return;
    }
    pdm_master_unregister(g_pstTemplateMaster->master);
    pdm_master_put(g_pstTemplateMaster->master);
    printk(KERN_INFO "TEMPLATE Master exited\n");
}


MODULE_LICENSE("GPL");
MODULE_AUTHOR("wanguo");
MODULE_DESCRIPTION("PDM TEMPLATE Master Module.");

