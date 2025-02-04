#include <linux/compat.h>
#include "pdm.h"

/**
 * @brief Unregisters a PDM Client managed by devres.
 *
 * This function safely unregisters a PDM Client device using devres management, ensuring all resources are properly released.
 *
 * @param dev Pointer to the parent device structure.
 * @param res Pointer to the resource data.
 */
static void devm_pdm_client_unregister(void *data)
{
	struct pdm_client *client = data;

	if ((!client) || (!client->adapter)) {
		OSA_ERROR("Invalid input parameters (adapter: %p, client: %p)\n", client->adapter, client);
		return;
	}

	mutex_lock(&client->adapter->client_list_mutex_lock);
	list_del(&client->entry);
	mutex_unlock(&client->adapter->client_list_mutex_lock);

	pdm_adapter_id_free(client->adapter, client);
	pdm_adapter_put(client->adapter);
}

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
int devm_pdm_client_register(struct pdm_adapter *adapter, struct pdm_client *client)
{
	int status;

	if (!adapter || !client) {
		OSA_ERROR("Invalid input parameters (adapter: %p, client: %p)\n", adapter, client);
		return -EINVAL;
	}

	if (!pdm_adapter_get(adapter)) {
		OSA_ERROR("Failed to get adapter\n");
		return -EBUSY;
	}

	status = pdm_adapter_id_alloc(adapter, client);
	if (status) {
		OSA_ERROR("Alloc id for client failed: %d\n", status);
		goto err_put_adapter;
	}

	client->adapter = adapter;
	snprintf(client->name, PDM_CLIENT_NAME_MAX_LEN, "%s-%d", dev_name(&adapter->dev), client->index);

	mutex_lock(&adapter->client_list_mutex_lock);
	list_add_tail(&client->entry, &adapter->client_list);
	mutex_unlock(&adapter->client_list_mutex_lock);

	status = devm_add_action_or_reset(&client->pdmdev->dev, devm_pdm_client_unregister, client);
	if (status) {
		OSA_ERROR("Failed to add devres, error: %d\n", status);
		goto err_free_id;
	}

	return 0;

err_free_id:
	pdm_adapter_id_free(adapter, client);
err_put_adapter:
	pdm_adapter_put(adapter);
	return status;
}


/**
 * @brief Frees a PDM Client managed by devres.
 *
 * This function drops the reference to the PDM Client and ensures it is properly freed when no longer needed.
 *
 * @param dev Pointer to the parent device structure.
 * @param res Pointer to the resource data.
 */
static void devm_pdm_client_free(void *data)
{
	kfree((struct pdm_client *)data);
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
struct pdm_client *devm_pdm_client_alloc(struct pdm_device *pdmdev, unsigned int data_size)
{
	struct pdm_client *client;
	unsigned int client_size = sizeof(struct pdm_client);
	unsigned int total_size = ALIGN(client_size + data_size, 8);

	if (!pdmdev) {
		OSA_ERROR("Invalid pdm_device pointer\n");
		return ERR_PTR(-EINVAL);
	}

	client = kzalloc(total_size, GFP_KERNEL);
	if (!client) {
		OSA_ERROR("Failed to allocate memory for pdm_client\n");
		return ERR_PTR(-ENOMEM);
	}

	pdmdev->client = client;
	client->pdmdev = pdmdev;
	if (data_size) {
		pdm_client_set_private_data(client, (void *)(client + client_size));
	}

	if (devm_add_action_or_reset(&pdmdev->dev, devm_pdm_client_free, client)) {
		return ERR_PTR(-ENOMEM);
	}

	return client;
}

/**
 * @brief Retrieves match data for a PDM device from the device tree.
 *
 * This function looks up the device tree to find matching data for the given PDM device,
 * which can be used for initialization or configuration.
 *
 * @param pdmdev Pointer to the PDM device structure.
 * @return Pointer to the match data if found; NULL otherwise.
 */
const void *pdm_client_get_match_data(struct pdm_client *client)
{
	const struct of_device_id *match;

	if (!client || !client->pdmdev || !client->pdmdev->dev.driver) {
		return NULL;
	}
	if (!client->pdmdev->dev.driver->of_match_table || !client->pdmdev->dev.parent) {
		return NULL;
	}

	match = of_match_device(client->pdmdev->dev.driver->of_match_table, client->pdmdev->dev.parent);
	if (!match) {
		return NULL;
	}
	return match->data;
}

/**
 * @brief Retrieves the device tree node for a PDM device's parent device.
 *
 * This function retrieves the device tree node associated with the parent device of the given PDM device.
 * It can be used to access properties or subnodes defined in the device tree for the parent device.
 *
 * @param pdmdev Pointer to the PDM device structure.
 * @return Pointer to the device_node structure if found; NULL otherwise.
 */
struct device_node *pdm_client_get_of_node(struct pdm_client *client)
{
	if (!client || !client->pdmdev || !client->pdmdev->dev.parent) {
		return NULL;
	}
	return dev_of_node(client->pdmdev->dev.parent);
}

/**
 * @brief Setup a PDM device.
 *
 * @param pdmdev Pointer to the PDM device structure.
 * @return 0 on success, negative error code on failure.
 */
int pdm_client_setup(struct pdm_client *client)
{
	const struct pdm_client_match_data *match_data;
	int status;

	match_data = pdm_client_get_match_data(client);
	if (!match_data) {
		OSA_DEBUG("Failed to get match data for device: %s\n", client->name);
		return 0;
	}

	if (match_data->setup) {
		status = match_data->setup(client);
		if (status) {
			OSA_ERROR("PDM Device Setup Failed, status=%d\n", status);
			return status;
		}
	}

	return status;
}

/**
 * @brief Cleanup a PDM device.
 *
 * @param pdmdev Pointer to the PDM device structure.
 */
void pdm_client_cleanup(struct pdm_client *client)
{
	const struct pdm_client_match_data *match_data;

	match_data = pdm_client_get_match_data(client);
	if (!match_data) {
		OSA_ERROR("Failed to get match data for device\n");
		return;
	}

	if (match_data->cleanup) {
		match_data->cleanup(client);
	}
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Client Module");
