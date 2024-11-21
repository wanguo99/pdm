#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/version.h>

#include "pdm.h"
#include "pdm_template.h"


struct pdm_master *g_pstPdmMaster;

static long pdc_template_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct pdm_device *client;
    int index = 1;

    osa_info("-------------------------\n");
    printk(KERN_ERR "[WANGUO] (%s:%d) \n", __func__, __LINE__);
    osa_info("-------------------------\n\n");

    mutex_lock(&g_pstPdmMaster->client_list_mutex_lock);
    osa_info("Device List:\n\n");
    list_for_each_entry(client, &g_pstPdmMaster->clients, node)
    {
        printk(KERN_ERR "[%d] Client Name: %s \n", index++, dev_name(&client->dev));
    }
    mutex_unlock(&g_pstPdmMaster->client_list_mutex_lock);

    printk(KERN_ERR "\n");
    return 0;
}

struct pdm_device *pdm_template_master_get_pdmdev_of_real_device(void *real_device)
{
    return pdm_master_get_pdmdev_of_real_device(g_pstPdmMaster, real_device);
}

int pdm_template_master_add_device(struct pdm_device *pdmdev)
{
    return pdm_master_add_device(g_pstPdmMaster, pdmdev);
}

int pdm_template_master_del_device(struct pdm_device *pdmdev)
{
    return pdm_master_delete_device(g_pstPdmMaster, pdmdev);
}

int pdm_template_master_init(void)
{
	int status = 0;
    struct pdm_template_master_priv *pstTemplateMasterPriv = NULL;

	g_pstPdmMaster = pdm_master_alloc(sizeof(struct pdm_template_master_priv));  // 申请pdm_master和私有数据内存
	if (g_pstPdmMaster == NULL)
	{
		printk(KERN_ERR "master allocation failed\n");
		return -ENOMEM;
	}

	pstTemplateMasterPriv = pdm_master_get_devdata(g_pstPdmMaster);
    if (NULL == pstTemplateMasterPriv)
    {
        printk(KERN_ERR "pdm_master_get_devdata failed.\n");
		goto err_master_free;
    }

	g_pstPdmMaster = pdm_master_get(g_pstPdmMaster);
    if (status < 0)
    {
        printk(KERN_ERR "pdm_master_get failed.\n");
		goto err_master_free;
    }

    strcpy(g_pstPdmMaster->name, "template");
	status = pdm_master_register(g_pstPdmMaster);
	if (status < 0)
	{
        printk(KERN_ERR "pdm_master_register failed.\n");
        goto err_master_put;
	}

    g_pstPdmMaster->fops.unlocked_ioctl = pdc_template_ioctl;

    printk(KERN_INFO "TEMPLATE Master initialized OK.\n");

    return 0;

err_master_put:
    pdm_master_put(g_pstPdmMaster);

err_master_free:
    pdm_master_free(g_pstPdmMaster);

	return status;
}

void pdm_template_master_exit(void)
{
    if (!g_pstPdmMaster)
    {
        printk(KERN_ERR "TEMPLATE Master exit called with g_pstTemplateMaster as NULL\n");
        return;
    }

    pdm_master_unregister(g_pstPdmMaster);
    pdm_master_put(g_pstPdmMaster);
    pdm_master_free(g_pstPdmMaster);

    printk(KERN_INFO "TEMPLATE Master exited\n");
}


MODULE_LICENSE("GPL");
MODULE_AUTHOR("wanguo");
MODULE_DESCRIPTION("PDM TEMPLATE Master Module.");

