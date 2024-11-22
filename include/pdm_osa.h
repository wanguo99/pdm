#ifndef _PDM_OSA_H_
#define _PDM_OSA_H_

/*
 * 公共日志打印接口
 */

// 提取文件名
// 从文件路径中提取文件名部分
#define BASENAME(file) (strrchr(file, '/') ? strrchr(file, '/') + 1 : file)

// 获取当前文件的基本名称
#define FILE_BASENAME (BASENAME(__FILE__))

// 定义日志格式宏
// 用于在日志消息前添加模块名称
#define OSA_fmt(fmt) "[" KBUILD_MODNAME "]: " fmt

// 不带代码信息的打印
// 用于简单的错误日志打印，不包含文件名、行号和函数名
#define OSA_print(fmt, ...) \
    printk(KERN_ERR OSA_fmt(fmt), ##__VA_ARGS__)

// 标准日志打印接口
// 用于错误日志打印，包含文件名、行号和函数名
#define OSA_ERROR(fmt, ...) \
    printk(KERN_ERR OSA_fmt("[ERROR] %s:%d %s(): " fmt), FILE_BASENAME, __LINE__, __func__, ##__VA_ARGS__)

// 用于警告日志打印，包含文件名、行号和函数名
#define OSA_WARN(fmt, ...) \
    printk(KERN_WARNING OSA_fmt("[WARNING] %s:%d %s(): " fmt), FILE_BASENAME, __LINE__, __func__, ##__VA_ARGS__)

// 用于信息日志打印，包含文件名、行号和函数名
#define OSA_INFO(fmt, ...) \
    printk(KERN_INFO OSA_fmt("[INFO] %s:%d %s(): " fmt), FILE_BASENAME, __LINE__, __func__, ##__VA_ARGS__)

// 用于调试日志打印，包含文件名、行号和函数名
#define OSA_DEBUG(fmt, ...) \
    printk(KERN_DEBUG OSA_fmt("[DEBUG] %s:%d %s(): " fmt), FILE_BASENAME, __LINE__, __func__, ##__VA_ARGS__)

// 变量的名称和值打印接口
// 用于打印整型变量的名称和值
#define OSA_VAR_INT(var) \
    OSA_INFO(#var " = %d\n", var)

// 用于打印无符号整型变量的名称和值
#define OSA_VAR_UINT(var) \
    OSA_INFO(#var " = %u\n", var)

// 用于打印长整型变量的名称和值
#define OSA_VAR_LONG(var) \
    OSA_INFO(#var " = %ld\n", var)

// 用于打印无符号长整型变量的名称和值
#define OSA_VAR_ULONG(var) \
    OSA_INFO(#var " = %lu\n", var)

// 用于打印浮点型变量的名称和值
#define OSA_VAR_FLOAT(var) \
    OSA_INFO(#var " = %f\n", var)

// 用于打印双精度浮点型变量的名称和值
#define OSA_VAR_DOUBLE(var) \
    OSA_INFO(#var " = %lf\n", var)

// 用于打印字符型变量的名称和值
#define OSA_VAR_CHAR(var) \
    OSA_INFO(#var " = %c\n", var)

// 用于打印字符串变量的名称和值
#define OSA_VAR_STRING(var) \
    OSA_INFO(#var " = %s\n", var)

// 用于打印指针变量的名称和值
#define OSA_VAR_PTR(var) \
    OSA_INFO(#var " = %p\n", var)

#endif /* _PDM_OSA_H_ */
