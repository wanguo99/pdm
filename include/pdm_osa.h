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
 * @brief 定义是否在日志中包含文件名和行号
 *
 * 通过定义或取消定义此宏来控制是否在日志中包含文件名和行号。
 */
#ifndef DEBUG_OSA_LOG_WITH_FILE_LINE
#define DEBUG_OSA_LOG_WITH_FILE_LINE 0
#endif

/**
 * @brief 定义是否在日志中包含函数名
 *
 * 通过定义或取消定义此宏来控制是否在日志中包含函数名。
 */
#ifndef DEBUG_OSA_LOG_WITH_FUNCTION
#define DEBUG_OSA_LOG_WITH_FUNCTION 0
#endif

/**
 * @brief 定义是否启用日志打印
 *
 * 通过定义或取消定义此宏来控制是否启用日志打印。
 */
#ifndef DEBUG_OSA_LOG_ENABLE
#define DEBUG_OSA_LOG_ENABLE 1
#endif


// 默认格式和参数定义
#define OSA_LOG_FMT "%s"
#define OSA_LOG_ARGS "- "

// 根据组合条件设置格式和参数
#if DEBUG_OSA_LOG_WITH_FILE_LINE && DEBUG_OSA_LOG_WITH_FUNCTION
    #undef OSA_LOG_FMT
    #define OSA_LOG_FMT "(%s:%d)->%s(): "
    #undef OSA_LOG_ARGS
    #define OSA_LOG_ARGS FILE_BASENAME, __LINE__, __func__
#elif DEBUG_OSA_LOG_WITH_FILE_LINE
    #undef OSA_LOG_FMT
    #define OSA_LOG_FMT "(%s:%d) "
    #undef OSA_LOG_ARGS
    #define OSA_LOG_ARGS FILE_BASENAME, __LINE__
#elif DEBUG_OSA_LOG_WITH_FUNCTION
    #undef OSA_LOG_FMT
    #define OSA_LOG_FMT "%s(): "
    #undef OSA_LOG_ARGS
    #define OSA_LOG_ARGS __func__
#endif

#if DEBUG_OSA_LOG_ENABLE
#define OSA_LOG_ENABLED 1
#define OSA_PRINTK(level, level_str, fmt, ...) \
    printk(level "%s" OSA_LOG_FMT fmt, level_str, OSA_LOG_ARGS, ##__VA_ARGS__)
#else
#define OSA_LOG_ENABLED 0
#define OSA_PRINTK(level, fmt, ...) /* 日志禁用时不输出任何内容 */
#endif



/**
 * @brief 不带代码信息的打印
 *
 * 该宏用于简单的错误日志打印，不包含文件名、行号和函数名。
 *
 * @param fmt 日志格式字符串
 * @param ... 可变参数列表
 */
#define OSA_print(fmt, ...) \
    OSA_PRINTK(KERN_ERR, fmt, ##__VA_ARGS__)

/**
 * @brief 标准日志打印接口（错误级别）
 *
 * 该宏用于错误日志打印，包含文件名、行号和函数名。
 *
 * @param fmt 日志格式字符串
 * @param ... 可变参数列表
 */
#define OSA_ERROR(fmt, ...) \
    OSA_PRINTK(KERN_ERR, "[ERROR] ", fmt, ##__VA_ARGS__)

/**
 * @brief 标准日志打印接口（警告级别）
 *
 * 该宏用于警告日志打印，包含文件名、行号和函数名。
 *
 * @param fmt 日志格式字符串
 * @param ... 可变参数列表
 */
#define OSA_WARN(fmt, ...) \
    OSA_PRINTK(KERN_WARNING, "[WARNING] ", fmt, ##__VA_ARGS__)

/**
 * @brief 标准日志打印接口（信息级别）
 *
 * 该宏用于信息日志打印，包含文件名、行号和函数名。
 *
 * @param fmt 日志格式字符串
 * @param ... 可变参数列表
 */
#define OSA_INFO(fmt, ...) \
    OSA_PRINTK(KERN_INFO, "[INFO] ", fmt, ##__VA_ARGS__)

/**
 * @brief 标准日志打印接口（调试级别）
 *
 * 该宏用于调试日志打印，包含文件名、行号和函数名。
 *
 * @param fmt 日志格式字符串
 * @param ... 可变参数列表
 */
#define OSA_DEBUG(fmt, ...) \
    OSA_PRINTK(KERN_DEBUG, "[DEBUG] ", fmt, ##__VA_ARGS__)

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
