#include <linux/debugfs.h>
#include <linux/proc_fs.h>
#include <linux/of_device.h>

#include "pdm.h"
#include "pdm_component.h"

/**
 * @brief DebugFS and ProcFS directory name.
 */
#define PDM_DEBUG_FS_DIR_NAME    "pdm"         /**< Name of debugfs and procfs directories */

static int pdm_bus_debug_fs_init(void);
static void pdm_bus_debug_fs_exit(void);

/**
 * @struct pdm_core_component_list
 * @brief List to store all registered PDM Core components.
 */
static struct list_head pdm_core_component_list;

/**
 * @struct pdm_core_components
 * @brief Array containing all PDM Core components that need to be registered.
 */
static struct pdm_component pdm_core_components[] = {
    {
        .name = "Debug Filesystem",
        .status = true,
        .ignore_failures = true,
        .init = pdm_bus_debug_fs_init,
        .exit = pdm_bus_debug_fs_exit,
    },
    {
        .name = "PDM Bus",
        .status = true,
        .ignore_failures = false,
        .init = pdm_bus_init,
        .exit = pdm_bus_exit,
    },
    {
        .name = "PDM Device",
        .status = true,
        .ignore_failures = false,
        .init = pdm_device_init,
        .exit = pdm_device_exit,
    },
    {
        .name = "PDM Client",
        .status = true,
        .ignore_failures = false,
        .init = pdm_client_init,
        .exit = pdm_client_exit,
    },
    {
        .name = "PDM Adapter",
        .status = true,
        .ignore_failures = false,
        .init = pdm_adapter_init,
        .exit = pdm_adapter_exit,
    },
    { }
};

/**
 * @brief Pointer to the PDM debug filesystem directory.
 */
static struct dentry *pdm_debugfs_dir;
static struct proc_dir_entry *pdm_procfs_dir;

/**
 * @brief Initializes the PDM debugging filesystem.
 *
 * This function creates the necessary directories in debugfs and procfs for PDM.
 *
 * @return 0 on success, negative error code on failure.
 */
static int pdm_bus_debug_fs_init(void)
{
    pdm_debugfs_dir = debugfs_create_dir(PDM_DEBUG_FS_DIR_NAME, NULL);
    if (IS_ERR(pdm_debugfs_dir)) {
        OSA_WARN("Failed to register PDM debugfs, error %ld\n", PTR_ERR(pdm_debugfs_dir));
        pdm_debugfs_dir = NULL;  // Set to NULL to indicate failure
    } else {
        OSA_INFO("PDM debugfs registered\n");
    }

    pdm_procfs_dir = proc_mkdir(PDM_DEBUG_FS_DIR_NAME, NULL);
    if (!pdm_procfs_dir) {
        OSA_WARN("Failed to register PDM procfs\n");
        if (pdm_debugfs_dir) {
            debugfs_remove_recursive(pdm_debugfs_dir);
            pdm_debugfs_dir = NULL;
        }
    } else {
        OSA_INFO("PDM procfs registered\n");
    }

    return 0;
}

/**
 * @brief Unregisters the PDM debugging filesystem.
 *
 * This function removes the directories created in debugfs and procfs for PDM.
 */
static void pdm_bus_debug_fs_exit(void)
{
    if (pdm_debugfs_dir) {
        debugfs_remove_recursive(pdm_debugfs_dir);
        OSA_INFO("PDM debugfs unregistered\n");
    }
    if (pdm_procfs_dir) {
        remove_proc_entry(PDM_DEBUG_FS_DIR_NAME, NULL);
        OSA_INFO("PDM procfs unregistered\n");
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
static void pdm_show_module_info(void)
{
    OSA_INFO("----------------------------------\n");
    OSA_INFO("| Name   : %s\n", PDM_MODULE_NAME);
    OSA_INFO("| Build  : %s\n", PDM_MODULE_BUILD_TIME);
    OSA_INFO("| Version: %s\n", PDM_MODULE_VERSIONS);
    OSA_INFO("----------------------------------\n");
}

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

    OSA_print("\n");
    OSA_INFO("===== PDM Init =====\n");

    pdm_show_module_info();

    INIT_LIST_HEAD(&pdm_core_component_list);
    status = pdm_component_register(&params);
    if (status < 0) {
        OSA_ERROR("Failed to register PDM Core Component, error: %d.\n", status);
        return status;
    }

    OSA_INFO("----- PDM Inited -----\n");
    return 0;
}

/**
 * @brief Exits the PDM module.
 *
 * This function exits the PDM module, including unregistering the bus, unloading main devices, and submodules.
 */
static void __exit pdm_exit(void)
{
    OSA_print("\n");
    OSA_INFO("===== PDM Exit =====\n");
    pdm_component_unregister(&pdm_core_component_list);
    OSA_INFO("----- PDM Exited -----\n");
}

module_init(pdm_init);
module_exit(pdm_exit);

MODULE_VERSION(PDM_MODULE_VERSIONS);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM <Peripheral Driver Module>");
