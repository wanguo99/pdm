#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/version.h>

#include "pdm.h"
#include "pdm_template.h"

/**
 * @brief 兼容旧内核版本的 i2c_device_id 结构体定义
 *
 * 该结构体用于兼容 Linux 内核版本低于 2.6.25 的情况。
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 25)
struct i2c_device_id {
    char name[I2C_NAME_SIZE];
    kernel_ulong_t driver_data;
};
#endif

/**
 * @brief 实际的 I2C 探测函数
 *
 * 该函数用于处理 I2C 设备的探测操作，分配 PDM 设备并注册到主设备。
 *
 * @param client I2C 客户端指针
 * @param id I2C 设备 ID
 * @return 成功返回 0，失败返回负错误码
 */
static int pdm_template_i2c_real_probe(struct i2c_client *client, const struct i2c_device_id *id) {
    struct pdm_device *pdmdev;
    struct pdm_template_device_priv *pstTemplateDevPriv;
    int ret;

    pdmdev = pdm_device_alloc(sizeof(struct pdm_template_device_priv));
    if (!pdmdev) {
        OSA_ERROR("Failed to allocate pdm_device.\n");
        return -ENOMEM;
    }

    pdmdev->real_device = client;
    strcpy(pdmdev->compatible, "pdm_template,i2c");
    ret = pdm_template_master_register_device(pdmdev);
    if (ret) {
        OSA_ERROR("Failed to add template device, ret=%d.\n", ret);
        goto free_pdmdev;
    }

    pstTemplateDevPriv = pdm_device_devdata_get(pdmdev);
    if (!pstTemplateDevPriv) {
        OSA_ERROR("Failed to get device private data.\n");
        ret = -EFAULT;
        goto unregister_pdmdev;
    }
    pstTemplateDevPriv->ops = NULL;

    OSA_DEBUG("Template I2C Device Probed.\n");
    return 0;

unregister_pdmdev:
    pdm_template_master_unregister_device(pdmdev);

free_pdmdev:
    pdm_device_free(pdmdev);

    return ret;
}

/**
 * @brief 实际的 I2C 移除函数
 *
 * 该函数用于处理 I2C 设备的移除操作，注销并释放 PDM 设备。
 *
 * @param client I2C 客户端指针
 * @return 成功返回 0，失败返回负错误码
 */
static int pdm_template_i2c_real_remove(struct i2c_client *client) {
    struct pdm_device *pdmdev = pdm_template_master_find_pdmdev(client);
    if (NULL == pdmdev) {
        OSA_ERROR("Failed to find pdm device from master.\n");
        return -ENODEV;
    }

    pdm_template_master_unregister_device(pdmdev);
    pdm_device_free(pdmdev);
    return 0;
}

/**
 * @brief 兼容旧内核版本的 I2C 探测函数
 *
 * 该函数用于兼容 Linux 内核版本低于 6.3.0 的情况。
 *
 * @param client I2C 客户端指针
 * @param id I2C 设备 ID
 * @return 成功返回 0，失败返回负错误码
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 3, 0)
static int pdm_template_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id) {
    return pdm_template_i2c_real_probe(client, id);
}
#else
static int pdm_template_i2c_probe(struct i2c_client *client) {
    return pdm_template_i2c_real_probe(client, NULL);
}
#endif

/**
 * @brief 兼容旧内核版本的 I2C 移除函数
 *
 * 该函数用于兼容 Linux 内核版本低于 6.0.0 的情况。
 *
 * @param client I2C 客户端指针
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
static int pdm_template_i2c_remove(struct i2c_client *client) {
    return pdm_template_i2c_real_remove(client);
}
#else
static void pdm_template_i2c_remove(struct i2c_client *client) {
    (void)pdm_template_i2c_real_remove(client);
}
#endif

/**
 * @brief I2C 设备 ID 表
 *
 * 该表定义了支持的 I2C 设备 ID。
 */
static const struct i2c_device_id pdm_template_i2c_id[] = {
    { "pdm_template", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, pdm_template_i2c_id);

/**
 * @brief 设备树匹配表
 *
 * 该表定义了支持的设备树兼容性字符串。
 */
static const struct of_device_id pdm_template_i2c_matches[] = {
    { .compatible = "pdm_template,i2c" },
    { }
};
MODULE_DEVICE_TABLE(of, pdm_template_i2c_matches);

/**
 * @brief I2C 驱动结构体
 *
 * 该结构体定义了 I2C 驱动的基本信息和操作函数。
 */
static struct i2c_driver pdm_template_i2c_driver = {
    .driver = {
        .name = "pdm_template_i2c",
        .of_match_table = pdm_template_i2c_matches,
        .owner = THIS_MODULE,
    },
    .probe = pdm_template_i2c_probe,
    .remove = pdm_template_i2c_remove,
    .id_table = pdm_template_i2c_id,
};

/**
 * @brief 初始化 I2C 驱动
 *
 * 该函数用于初始化 I2C 驱动，注册 I2C 驱动到系统。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_template_i2c_driver_init(void) {
    int ret;

    ret = i2c_add_driver(&pdm_template_i2c_driver);
    if (ret) {
        OSA_ERROR("Failed to register Template I2C Driver.\n");
        return ret;
    }
    OSA_INFO("Template I2C Driver Initialized.\n");
    return 0;
}

/**
 * @brief 退出 I2C 驱动
 *
 * 该函数用于退出 I2C 驱动，注销 I2C 驱动。
 */
void pdm_template_i2c_driver_exit(void) {
    i2c_del_driver(&pdm_template_i2c_driver);
    OSA_INFO("Template I2C Driver Exited.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wanguo");
MODULE_DESCRIPTION("PDM Template I2C Driver.");
