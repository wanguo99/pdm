#include <linux/i2c.h>

#include "pdm.h"
#include "pdm_sensor_priv.h"

enum AP3216C_DATA_TYPE {
	AP3216C_DATA_TYPE_NULL		= 0x00,	/* 无效数据 */
	AP3216C_DATA_TYPE_IR		= 0x01,	/* IR 数据 */
	AP3216C_DATA_TYPE_ALS		= 0x02,	/* ALS 数据 */
	AP3216C_DATA_TYPE_PS		= 0X03,	/* PS 数据 */
	AP3216C_DATA_TYPE_INVALID	= 0xFF,	/* 无效数据 */
};

/* AP3316C 寄存器 */
#define AP3216C_SYSTEMCONG	0x00	/* 配置寄存器 */
#define AP3216C_INTSTATUS	0X01	/* 中断状态寄存器 */
#define AP3216C_INTCLEAR	0X02	/* 中断清除寄存器 */
#define AP3216C_IRDATALOW	0x0A	/* IR 数据低字节 */
#define AP3216C_IRDATAHIGH	0x0B	/* IR 数据高字节 */
#define AP3216C_ALSDATALOW	0x0C	/* ALS 数据低字节 */
#define AP3216C_ALSDATAHIGH	0X0D	/* ALS 数据高字节 */
#define AP3216C_PSDATALOW	0X0E	/* PS 数据低字节 */
#define AP3216C_PSDATAHIGH	0X0F	/* PS 数据高字节 */

#define AP3216C_READ_MSG_COUNT	0X02	/* 读寄存器长度 */

static int pdm_sensor_ap3216c_read_reg(struct pdm_client *client, unsigned char reg, unsigned char *val, size_t len)
{
	struct i2c_client *i2cdev;
	struct i2c_msg msgs[AP3216C_READ_MSG_COUNT];
	int status;

	if (!client || !client->hardware.i2c.client) {
		OSA_ERROR("Invalid client\n");
		return -EINVAL;
	}
	i2cdev = client->hardware.i2c.client;

	msgs[0].addr = i2cdev->addr;
	msgs[0].flags = 0;
	msgs[0].buf = &reg;
	msgs[0].len = sizeof(reg);

	msgs[1].addr = i2cdev->addr;
	msgs[1].flags = I2C_M_RD;
	msgs[1].buf = val;
	msgs[1].len = len;

	status = i2c_transfer(i2cdev->adapter, msgs, AP3216C_READ_MSG_COUNT);
	if (AP3216C_READ_MSG_COUNT != status) {
		OSA_ERROR("i2c rd failed=%d reg=%06x len=%d\n", status, reg, len);
		return -EREMOTEIO;
	}

	return 0;
}

static int pdm_sensor_ap3216c_read_ir(struct pdm_client *client, unsigned short *val)
{
	unsigned char data_low;
	unsigned char data_high;
	int status;

	status = pdm_sensor_ap3216c_read_reg(client, AP3216C_IRDATALOW, &data_low, sizeof(data_low));
	if (status) {
		OSA_ERROR("read reg ir low_data failed, status: %d\n", status);
		return status;
	}

	status = pdm_sensor_ap3216c_read_reg(client, AP3216C_IRDATAHIGH, &data_high, sizeof(data_high));
	if (status) {
		OSA_ERROR("read reg ir high_data failed, status: %d\n", status);
		return status;
	}

	if(data_low & 0X80) {
		*val = 0;
	} else {
		*val = ((unsigned short)data_high << 2) | (data_low & 0X03);
	}

	OSA_INFO("IR: 0x%x\n", *val);
	return 0;
}

