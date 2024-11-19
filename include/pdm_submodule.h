#ifndef _PDM_SUBMODULE_H_
#define _PDM_SUBMODULE_H_

/*                                                                              */
/*                                公共数据类型定义                                      */
/*                                                                              */

// 定义 PDM 子驱动结构体
struct pdm_subdriver {
    const char *name;
    int (*init)(void);          // 初始化函数指针
    void (*exit)(void);         // 退出函数指针
    struct list_head list;      // 用于链表管理
};



/*                                                                              */
/*                                公共函数声明                                        */
/*                                                                              */
int pdm_submodule_register_drivers(void);
void pdm_submodule_unregister_drivers(void);

#endif /* _PDM_SUBMODULE_H_ */
