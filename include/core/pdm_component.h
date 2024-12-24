#ifndef _PDM_COMPONENT_H_
#define _PDM_COMPONENT_H_

/**
 * @file pdm_component.h
 * @brief PDM Component Management Interface.
 *
 * This file defines the interfaces and structures for managing PDM components, including registration and
 * unregistration of drivers.
 */

#include <linux/list.h>

/**
 * @struct pdm_component
 * @brief PDM Component structure definition.
 *
 * This structure defines the basic information and operation functions for a PDM component.
 */
struct pdm_component {
    bool enable;                /**< Whether the driver need loaded. Default is false. Set to true to enable. */
    bool ignore_failures;       /**< Whether to ignore initialization failures. */
    const char *name;           /**< Name of the component. */
    int (*init)(void);          /**< Initialization function for the component. */
    void (*exit)(void);         /**< Exit function for the component. */
    struct list_head entry;      /**< List node for management in a linked entry. */
};

/**
 * @struct pdm_component_params
 * @brief Structure for encapsulating all parameters required for component registration.
 *
 * This structure is used to pass an array of components and other necessary parameters to the registration function.
 */
struct pdm_component_params {
    struct pdm_component *components;  /**< Array of components to register. */
    int count;                         /**< Length of the components array. */
    struct list_head *list;            /**< Pointer to the head of the component list. */
};

/**
 * @brief Unregisters all components from the given list.
 *
 * This function unregisters all registered PDM components by calling their exit functions in sequence.
 *
 * @param list Pointer to the head of the component list.
 */
void pdm_component_unregister(struct list_head *list);

/**
 * @brief Registers all components specified in the params structure and adds them to the list.
 *
 * This function registers all PDM components specified in the params structure by calling their initialization
 * functions in sequence.
 *
 * @param params Pointer to the pdm_component_params structure containing registration details.
 * @return 0 on success, negative error code on failure.
 */
int pdm_component_register(struct pdm_component_params *params);

#endif /* _PDM_COMPONENT_H_ */