static int pdm_sensor_ap3216c_read_als(struct pdm_client *client, unsigned short *val)
{
	unsigned char data_low;
	unsigned char data_high;
	int status;

	status = pdm_sensor_ap3216c_read_reg(client, AP3216C_ALSDATALOW, &data_low, sizeof(data_low));
	if (status) {
		OSA_ERROR("read reg ir low_data failed, status: %d\n", status);
		return status;
	}

	status = pdm_sensor_ap3216c_read_reg(client, AP3216C_ALSDATAHIGH, &data_high, sizeof(data_high));
	if (status) {
		OSA_ERROR("read reg ir high_data failed, status: %d\n", status);
		return status;
	}

	*val = ((unsigned short)data_high << 8) | data_low ;

	OSA_INFO("ALS: 0x%x\n", *val);

	return 0;
}

static int pdm_sensor_ap3216c_read_ps(struct pdm_client *client, unsigned short *val)
{
	unsigned char data_low;
	unsigned char data_high;
	int status;

	status = pdm_sensor_ap3216c_read_reg(client, AP3216C_PSDATALOW, &data_low, sizeof(data_low));
	if (status) {
		OSA_ERROR("read reg ir low_data failed, status: %d\n", status);
		return status;
	}

	status = pdm_sensor_ap3216c_read_reg(client, AP3216C_PSDATAHIGH, &data_high, sizeof(data_high));
	if (status) {
		OSA_ERROR("read reg ir high_data failed, status: %d\n", status);
		return status;
	}

	if(data_low & 0X40) {
		*val = 0;
	} else {
		*val = ((unsigned short)(data_high & 0x3F) << 4) | (data_low & 0X0F);
	}

	OSA_INFO("PS: 0x%x\n", *val);

	return 0;
}

static int pdm_sensor_ap3216c_read(struct pdm_client *client, unsigned int type, unsigned int *val)
{
	int status;
	unsigned short value;

	if (type < AP3216C_DATA_TYPE_IR || type > AP3216C_DATA_TYPE_PS) {
		OSA_ERROR("Invalid data type\n");
		return -EINVAL;
	}

	switch (type) {
		case AP3216C_DATA_TYPE_IR: {
			status = pdm_sensor_ap3216c_read_ir(client, &value);
			break;
		}
		case AP3216C_DATA_TYPE_ALS: {
			status = pdm_sensor_ap3216c_read_als(client, &value);
			break;
		}
		case AP3216C_DATA_TYPE_PS: {
			status = pdm_sensor_ap3216c_read_ps(client, &value);
			break;
		}
		default: {
			OSA_ERROR("Invalid data type\n");
			status = -EINVAL;
			break;
		}
	}

	if (status) {
		OSA_ERROR("Read reg failed, type: %d, status: %d\n", type, status);
		return status;
	}

	*val = value;
	OSA_INFO("Read Reg type: %d, Value: %d\n", type, value);

	return 0;
}

static int pdm_sensor_ap3216c_write(struct pdm_client *client, unsigned int type, unsigned int val)
{
	OSA_INFO("\n");
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
static int pdm_sensor_ap3216c_setup(struct pdm_client *client)
{
	struct pdm_sensor_priv *sensor_priv;
	struct device_node *np;

	if (!client) {
		OSA_ERROR("Invalid client\n");
		return -EINVAL;
	}

	sensor_priv = pdm_client_get_private_data(client);
	if (!sensor_priv) {
		OSA_ERROR("Get PDM Client DevData Failed\n");
		return -ENOMEM;
	}
	sensor_priv->read = pdm_sensor_ap3216c_read;
	sensor_priv->write = pdm_sensor_ap3216c_write;

	np = pdm_client_get_of_node(client);
	if (!np) {
		OSA_ERROR("No DT node found\n");
		return -EINVAL;
	}


	OSA_DEBUG("ap3216c SENSOR Setup: %s\n", dev_name(&client->dev));
	return 0;
}

static void pdm_sensor_ap3216c_cleanup(struct pdm_client *client)
{
	return;
}

/**
 * @brief Match data structure for initializing SPI type SENSOR devices.
 */
const struct pdm_client_match_data pdm_sensor_ap3216c_match_data = {
	.setup = pdm_sensor_ap3216c_setup,
	.cleanup = pdm_sensor_ap3216c_cleanup,
};
