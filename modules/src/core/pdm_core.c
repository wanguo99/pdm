#include <linux/debugfs.h>
#include <linux/proc_fs.h>
#include <linux/of_device.h>

#include "pdm.h"
#include "pdm_component.h"

/**
 * @struct pdm_core_component_list
 * @brief List to store all registered PDM Core components.
 */
static struct list_head pdm_core_component_list;

/**
 * @brief Pointer to the PDM debug filesystem directory.
 */
static struct dentry *pdm_debugfs_dir;

/**
 * @brief Initializes the PDM debugging filesystem.
 *
 * This function creates the necessary directories in debugfs for PDM.
 *
 * @return 0 on success, negative error code on failure.
 */
static int pdm_bus_debug_fs_init(void)
{
	pdm_debugfs_dir = debugfs_create_dir(PDM_MODULE_NAME, NULL);
	if (IS_ERR_OR_NULL(pdm_debugfs_dir)) {
		OSA_WARN("Failed to register PDM debugfs, error %ld\n", PTR_ERR(pdm_debugfs_dir));
		return PTR_ERR(pdm_debugfs_dir);
	}

	OSA_DEBUG("PDM debugfs registered\n");
	return 0;
}

/**
 * @brief Unregisters the PDM debugging filesystem.
 *
 * This function removes the directories created in debugfs for PDM.
 */
static void pdm_bus_debug_fs_exit(void)
{
	if (!IS_ERR_OR_NULL(pdm_debugfs_dir)) {
		debugfs_remove_recursive(pdm_debugfs_dir);
		OSA_DEBUG("PDM debugfs unregistered\n");
	}
}

static struct proc_dir_entry *pdm_procfs_dir;

/**
 * @brief Initializes the PDM proc filesystem.
 *
 * This function creates the necessary directories in procfs for PDM.
 *
 * @return 0 on success, negative error code on failure.
 */
static int pdm_bus_proc_fs_init(void)
{
	pdm_procfs_dir = proc_mkdir(PDM_MODULE_NAME, NULL);
	if (!pdm_procfs_dir) {
		OSA_WARN("Failed to register PDM procfs\n");
		return -ENOMEM;
	}

	OSA_DEBUG("PDM procfs registered\n");
	return 0;
}

/**
 * @brief Unregisters the PDM proc filesystem.
 *
 * This function removes the directories created in procfs for PDM.
 */
static void pdm_bus_proc_fs_exit(void)
{
	if (pdm_procfs_dir) {
		remove_proc_entry(PDM_MODULE_NAME, NULL);
		OSA_DEBUG("PDM procfs unregistered\n");
	}
}

/**
 * @brief Displays module information, including name, build time, and version.
 *
 * This function logs the module's name, build time, and version using the OSA_INFO macro.
 * The information displayed includes:
 * - Module name (from KBUILD_MODNAME)
 * - Build time (from KBUILD_MODULE_BUILD_TIME)
 * - Version (from KBUILD_MODULE_VERSION)
 *
 * This function does not modify any state or interact with the filesystem.
 */
static void pdm_show_module_init_info(void)
{
#if 0
	OSA_print("\n");
	OSA_INFO("\t========== Module Loading ==========\n");
	OSA_INFO("\t| NAME   : %s\n", PDM_MODULE_NAME);
	OSA_INFO("\t| BUILD  : %s\n", PDM_MODULE_BUILD_TIME);
	OSA_INFO("\t| VERSION: %s\n", PDM_MODULE_VERSIONS);
	OSA_INFO("\t------------------------------------\n");
#endif
}

static void pdm_show_module_exit_info(void)
{
#if 0
	OSA_print("\n");
	OSA_INFO("\t========== Module Removed ==========\n");
	OSA_INFO("\t| NAME   : %s\n", PDM_MODULE_NAME);
	OSA_INFO("\t| BUILD  : %s\n", PDM_MODULE_BUILD_TIME);
	OSA_INFO("\t| VERSION: %s\n", PDM_MODULE_VERSIONS);
	OSA_INFO("\t------------------------------------\n");
#endif
}

/**
 * @struct pdm_core_components
 * @brief Array containing all PDM Core components that need to be registered.
 */
static struct pdm_component pdm_core_components[] = {
	{
		.name = "Debug Filesystem",
		.enable = true,
		.ignore_failures = true,
		.init = pdm_bus_debug_fs_init,
		.exit = pdm_bus_debug_fs_exit,
	},
	{
		.name = "Proc Filesystem",
		.enable = true,
		.ignore_failures = true,
		.init = pdm_bus_proc_fs_init,
		.exit = pdm_bus_proc_fs_exit,
	},
	{
		.name = "PDM Bus",
		.enable = true,
		.ignore_failures = false,
		.init = pdm_bus_init,
		.exit = pdm_bus_exit,
	},
	{
		.name = "PDM Device",
		.enable = true,
		.ignore_failures = false,
		.init = pdm_device_init,
		.exit = pdm_device_exit,
	},
	{
		.name = "PDM Client",
		.enable = true,
		.ignore_failures = false,
		.init = pdm_client_init,
		.exit = pdm_client_exit,
	},
	{
		.name = "PDM Adapter",
		.enable = true,
		.ignore_failures = false,
		.init = pdm_adapter_init,
		.exit = pdm_adapter_exit,
	},
	{  .name = NULL }
};

/**
 * @brief Initializes the PDM module.
 *
 * This function initializes the PDM module, including registering the bus, initializing main devices, and submodules.
 *
 * @return 0 on success, negative error code on failure.
 */
static int __init pdm_init(void)
{
	int status;
	struct pdm_component_params params = {
		.components = pdm_core_components,
		.count = ARRAY_SIZE(pdm_core_components),
		.list = &pdm_core_component_list,
	};

	pdm_show_module_init_info();
	INIT_LIST_HEAD(&pdm_core_component_list);
	status = pdm_component_register(&params);
	if (status < 0) {
		OSA_ERROR("Failed to register PDM Core Component, error: %d\n", status);
		return status;
	}

	return 0;
}

/**
 * @brief Exits the PDM module.
 *
 * This function exits the PDM module, including unregistering the bus, unloading main devices, and submodules.
 */
static void __exit pdm_exit(void)
{
	pdm_show_module_exit_info();
	pdm_component_unregister(&pdm_core_component_list);
}

module_init(pdm_init);
module_exit(pdm_exit);

MODULE_VERSION(PDM_MODULE_VERSIONS);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM <Peripheral Driver Module>");
