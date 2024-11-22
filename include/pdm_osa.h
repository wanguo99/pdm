#ifndef _PDM_OSA_H_
#define _PDM_OSA_H_

/**
 * @file pdm_osa.h
 * @brief 公共日志打印接口
 *
 * 本文件定义了一系列用于日志打印的宏，这些宏可以在内核模块中使用，
 * 以便在不同级别的日志中包含模块名称、文件名、行号和函数名等信息。
 */

/**
 * @brief 从文件路径中提取文件名部分
 *
 * 该宏从给定的文件路径中提取文件名部分。
 *
 * @param file 文件路径
 * @return 文件名部分
 */
#define BASENAME(file) (strrchr(file, '/') ? strrchr(file, '/') + 1 : file)

/**
 * @brief 获取当前文件的基本名称
 *
 * 该宏获取当前文件的基本名称（即不包含路径的部分）。
 */
#define FILE_BASENAME (BASENAME(__FILE__))

/**
 * @brief 定义日志格式宏
 *
 * 该宏用于在日志消息前添加模块名称。
 *
 * @param fmt 日志格式字符串
 */
#define OSA_fmt(fmt) "[" KBUILD_MODNAME "]: " fmt

/**
 * @brief 不带代码信息的打印
 *
 * 该宏用于简单的错误日志打印，不包含文件名、行号和函数名。
 *
 * @param fmt 日志格式字符串
 * @param ... 可变参数列表
 */
#define OSA_print(fmt, ...) \
    printk(KERN_ERR OSA_fmt(fmt), ##__VA_ARGS__)

/**
 * @brief 标准日志打印接口（错误级别）
 *
 * 该宏用于错误日志打印，包含文件名、行号和函数名。
 *
 * @param fmt 日志格式字符串
 * @param ... 可变参数列表
 */
#define OSA_ERROR(fmt, ...) \
    printk(KERN_ERR OSA_fmt("[ERROR] %s:%d %s(): " fmt), FILE_BASENAME, __LINE__, __func__, ##__VA_ARGS__)

/**
 * @brief 标准日志打印接口（警告级别）
 *
 * 该宏用于警告日志打印，包含文件名、行号和函数名。
 *
 * @param fmt 日志格式字符串
 * @param ... 可变参数列表
 */
#define OSA_WARN(fmt, ...) \
    printk(KERN_WARNING OSA_fmt("[WARNING] %s:%d %s(): " fmt), FILE_BASENAME, __LINE__, __func__, ##__VA_ARGS__)

/**
 * @brief 标准日志打印接口（信息级别）
 *
 * 该宏用于信息日志打印，包含文件名、行号和函数名。
 *
 * @param fmt 日志格式字符串
 * @param ... 可变参数列表
 */
#define OSA_INFO(fmt, ...) \
    printk(KERN_INFO OSA_fmt("[INFO] %s:%d %s(): " fmt), FILE_BASENAME, __LINE__, __func__, ##__VA_ARGS__)

/**
 * @brief 标准日志打印接口（调试级别）
 *
 * 该宏用于调试日志打印，包含文件名、行号和函数名。
 *
 * @param fmt 日志格式字符串
 * @param ... 可变参数列表
 */
#define OSA_DEBUG(fmt, ...) \
    printk(KERN_DEBUG OSA_fmt("[DEBUG] %s:%d %s(): " fmt), FILE_BASENAME, __LINE__, __func__, ##__VA_ARGS__)

/**
 * @brief 打印整型变量的名称和值
 *
 * 该宏用于打印整型变量的名称和值。
 *
 * @param var 整型变量
 */
#define OSA_VAR_INT(var) \
    OSA_INFO(#var " = %d\n", var)

/**
 * @brief 打印无符号整型变量的名称和值
 *
 * 该宏用于打印无符号整型变量的名称和值。
 *
 * @param var 无符号整型变量
 */
#define OSA_VAR_UINT(var) \
    OSA_INFO(#var " = %u\n", var)

/**
 * @brief 打印长整型变量的名称和值
 *
 * 该宏用于打印长整型变量的名称和值。
 *
 * @param var 长整型变量
 */
#define OSA_VAR_LONG(var) \
    OSA_INFO(#var " = %ld\n", var)

/**
 * @brief 打印无符号长整型变量的名称和值
 *
 * 该宏用于打印无符号长整型变量的名称和值。
 *
 * @param var 无符号长整型变量
 */
#define OSA_VAR_ULONG(var) \
    OSA_INFO(#var " = %lu\n", var)

/**
 * @brief 打印浮点型变量的名称和值
 *
 * 该宏用于打印浮点型变量的名称和值。
 *
 * @param var 浮点型变量
 */
#define OSA_VAR_FLOAT(var) \
    OSA_INFO(#var " = %f\n", var)

/**
 * @brief 打印双精度浮点型变量的名称和值
 *
 * 该宏用于打印双精度浮点型变量的名称和值。
 *
 * @param var 双精度浮点型变量
 */
#define OSA_VAR_DOUBLE(var) \
    OSA_INFO(#var " = %lf\n", var)

/**
 * @brief 打印字符型变量的名称和值
 *
 * 该宏用于打印字符型变量的名称和值。
 *
 * @param var 字符型变量
 */
#define OSA_VAR_CHAR(var) \
    OSA_INFO(#var " = %c\n", var)

/**
 * @brief 打印字符串变量的名称和值
 *
 * 该宏用于打印字符串变量的名称和值。
 *
 * @param var 字符串变量
 */
#define OSA_VAR_STRING(var) \
    OSA_INFO(#var " = %s\n", var)

/**
 * @brief 打印指针变量的名称和值
 *
 * 该宏用于打印指针变量的名称和值。
 *
 * @param var 指针变量
 */
#define OSA_VAR_PTR(var) \
    OSA_INFO(#var " = %p\n", var)

#endif /* _PDM_OSA_H_ */
