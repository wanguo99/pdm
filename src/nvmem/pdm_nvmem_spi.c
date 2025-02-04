#include <linux/spi/spi.h>
#include <linux/regmap.h>

#include "pdm.h"
#include "pdm_nvmem_priv.h"


static int pdm_nvmem_regmap_spi_read_reg(struct pdm_client *client, unsigned int offset, void *val, size_t bytes)
{
	struct spi_transfer t[2];
	struct spi_message  m;
	unsigned char cmd = PDM_NVMEM_CMD_READ_REG;
	char *buf = val;
	int status;

	spi_message_init(&m);
	memset(t, 0, sizeof(t));

	t[0].tx_buf = &cmd;
	t[0].len = 1;
	spi_message_add_tail(&t[0], &m);

	t[1].rx_buf = buf;
	t[1].len = 1;
	spi_message_add_tail(&t[1], &m);

	status = spi_sync(client->hardware.spi.spidev, &m);

	return 0;
}

static int pdm_nvmem_regmap_spi_write_reg(struct pdm_client *client, unsigned int offset, void *val, size_t bytes)
{
	return 0;
}

static int pdm_nvmem_regmap_spi_init(struct pdm_client *client)
{
	struct pdm_nvmem_priv *nvmem_priv;
	struct regmap_config regmap_config;
	struct regmap *regmap;

	nvmem_priv = pdm_client_get_private_data(client);
	if (!nvmem_priv) {
		OSA_ERROR("Get PDM Client DevData Failed\n");
		return -ENOMEM;
	}

	nvmem_priv->read_reg = pdm_nvmem_regmap_spi_read_reg;
	nvmem_priv->write_reg = pdm_nvmem_regmap_spi_write_reg;

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
static int pdm_nvmem_spi_setup(struct pdm_client *client)
{
	struct device_node *np;
	int status;

	if (!client) {
		OSA_ERROR("Invalid client\n");
	}

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

	OSA_DEBUG("SPI NVMEM Setup: %s-%d\n", dev_name(&client->adapter->dev), client->index);
	return 0;
}

static void pdm_nvmem_spi_cleanup(struct pdm_client *client)
{
	return;
}

/**
 * @brief Match data structure for initializing SPI type NVMEM devices.
 */
const struct pdm_client_match_data pdm_nvmem_spi_match_data = {
	.setup = pdm_nvmem_spi_setup,
	.cleanup = pdm_nvmem_spi_cleanup,
};
