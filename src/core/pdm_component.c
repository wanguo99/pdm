#include "pdm.h"
#include "pdm_component.h"

/**
 * @brief Registers a single PDM component.
 *
 * This function registers a single PDM component by calling its initialization function. Only components that
 * successfully initialize are added to the component list. If initialization fails and ignore_failures is true,
 * registration continues with subsequent components without adding this one to the list.
 *
 * @param driver Pointer to the component structure to register.
 * @param list Pointer to the head of the component list.
 * @return 0 on success, negative error code on failure.
 */
static int pdm_component_register_single(struct pdm_component *driver, struct list_head *list)
{
    const char *name = driver->name ? driver->name : "Unknown";
    int status = 0;

    if (driver->enable && driver->init) {
        status = driver->init();
        if (status) {
            if (driver->ignore_failures) {
                OSA_WARN("Failed to register component <%s>, status = %d\n", name, status);
                return 0;
            } else {
                OSA_ERROR("Failed to register component <%s>, status = %d\n", name, status);
                return status;
            }
        }
    }

    list_add_tail(&driver->entry, list);
    return 0;
}


/**
 * @brief Unregisters a single PDM component.
 *
 * This function unregisters a single PDM component by calling its exit function and removing it from the
 * component list. Only components that were successfully registered and added to the list will be cleaned up.
 *
 * @param driver Pointer to the component structure to unregister.
 */
static void pdm_component_unregister_single(struct pdm_component *driver)
{
    list_del(&driver->entry);

    if (driver->enable && driver->exit) {
        driver->exit();
    }
}

/**
 * @brief Unregisters all components in the given list.
 *
 * This function unregisters all registered PDM components by calling their exit functions in reverse order.
 *
 * @param list Pointer to the head of the component list.
 */
void pdm_component_unregister(struct list_head *list)
{
    struct pdm_component *driver, *tmp;

    if (!list || list_empty(list)) {
        OSA_ERROR("Invalid or uninitialized list pointer\n");
        return;
    }

    list_for_each_entry_safe_reverse(driver, tmp, list, entry) {
        pdm_component_unregister_single(driver);
    }
}

/**
 * @brief Registers all components specified in the params structure and adds them to the list.
 *
 * This function registers all PDM components specified in the params structure by calling their initialization
 * functions in sequence.
 * If any component fails to initialize and ignore_failures is false, it will stop further registration and clean up
 * previously registered components.
 *
 * @param params Pointer to the pdm_component_params structure containing registration details.
 * @return 0 on success, negative error code on failure.
 */
int pdm_component_register(struct pdm_component_params *params)
{
    int i, status = 0;

    if (!params || !params->components || params->count <= 0 || !params->list) {
        OSA_ERROR("Invalid input parameters\n");
        return -EINVAL;
    }

    for (i = 0; i < params->count; i++) {
        status = pdm_component_register_single(&params->components[i], params->list);
        if (status) {
            pdm_component_unregister(params->list);
            return status;
        }
    }
    return 0;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM Component Module.");
