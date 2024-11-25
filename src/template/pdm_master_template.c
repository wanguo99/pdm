#include "pdm.h"
#include "pdm_template.h"

/**
 * @brief 全局 PDM 主设备指针
 *
 * 该指针用于存储 PDM 主设备的实例。
 */
static struct pdm_master *g_pstPdmMaster = NULL;


/**
 * @brief PDC 模板设备的 WRITE 操作
 *
 * 该函数处理 PDC 模板设备的 WRITE 请求。
 *
 * @filp: 文件结构
 * @buf: 用户空间缓冲区
 * @count: 要写入的字节数
 * @ppos: 当前文件位置
 * @return 成功返回 0，失败返回负错误码
 */
static ssize_t pdc_template_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)

{
    OSA_INFO(" Called pdc_template_write \n");
    return count;
}

/**
 * @brief PDC 模板设备的 IOCTL 操作
 *
 * 该函数处理 PDC 模板设备的 IOCTL 请求。
 *
 * @param filp 文件指针
 * @param cmd 命令码
 * @param arg 参数
 * @return 成功返回 0，失败返回负错误码
 */
static long pdc_template_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    OSA_INFO(" Called pdc_template_ioctl \n");
    return 0;
}

/**
 * @brief 查找 PDM 设备
 *
 * 该函数用于查找与给定实际设备对应的 PDM 设备。
 *
 * @param real_device 实际设备指针
 * @return 成功返回 PDM 设备指针，失败返回 NULL
 */
struct pdm_device *pdm_template_master_find_pdmdev(void *real_device)
{
    return pdm_master_client_find(g_pstPdmMaster, real_device);
}

/**
 * @brief 注册 PDM 设备
 *
 * 该函数用于注册 PDM 设备，将其添加到设备管理器中，并注册设备。
 *
 * @param pdmdev PDM 设备指针
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_template_master_register_device(struct pdm_device *pdmdev)
{
    int ret;

    if (!g_pstPdmMaster || !pdmdev) {
        OSA_ERROR("Invalid input parameters\n");
        return -EINVAL;
    }

    ret = pdm_master_client_add(g_pstPdmMaster, pdmdev);
    if (ret) {
        OSA_ERROR("Failed to add template device, ret=%d.\n", ret);
        return ret;
    }

    ret = pdm_device_register(pdmdev);
    if (ret) {
        OSA_ERROR("Failed to register pdm_device, ret=%d.\n", ret);
        pdm_master_client_delete(g_pstPdmMaster, pdmdev);
        return ret;
    }

    OSA_INFO("Device %s registered.\n", dev_name(&pdmdev->dev));
    return 0;
}

/**
 * @brief 注销 PDM 设备
 *
 * 该函数用于注销 PDM 设备，将其从设备管理器中移除，并注销设备。
 *
 * @param pdmdev PDM 设备指针
 */
void pdm_template_master_unregister_device(struct pdm_device *pdmdev)
{
    if (!pdmdev) {
        OSA_ERROR("pdmdev is NULL\n");
        return;
    }

    pdm_device_unregister(pdmdev);
    pdm_master_client_delete(g_pstPdmMaster, pdmdev);
}

/**
 * @brief 初始化 PDM 模板主设备
 *
 * 该函数用于初始化 PDM 模板主设备，分配内存、设置名称、注册设备等。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_template_master_init(void)
{
    int status = 0;
    struct pdm_template_master_priv *pstTemplateMasterPriv = NULL;

    g_pstPdmMaster = pdm_master_alloc(sizeof(struct pdm_template_master_priv));
    if (!g_pstPdmMaster) {
        OSA_ERROR("Master allocation failed\n");
        return -ENOMEM;
    }

    pstTemplateMasterPriv = pdm_master_devdata_get(g_pstPdmMaster);
    if (!pstTemplateMasterPriv) {
        OSA_ERROR("pdm_master_devdata_get failed.\n");
        status = -ENODATA;
        goto err_master_free;
    }

    strcpy(g_pstPdmMaster->name, "template");
    status = pdm_master_register(g_pstPdmMaster);
    if (status < 0) {
        OSA_ERROR("pdm_master_register failed.\n");
        goto err_master_free;
    }

    // TODO: 需要在注册master之后对私有数据进行初始化,否则可能会被初始数据覆盖
    g_pstPdmMaster->fops.unlocked_ioctl = pdc_template_ioctl;
    g_pstPdmMaster->fops.write = pdc_template_write;

    OSA_INFO("Template Master initialized OK.\n");
    return 0;

err_master_free:
    pdm_master_free(g_pstPdmMaster);
    g_pstPdmMaster = NULL;
    return status;
}

/**
 * @brief 退出 PDM 模板主设备
 *
 * 该函数用于退出 PDM 模板主设备，注销设备并释放内存。
 */
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
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Template Master Driver");
