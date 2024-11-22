#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/version.h>

#include "pdm.h"
#include "pdm_template.h"

static struct pdm_master *g_pstPdmMaster = NULL;  // Initialize to NULL

static long pdc_template_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct pdm_device *client;
    int index = 1;

    if (!g_pstPdmMaster) {
        OSA_ERROR("Master is not initialized.\n");
        return -ENODEV;
    }

    OSA_INFO("-------------------------\n");
    OSA_INFO("-------------------------\n\n");

    mutex_lock(&g_pstPdmMaster->client_list_mutex_lock);
    OSA_INFO("Device List:\n\n");
    list_for_each_entry(client, &g_pstPdmMaster->client_list, entry)
    {
        OSA_INFO("[%d] Client Name: %s.\n", index++, dev_name(&client->dev));
    }
    mutex_unlock(&g_pstPdmMaster->client_list_mutex_lock);

    OSA_INFO("\n");
    return 0;
}

struct pdm_device *pdm_template_master_find_pdmdev(void *real_device)
{
    return pdm_master_find_pdmdev(g_pstPdmMaster, real_device);
}

int pdm_template_master_register_device(struct pdm_device *pdmdev)
{
    int ret;

    if (!g_pstPdmMaster || !pdmdev) {
        pr_err("pdm_template_master_register_device: Invalid input parameters\n");
        return -EINVAL;
    }

    ret = pdm_master_add_device(g_pstPdmMaster, pdmdev);
    if (ret) {
        pr_err("Failed to add template device, ret=%d.\n", ret);
        return ret;
    }

    ret = pdm_device_register(pdmdev);
    if (ret) {
        pr_err("Failed to register pdm_device, ret=%d.\n", ret);
        pdm_master_delete_device(g_pstPdmMaster, pdmdev);
        return ret;
    }

    pr_info("Device %s registered with master.\n", dev_name(&pdmdev->dev));
    return 0;
}


void pdm_template_master_unregister_device(struct pdm_device *pdmdev)
{
    if (!pdmdev) {
        pr_err("pdm_template_master_unregister_device: pdmdev is NULL\n");
        return;
    }

    pdm_device_unregister(pdmdev);
    pdm_master_delete_device(g_pstPdmMaster, pdmdev);

    pr_info("Device %s unregistered from master.\n", dev_name(&pdmdev->dev));
}


int pdm_template_master_init(void)
{
    int status = 0;
    struct pdm_template_master_priv *pstTemplateMasterPriv = NULL;

    g_pstPdmMaster = pdm_master_alloc(sizeof(struct pdm_template_master_priv));
    if (!g_pstPdmMaster) {
        OSA_ERROR("Master allocation failed\n");
        return -ENOMEM;
    }

    pstTemplateMasterPriv = pdm_master_get_devdata(g_pstPdmMaster);
    if (!pstTemplateMasterPriv) {
        OSA_ERROR("pdm_master_get_devdata failed.\n");
        status = -ENODATA;
        goto err_master_free;
    }

    strcpy(g_pstPdmMaster->name, "template");
    g_pstPdmMaster->fops.unlocked_ioctl = pdc_template_ioctl;
    status = pdm_master_register(g_pstPdmMaster);
    if (status < 0) {
        OSA_ERROR("pdm_master_register failed.\n");
        goto err_master_free;
    }

    OSA_INFO("Template Master initialized OK.\n");
    return 0;

err_master_free:
    pdm_master_free(g_pstPdmMaster);
    g_pstPdmMaster = NULL;  // Reset to NULL to avoid dangling pointer
    return status;
}

void pdm_template_master_exit(void)
{
    if (g_pstPdmMaster) {
        pdm_master_unregister(g_pstPdmMaster);
        pdm_master_free(g_pstPdmMaster);
        g_pstPdmMaster = NULL;  // Ensure no dangling pointer
    }

    OSA_INFO("Template Master exit.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wanguo");
MODULE_DESCRIPTION("PDM TEMPLATE Master Module.");
