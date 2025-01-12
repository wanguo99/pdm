#include <linux/i2c.h>
#include <linux/delay.h>

#include "pdm.h"
#include "pdm_sensor_priv.h"
#include "pdm_sensor_ioctl.h"

#define AP3216C_SYSTEMCONG	0x00	/* 配置寄存器 */
#define AP3216C_INTSTATUS	0x01	/* 中断状态寄存器 */
#define AP3216C_INTCLEAR	0x02	/* 中断清除寄存器 */

#define AP3216C_I2C_READ_MSG_COUNT	(2)	/* 读寄存器长度 */
#define AP3216C_RESET_DELAY_MS		(50)	/* 复位延迟时间(ms) */

/**
 * @brief Data type and register mapping structure.
 */
struct ap3216c_data_type_info {
	enum pdm_sensor_type type;
	unsigned char low_reg;
	unsigned char high_reg;
	bool special_case; // Indicates if the data needs special handling (e.g., masking)
};

static const struct ap3216c_data_type_info ap3216c_data_types[] = {
	{ PDM_SENSOR_TYPE_IR,  0x0A, 0x0B, true }, // IR
	{ PDM_SENSOR_TYPE_ALS, 0x0C, 0x0D, false }, // ALS
	{ PDM_SENSOR_TYPE_PS,  0x0E, 0x0F, true }, // PS
};

/**
 * @brief Reads a single register from the AP3216C sensor.
 */
static int pdm_sensor_ap3216c_read_reg(struct pdm_client *client, unsigned char reg, unsigned char *val, size_t len)
{
	struct i2c_msg msgs[AP3216C_I2C_READ_MSG_COUNT] = {
		{ .addr = client->hardware.i2c.client->addr, .flags = 0, .buf = &reg, .len = sizeof(reg) },
		{ .addr = client->hardware.i2c.client->addr, .flags = I2C_M_RD, .buf = val, .len = len }
	};
	return (AP3216C_I2C_READ_MSG_COUNT == i2c_transfer(client->hardware.i2c.client->adapter, msgs, AP3216C_I2C_READ_MSG_COUNT)) ? 0 : -EREMOTEIO;
}

/**
 * @brief Writes a single byte to a specified register of the AP3216C sensor.
 */
static int pdm_sensor_ap3216c_write_reg(struct pdm_client *client, unsigned char reg, unsigned char val)
{
	unsigned char buf[] = { reg, val };
	struct i2c_msg msg = { .addr = client->hardware.i2c.client->addr, .flags = 0, .buf = buf, .len = sizeof(buf) };
	return (1 == i2c_transfer(client->hardware.i2c.client->adapter, &msg, 1)) ? 0 : -EREMOTEIO;
}

/**
 * @brief Enables the AP3216C sensor by writing configuration values and ensuring reset delay.
 */
static int pdm_sensor_ap3216c_enable(struct pdm_client *client)
{
	int status;

	status = pdm_sensor_ap3216c_write_reg(client, AP3216C_SYSTEMCONG, 0x04);
	if (status) {
		OSA_ERROR("Failed to write reset value to SYSTEMCONG register: %d\n", status);
		return status;
	}
	mdelay(AP3216C_RESET_DELAY_MS); // Ensure reset delay

	status = pdm_sensor_ap3216c_write_reg(client, AP3216C_SYSTEMCONG, 0x03);
	if (status) {
		OSA_ERROR("Failed to write enable value to SYSTEMCONG register: %d\n", status);
		return status;
	}

	OSA_DEBUG("AP3216C SENSOR Enabled.\n");
	return 0;
}

/**
 * @brief Reads data from the AP3216C sensor based on the specified type info.
 */
static int pdm_sensor_ap3216c_read(struct pdm_client *client, unsigned int type, unsigned int *val)
{
	const struct ap3216c_data_type_info *info = NULL;
	int status = -EINVAL;
	unsigned short value;
	unsigned char data_low, data_high;

	for (size_t i = 0; i < ARRAY_SIZE(ap3216c_data_types); ++i) {
		if (ap3216c_data_types[i].type == type) {
			info = &ap3216c_data_types[i];
			break;
		}
	}

	if (!info) {
		OSA_ERROR("Invalid data type\n");
		return -EINVAL;
	}

	status = pdm_sensor_ap3216c_read_reg(client, info->low_reg, &data_low, sizeof(data_low));
	if (status) {
		OSA_ERROR("read reg low_data failed, status: %d\n", status);
		return status;
	}

	status = pdm_sensor_ap3216c_read_reg(client, info->high_reg, &data_high, sizeof(data_high));
	if (status) {
		OSA_ERROR("read reg high_data failed, status: %d\n", status);
		return status;
	}

	if (info->special_case && ((data_low & 0x80) || (data_low & 0x40))) {
		value = 0;
	} else {
		value = ((unsigned short)data_high << 8) | data_low;
		if (info->special_case) {
			value &= 0x3FF;
		}
	}

	*val = value;
	OSA_INFO("Read Reg type: %d, Value: %d\n", type, value);

	return 0;
}

/**
 * @brief Initializes the AP3216C sensor settings.
 */
static int pdm_sensor_ap3216c_setup(struct pdm_client *client)
{
	struct pdm_sensor_priv *sensor_priv = pdm_client_get_private_data(client);
	const struct device_node *np = pdm_client_get_of_node(client);
	int status;

	if (!client || !sensor_priv || !np) {
		OSA_ERROR("Invalid parameters\n");
		return -EINVAL;
	}

	sensor_priv->read = pdm_sensor_ap3216c_read;
	client->hardware.i2c.client = to_i2c_client(client->pdmdev->dev.parent);

	status = pdm_sensor_ap3216c_enable(client);
	if (status) {
		OSA_ERROR("Failed to enable AP3216C sensor: %d\n", status);
		return status;
	}

	OSA_DEBUG("PDM SENSOR Setup: %s\n", dev_name(&client->dev));

	return 0;
}

const struct pdm_client_match_data pdm_sensor_ap3216c_match_data = {
	.setup = pdm_sensor_ap3216c_setup,
	.cleanup = NULL,
};
