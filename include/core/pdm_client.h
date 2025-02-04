#ifndef _PDM_CLIENT_H_
#define _PDM_CLIENT_H_

/**
 * @brief PDM Client device name.
 *
 * This is the name used to identify the PDM Client device in the system.
 */
#define PDM_CLIENT_DEVICE_NAME			"pdm_client"
#define PDM_CLIENT_NAME_MAX_LEN			(32)
/**
 * @brief Minor count for PDM Client devices.
 *
 * Defines the number of minor devices allowed for PDM Client.
 * MINORMASK is the upper limit minus 1.
 */
#define PDM_CLIENT_MINORS			(MINORMASK + 1)

/**
 * @struct pdm_client_match_data
 * @brief Match data structure for initializing specific types of PDM Client.
 *
 * This structure contains setup and cleanup function pointers for initializing
 * and cleaning up specific types of PDM Client.
 */
struct pdm_client_match_data {
	int (*setup)(struct pdm_client *client);
	void (*cleanup)(struct pdm_client *client);
};

/**
 * @struct pdm_device_gpio_data
 * @brief Data structure for GPIO-controlled PDM Devices.
 *
 * This structure holds the necessary data for controlling an device via GPIO.
 */
struct pdm_client_gpio {
	struct gpio_desc *gpiod;
};

/**
 * @struct pdm_device_pwm_data
 * @brief Data structure for PWM-controlled PDM Devices.
 *
 * This structure holds the necessary data for controlling an device via PWM.
 */
struct pdm_client_pwm {
	struct pwm_device *pwmdev;
};

/**
 * @struct pdm_device_spi_data
 * @brief Data structure for SPI-controlled PDM Devices.
 *
 * This structure holds the necessary data for controlling an device via SPI.
 */
struct pdm_client_spi {
	struct spi_device *spidev;
};

/**
 * @struct pdm_device_i2c_data
 * @brief Data structure for I2C-controlled PDM Devices.
 *
 * This structure holds the necessary data for controlling an device via I2C.
 */
struct pdm_client_i2c {
	struct i2c_client *client;
};

/**
* @union pdm_device_hw_data
 * @brief Union to hold hardware-specific data for different types of device controls.
 *
 * This union allows the same structure to accommodate different types of device control
 * mechanisms (e.g., GPIO or PWM).
 */
union pdm_client_hardware {
	struct pdm_client_gpio  gpio;
	struct pdm_client_pwm   pwm;
	struct pdm_client_spi   spi;
	struct pdm_client_i2c   i2c;
};

/**
 * @brief PDM Client structure.
 *
 * This structure defines the basic information for a PDM Client device,
 * which is a device that interacts with a PDM (Pulse Density Modulation) adapter.
 */
struct pdm_client {
	int index;				/**< Client ID allocated by the adapter */
	char name[PDM_CLIENT_NAME_MAX_LEN];	/**< PDM Client name */
	bool force_dts_id;			/**< Flag indicating whether to force ID from Device Tree Source (DTS) */
	struct pdm_adapter *adapter;		/**< Pointer to the owning PDM Adapter */
	struct pdm_device *pdmdev;		/**< Pointer to the PDM Device */
	struct list_head entry;			/**< List node for linking devices in a linked list */
	struct regmap *map;			/**< PDM Client regmap handle. */
	union pdm_client_hardware hardware;	 /**< PDM Client hardware information. */
	void *priv_data;			/**< PDM Client private data. */
};

/**
 * @brief Structure to hold device resources for pdm_client.
 *
 * This structure is used for managing the resources associated with a pdm_client
 * device in a devres (device resource) context.
 */
struct pdm_client_devres {
	struct pdm_client *client;			/**< Pointer to the associated pdm_client structure */
};


/**
 * @brief Retrieves the driver data associated with the device.
 *
 * This function retrieves the driver data stored in the device structure.
 *
 * @param client Pointer to the PDM Client structure.
 * @return Pointer to the driver data stored in the device structure.
 */
static inline void *pdm_client_get_private_data(struct pdm_client *client)
{
	if (!client) {
		return NULL;
	}
	return client->priv_data;

}

/**
 * @brief Sets the driver data associated with the device.
 *
 * This function sets the driver data in the device structure.
 *
 * @param client Pointer to the PDM Client structure.
 * @param data Pointer to the driver data to be set.
 */
static inline void pdm_client_set_private_data(struct pdm_client *client, void *data)
{
	client->priv_data = data;
}

/**
 * @brief Allocates and initializes a pdm_client structure, along with its associated resources.
 *
 * This function allocates memory for a new PDM client, initializes the structure, and returns
 * a pointer to the newly allocated pdm_client.
 *
 * @param pdmdev Pointer to the PDM device structure to which the client is associated.
 * @param data_size Size of additional data to allocate for the client.
 * @return Pointer to the newly allocated pdm_client structure, or ERR_PTR on failure.
 */
struct pdm_client *devm_pdm_client_alloc(struct pdm_device *pdmdev, unsigned int data_size);

/**
 * @brief Setup a PDM client.
 *
 * @param pdmdev Pointer to the PDM client structure.
 * @return 0 on success, negative error code on failure.
 */
int pdm_client_setup(struct pdm_client *client);

/**
 * @brief Cleanup a PDM device.
 *
 * @param pdmdev Pointer to the PDM device structure.
 */
void pdm_client_cleanup(struct pdm_client *client);

/**
 * @brief Retrieves match data for a PDM device from the device tree.
 *
 * This function looks up the device tree to find matching data for the given PDM device,
 * which can be used for initialization or configuration.
 *
 * @param pdmdev Pointer to the PDM device structure.
 * @return Pointer to the match data if found; NULL otherwise.
 */
const void *pdm_client_get_match_data(struct pdm_client *client);

/**
 * @brief Retrieves the device tree node for a PDM client's parent device.
 *
 * This function retrieves the device tree node associated with the parent device of the given PDM device.
 * It can be used to access properties or subnodes defined in the device tree for the parent device.
 *
 * @param client Pointer to the PDM client structure.
 * @return Pointer to the device_node structure if found; NULL otherwise.
 */
struct device_node *pdm_client_get_of_node(struct pdm_client *client);

/**
 * @brief Registers a PDM client with the associated PDM adapter.
 *
 * This function registers the PDM client with the PDM adapter, setting up the necessary
 * resources and file operations, and linking the client to the adapter's client list.
 *
 * @param adapter Pointer to the PDM adapter structure.
 * @param client Pointer to the PDM client structure.
 * @return 0 on success, negative error code on failure.
 */
int devm_pdm_client_register(struct pdm_adapter *adapter, struct pdm_client *client);

/**
 * @brief Initializes the PDM Client module.
 *
 * This function initializes the PDM Client module, including registering necessary drivers
 * and setting initial states for the client devices.
 *
 * @return 0 on success, negative error code on failure.
 */
int pdm_client_init(void);

/**
 * @brief Exits the PDM Client module.
 *
 * This function cleans up all resources related to the PDM Client module, including unregistering
 * drivers and releasing any allocated memory or resources.
 */
void pdm_client_exit(void);

#endif /* _PDM_CLIENT_H_ */
