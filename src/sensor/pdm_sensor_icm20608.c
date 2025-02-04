#include <linux/spi/spi.h>
#include <linux/delay.h>

#include "pdm.h"
#include "pdm_sensor_priv.h"
#include "pdm_sensor_icm20608.h"

static int pdm_sensor_icm20608_read_reg(struct pdm_client *client, u8 reg, unsigned char *buf)
{
	struct spi_transfer xfer;
	struct spi_message msg;
	unsigned char txd[PDM_SENSOR_ICM20608_RW_LEN];
	unsigned char rxd[PDM_SENSOR_ICM20608_RW_LEN];
	int status;

	if (!client || client->hardware.spi.spidev) {
		OSA_ERROR("invalid argument\n");
		return -EINVAL;
	}

	memset(&xfer, 0, sizeof(xfer));
	memset(&txd, 0, sizeof(txd));
	memset(&rxd, 0, sizeof(rxd));

	txd[0] = reg | BIT(8);
	xfer.tx_buf = txd;
	xfer.rx_buf = rxd;
	xfer.len = PDM_SENSOR_ICM20608_RW_LEN;

	spi_message_init(&msg);
	spi_message_add_tail(&xfer, &msg);

	status = spi_sync(client->hardware.spi.spidev, &msg);
	if(status) {
		OSA_ERROR("syi_sync error: %d\n", status);
	}

	return status;
}

static int pdm_sensor_icm20608_write_reg(struct pdm_client *client, u8 reg, u8 value)
{
	struct spi_transfer xfer;
	struct spi_message msg;
	unsigned char txd[PDM_SENSOR_ICM20608_RW_LEN];
	unsigned char rxd[PDM_SENSOR_ICM20608_RW_LEN];
	int status;

	if (!client || client->hardware.spi.spidev) {
		OSA_ERROR("invalid argument\n");
		return -EINVAL;
	}

	memset(&xfer, 0, sizeof(xfer));
	memset(&txd, 0, sizeof(txd));
	memset(&rxd, 0, sizeof(rxd));

	txd[0] = reg & ~BIT(8);

	xfer.tx_buf = &txd;
	xfer.rx_buf = &rxd;
	xfer.len = PDM_SENSOR_ICM20608_RW_LEN;
	spi_message_init(&msg);
	spi_message_add_tail(&xfer, &msg);

	status = spi_sync(client->hardware.spi.spidev, &msg);
	if(status) {
		OSA_ERROR("syi_sync error: %d\n", status);
	}

	return status;
}

static int pdm_sensor_ap3216c_read(struct pdm_client *client, unsigned int type, unsigned int *val)
{
	unsigned char value;
	unsigned char offset;
	int status;

	for (offset = ICM20_ACCEL_XOUT_H; offset <= ICM20_GYRO_ZOUT_L; offset++) {
		status = pdm_sensor_icm20608_read_reg(client, offset, &value);
		if (status) {
			OSA_ERROR("read reg low_data failed, status: %d\n", status);
			return status;
		}
		OSA_VAR_UCHAR_NUMERIC(value);
	}

	return 0;
}

static int pdm_sensor_icm20608_init(struct pdm_client *client)
{
	int status;
	u8 value = 0;

	pdm_sensor_icm20608_write_reg(client, ICM20_PWR_MGMT_1, 0x80);
	mdelay(50);
	pdm_sensor_icm20608_write_reg(client, ICM20_PWR_MGMT_1, 0x01);
	mdelay(50);

	status = pdm_sensor_icm20608_read_reg(client, ICM20_WHO_AM_I, &value);
	if (status) {
		OSA_ERROR("Failed to read ICM20608 ID, status = %d\n", status);
	}
	printk("ICM20608 ID = %#X\r\n", value);

	pdm_sensor_icm20608_write_reg(client, ICM20_SMPLRT_DIV, 0x00);		/* 输出速率是内部采样率				*/
	pdm_sensor_icm20608_write_reg(client, ICM20_GYRO_CONFIG, 0x18);		/* 陀螺仪±2000dps量程 			*/
	pdm_sensor_icm20608_write_reg(client, ICM20_ACCEL_CONFIG, 0x18);	/* 加速度计±16G量程 				*/
	pdm_sensor_icm20608_write_reg(client, ICM20_CONFIG, 0x04);		/* 陀螺仪低通滤波BW=20Hz 			*/
	pdm_sensor_icm20608_write_reg(client, ICM20_ACCEL_CONFIG2, 0x04);	/* 加速度计低通滤波BW=21.2Hz 			*/
	pdm_sensor_icm20608_write_reg(client, ICM20_PWR_MGMT_2, 0x00);		/* 打开加速度计和陀螺仪所有轴 			*/
	pdm_sensor_icm20608_write_reg(client, ICM20_LP_MODE_CFG, 0x00);		/* 关闭低功耗 				*/
	pdm_sensor_icm20608_write_reg(client, ICM20_FIFO_EN, 0x00);		/* 关闭FIFO				*/

	return 0;
}

/**
 * @brief Initializes the ICM20608 sensor settings.
 */
static int pdm_sensor_icm20608_setup(struct pdm_client *client)
{
	struct pdm_sensor_priv *sensor_priv = pdm_client_get_private_data(client);
	const struct device_node *np = pdm_client_get_of_node(client);
	int status;

	if (!client || !sensor_priv || !np) {
		OSA_ERROR("Invalid parameters\n");
		return -EINVAL;
	}

	sensor_priv->read = pdm_sensor_ap3216c_read;
	client->hardware.spi.spidev = to_spi_device(client->pdmdev->dev.parent);

	status = pdm_sensor_icm20608_init(client);
	if (status) {
		OSA_ERROR("Failed to enable AP3216C sensor: %d\n", status);
		return status;
	}

	OSA_DEBUG("PDM SENSOR Setup: %s\n", client->name);

	return 0;
}

const struct pdm_client_match_data pdm_sensor_icm20608_match_data = {
	.setup = pdm_sensor_icm20608_setup,
	.cleanup = NULL,
};

