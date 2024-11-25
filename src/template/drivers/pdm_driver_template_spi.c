#include <linux/spi/spi.h>

#include "pdm.h"
#include "pdm_template.h"
#include "pdm_driver_template.h"

/**
 * @brief SPI 设备探测函数
 *
 * 该函数在 SPI 设备被探测到时调用，负责初始化和注册 PDM 设备。
 *
 * @param spi 指向 SPI 设备的指针
 * @return 成功返回 0，失败返回负错误码
 */
static int pdm_template_spi_probe(struct spi_device *spi)
{
    const struct pdm_template_device_priv *data;
    struct pdm_device *pdmdev;
    const char *compatible;
    int status;

    pdmdev = pdm_device_alloc(sizeof(struct pdm_template_device_priv));
    if (!pdmdev) {
        OSA_ERROR("Failed to allocate pdm_device.\n");
        return -ENOMEM;
    }

    status = of_property_read_string(spi->dev.of_node, "compatible", &compatible);
    if (status) {
        pr_err("Failed to read compatible property: %d\n", status);
        goto unregister_pdmdev;
    }

    strcpy(pdmdev->compatible, compatible);
    pdmdev->real_device = spi;
    status = pdm_template_master_register_device(pdmdev);
    if (status) {
        OSA_ERROR("Failed to add template device, status=%d.\n", status);
        goto free_pdmdev;
    }

	data = of_device_get_match_data(&spi->dev);
	if (!data)
	{
        OSA_ERROR("Failed to get match data, status=%d.\n", status);
		goto unregister_pdmdev;
    }
    pdm_device_devdata_set(pdmdev, (void *)data);

    OSA_INFO("Template SPI Device Probed.\n");
    return 0;

unregister_pdmdev:
    pdm_template_master_unregister_device(pdmdev);

free_pdmdev:
    pdm_device_free(pdmdev);

    return status;

}

/**
 * @brief SPI 设备移除函数
 *
 * 该函数在 SPI 设备被移除时调用，负责注销和释放 PDM 设备。
 *
 * @param spi 指向 SPI 设备的指针
 */
static void pdm_template_spi_remove(struct spi_device *spi)
{
    struct pdm_device *pdmdev = pdm_template_master_find_pdmdev(spi);
    if (NULL == pdmdev) {
        OSA_ERROR("Failed to find pdm device from master.\n");
        return;
    }

    pdm_template_master_unregister_device(pdmdev);
    pdm_device_free(pdmdev);

    OSA_INFO("Template SPI Device Removed.\n");
    return;
}


/**
 * @ dts节点配置示例

 * &qspi {
 *     pinctrl-names = "default";
 *     pinctrl-0 = <&pinctrl_qspi>;
 *     status = "okay";

 *     template-spi-0@1 {
 *     #address-cells = <1>;
 *     #size-cells = <1>;
 *         compatible = "pdm,template-spi";
 *         spi-max-frequency = <100000000>;
 *         spi-rx-bus-width = <4>;
 *         spi-tx-bus-width = <1>;
 *         reg = <1>;
 *     };
 * };
*/

static const struct of_device_id of_spi_template_match[] = {
	{ .compatible = "pdm,template-spi", },
	{},
};
MODULE_DEVICE_TABLE(of, of_spi_template_match);

static const struct spi_device_id spi_template_ids[] = {
	{ .name = "template-spi" },
	{ }
};
MODULE_DEVICE_TABLE(spi, spi_template_ids);


static struct spi_driver pdm_template_spi_driver = {
	.probe		= pdm_template_spi_probe,
	.remove	= pdm_template_spi_remove,
	.driver		= {
		.name	= "pdm-template-spi",
		.of_match_table = of_spi_template_match,
	},
	.id_table   = spi_template_ids,
};


/**
 * @brief 初始化 SPI 驱动
 *
 * 该函数用于初始化 SPI 驱动，注册 SPI 驱动到系统。
 *
 * @return 成功返回 0，失败返回负错误码
 */
int pdm_template_spi_driver_init(void) {
    int status;

    status = spi_register_driver(&pdm_template_spi_driver);
    if (status) {
        OSA_ERROR("Failed to register Template SPI Driver.\n");
        return status;
    }
    OSA_INFO("Template SPI Driver Initialized.\n");
    return 0;
}

/**
 * @brief 退出 SPI 驱动
 *
 * 该函数用于退出 SPI 驱动，注销 SPI 驱动。
 */
void pdm_template_spi_driver_exit(void) {
    spi_unregister_driver(&pdm_template_spi_driver);
    OSA_INFO("Template SPI Driver Exited.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Template SPI Driver");
