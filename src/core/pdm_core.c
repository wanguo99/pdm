#include <linux/debugfs.h>
#include <linux/proc_fs.h>
#include <linux/of_device.h>

#include "pdm.h"
#include "pdm_component.h"


/**
 * @brief 调试文件系统目录
 *
 * 该指针用于存储 PDM 调试文件系统的目录。
 */
static struct dentry *pdm_debugfs_dir;
static struct proc_dir_entry *pdm_procfs_dir;

/**
 * @brief PDM Core 组件列表
 *
 * 该列表用于存储所有注册的 PDM Core 组件。
 */
static struct list_head pdm_core_component_list;

/**
 * @brief 初始化 PDM 调试文件系统
 *
 * 该函数用于在 debugfs 和 procfs 中创建 PDM 相关的目录。
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
 * @brief 卸载 PDM 调试文件系统
 *
 * 该函数用于删除在 debugfs 和 procfs 中创建的 PDM 相关目录。
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
 * @brief PDM Core 组件数组
 *
 * 该数组包含所有需要注册的 PDM Core 组件。
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
};


/*                                                                              */
/*                              module_init                                     */
/*                                                                              */

/**
 * @brief 初始化 PDM 模块
 *
 * 该函数用于初始化 PDM 模块，包括注册总线、初始化主设备和子模块等。
 *
 * @return 成功返回 0，失败返回负错误码
 */
static int __init pdm_init(void)
{
    struct pdm_component_params params;
    int status;

    OSA_INFO("===== PDM Init =====\n");

    INIT_LIST_HEAD(&pdm_core_component_list);
    params.components = pdm_core_components;
    params.count = ARRAY_SIZE(pdm_core_components);
    params.list = &pdm_core_component_list;
    status = pdm_component_register(&params);
    if (status < 0) {
        OSA_ERROR("Failed to register PDM Core Component, error: %d.\n", status);
        return status;
    }

    OSA_INFO("----- PDM Inited ----- \n");
    return 0;
}

/**
 * @brief 退出 PDM 模块
 *
 * 该函数用于退出 PDM 模块，包括注销总线、卸载主设备和子模块等。
 */
static void __exit pdm_exit(void)
{
    OSA_INFO("===== PDM Exit =====\n");
    pdm_component_unregister(&pdm_core_component_list);
    OSA_INFO("----- PDM Exited -----\n");
}

module_init(pdm_init);
module_exit(pdm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<guohaoprc@163.com>");
MODULE_DESCRIPTION("PDM <Peripheral Driver Module>");
