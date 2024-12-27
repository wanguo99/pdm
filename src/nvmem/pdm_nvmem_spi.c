#include <linux/spi/spi.h>
#include <linux/regmap.h>

#include "pdm.h"
#include "pdm_nvmem_priv.h"


static int pdm_nvmem_spi_read_reg(struct pdm_client *client, unsigned char addr, unsigned char *value)
{
    OSA_INFO("SPI PDM NVMEM Read: %s [0x%x]\n", dev_name(&client->dev), addr);
    return 0;
}

static int pdm_nvmem_spi_write_reg(struct pdm_client *client, unsigned char addr, unsigned char value)
{
    OSA_INFO("SPI PDM NVMEM Write %s [0x%x] to 0x%x\n", dev_name(&client->dev), addr, value);
    return 0;
}

/**
 * @struct pdm_nvmem_operations
 * @brief PDM NVMEM device operations structure (SPI version).
 *
 * This structure defines the operation functions for a PDM NVMEM device using SPI.
 */
static const struct pdm_nvmem_operations pdm_device_nvmem_ops_spi = {
    .read_reg = pdm_nvmem_spi_read_reg,
    .write_reg = pdm_nvmem_spi_write_reg,
};

static int pdm_nvmem_regmap_spi_init(struct pdm_client *client)
{
    struct regmap_config regmap_config;
    struct regmap *regmap;

    memset(&regmap_config, 0, sizeof(regmap_config));
    regmap_config.val_bits = 8;
    regmap_config.reg_bits = 8;
    regmap_config.disable_locking = true;

    regmap = devm_regmap_init_spi(client->hardware.spi.spidev, &regmap_config);
    if (IS_ERR(regmap)) {
        return PTR_ERR(regmap);
    }

    return 0;
}

/**
 * @brief Initializes SPI settings for a PDM device.
 *
 * This function initializes the SPI settings for the specified PDM device and sets up the operation functions.
 *
 * @param client Pointer to the PDM client structure.
 * @return Returns 0 on success; negative error code on failure.
 */
int pdm_nvmem_spi_setup(struct pdm_client *client)
{
    struct pdm_nvmem_priv *nvmem_priv;
    struct device_node *np;
    int status;

    if (!client) {
        OSA_ERROR("Invalid client\n");
    }

    nvmem_priv = pdm_client_get_private_data(client);
    if (!nvmem_priv) {
        OSA_ERROR("Get PDM Client DevData Failed\n");
        return -ENOMEM;
    }
    nvmem_priv->ops = &pdm_device_nvmem_ops_spi;

    np = pdm_client_get_of_node(client);
    if (!np) {
        OSA_ERROR("No DT node found\n");
        return -EINVAL;
    }

    if (of_get_property(np, "enable-regmap", NULL)) {
        OSA_INFO("No default-state property found, using defaults as off\n");
        status = pdm_nvmem_regmap_spi_init(client);
        if (status) {
            OSA_ERROR("pdm_nvmem_regmap_spi_init failed, status: %d\n", status);
            return status;
        }
    }

    OSA_DEBUG("SPI NVMEM Setup: %s\n", dev_name(&client->dev));
    return 0;
}
