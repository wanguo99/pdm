#ifndef _PDM_SUBMODULE_H_
#define _PDM_SUBMODULE_H_

#include <linux/list.h>

// PDM 子驱动结构体定义
struct pdm_subdriver {
    const char *name;
    int (*init)(void);
    void (*exit)(void);
    struct list_head list;
};

// 公共函数声明
int pdm_submodule_register_drivers(void);
void pdm_submodule_unregister_drivers(void);

#endif /* _PDM_SUBMODULE_H_ */
