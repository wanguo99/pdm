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
        osa_error("Master is not initialized.\n");
        return -ENODEV;
    }

    osa_info("-------------------------\n");
    osa_info("-------------------------\n\n");

    mutex_lock(&g_pstPdmMaster->client_list_mutex_lock);
    osa_info("Device List:\n\n");
    list_for_each_entry(client, &g_pstPdmMaster->clients, node)
    {
        osa_info("[%d] Client Name: %s.\n", index++, dev_name(&client->dev));
    }
    mutex_unlock(&g_pstPdmMaster->client_list_mutex_lock);

    osa_info("\n");
    return 0;
}

struct pdm_device *pdm_template_master_get_pdmdev_of_real_device(void *real_device)
{
    return pdm_master_get_pdmdev_of_real_device(g_pstPdmMaster, real_device);
}

int pdm_template_master_add_device(struct pdm_device *pdmdev)
{
    if (!g_pstPdmMaster) {
        osa_error("Master is not initialized.\n");
        return -ENODEV;
    }
    return pdm_master_add_device(g_pstPdmMaster, pdmdev);
}

int pdm_template_master_del_device(struct pdm_device *pdmdev)
{
    if (!g_pstPdmMaster) {
        osa_error("Master is not initialized.\n");
        return -ENODEV;
    }
    return pdm_master_delete_device(g_pstPdmMaster, pdmdev);
}

int pdm_template_master_init(void)
{
    int status = 0;
    struct pdm_template_master_priv *pstTemplateMasterPriv = NULL;

    g_pstPdmMaster = pdm_master_alloc(sizeof(struct pdm_template_master_priv));
    if (!g_pstPdmMaster) {
        osa_error("Master allocation failed\n");
        return -ENOMEM;
    }

    pstTemplateMasterPriv = pdm_master_get_devdata(g_pstPdmMaster);
    if (!pstTemplateMasterPriv) {
        osa_error("pdm_master_get_devdata failed.\n");
        status = -ENODATA;
        goto err_master_free;
    }

    strcpy(g_pstPdmMaster->name, "template");
    status = pdm_master_register(g_pstPdmMaster);
    if (status < 0) {
        osa_error("pdm_master_register failed.\n");
        goto err_master_free;
    }

    g_pstPdmMaster->fops.unlocked_ioctl = pdc_template_ioctl;

    osa_info("Template Master initialized OK.\n");
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

    osa_info("Template Master exit.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wanguo");
MODULE_DESCRIPTION("PDM TEMPLATE Master Module.");
