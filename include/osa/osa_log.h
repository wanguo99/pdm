#ifndef _OSA_LOG_H_
#define _OSA_LOG_H_

/**
 * @file osa_log.h
 * @brief Common logging interface for kernel modules.
 *
 * This file defines a set of macros for logging that can be used in kernel modules,
 * including module name, filename, line number, and function name in log messages.
 */

#include <linux/kernel.h>
#include <linux/string.h>

/*
 * Extract the basename from a file path.
 */
#define BASENAME(file) (strrchr(file, '/') ? strrchr(file, '/') + 1 : file)

/*
 * Get the base name of the current file (excluding path).
 */
#define FILE_BASENAME (BASENAME(__FILE__))

/*
 * Define whether to include file and line information in logs.
 */
#ifndef DEBUG_OSA_LOG_WITH_FILE_LINE
#define DEBUG_OSA_LOG_WITH_FILE_LINE 0
#endif

/*
 * Define whether to include function name in logs.
 */
#ifndef DEBUG_OSA_LOG_WITH_FUNCTION
#define DEBUG_OSA_LOG_WITH_FUNCTION 0
#endif

/*
 * Define whether to enable logging.
 */
#ifndef DEBUG_OSA_LOG_ENABLE
#define DEBUG_OSA_LOG_ENABLE 1
#endif

/*
 * Default format and arguments definition.
 */
#define OSA_LOG_FMT "%s"
#define OSA_LOG_ARGS "- "

/*
 * Set default log format and args based on compile-time options.
 */
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

/*
 * Enable or disable logging based on DEBUG_OSA_LOG_ENABLE.
 */
#if DEBUG_OSA_LOG_ENABLE
#define OSA_LOG_ENABLED 1

/*
 * Log printing macro.
 */
#define OSA_PRINTK(level, level_str, fmt, ...) \
    printk(level "%s" OSA_LOG_FMT fmt, level_str, OSA_LOG_ARGS, ##__VA_ARGS__)

#else
#define OSA_LOG_ENABLED 0

/*
 * Disable log printing macro.
 */
#define OSA_PRINTK(level, fmt, ...) /* 日志禁用时不输出任何内容 */
#endif

/*
 * Simple print without code information.
 */
#define OSA_print(fmt, ...) \
    OSA_PRINTK(KERN_ERR, "", fmt, ##__VA_ARGS__)

/*
 * Standard logging macros with different severity levels.
 */
#define OSA_EMERG(fmt, ...) \
    OSA_PRINTK(KERN_EMERG, "[EMERG] ", fmt, ##__VA_ARGS__)
#define OSA_ALERT(fmt, ...) \
    OSA_PRINTK(KERN_ALERT, "[ALERT] ", fmt, ##__VA_ARGS__)
#define OSA_CRIT(fmt, ...) \
    OSA_PRINTK(KERN_CRIT, "[CRIT] ", fmt, ##__VA_ARGS__)
#define OSA_ERROR(fmt, ...) \
    OSA_PRINTK(KERN_ERR, "[ERROR] ", fmt, ##__VA_ARGS__)
#define OSA_WARN(fmt, ...) \
    OSA_PRINTK(KERN_WARNING, "[WARNING] ", fmt, ##__VA_ARGS__)
#define OSA_NOTICE(fmt, ...) \
    OSA_PRINTK(KERN_NOTICE, "[NOTICE] ", fmt, ##__VA_ARGS__)
#define OSA_INFO(fmt, ...) \
    OSA_PRINTK(KERN_INFO, "[INFO] ", fmt, ##__VA_ARGS__)
#define OSA_DEBUG(fmt, ...) \
    OSA_PRINTK(KERN_DEBUG, "[DEBUG] ", fmt, ##__VA_ARGS__)

/*
 * Macros to print variable names and values.
 */
#define OSA_VAR_INT(var) \
    OSA_INFO(#var " = %d\n", var)
#define OSA_VAR_UINT(var) \
    OSA_INFO(#var " = %u\n", var)
#define OSA_VAR_LONG(var) \
    OSA_INFO(#var " = %ld\n", var)
#define OSA_VAR_ULONG(var) \
    OSA_INFO(#var " = %lu\n", var)
#define OSA_VAR_FLOAT(var) \
    OSA_INFO(#var " = %f\n", var)
#define OSA_VAR_DOUBLE(var) \
    OSA_INFO(#var " = %lf\n", var)
#define OSA_VAR_CHAR(var) \
    OSA_INFO(#var " = %c\n", var)
#define OSA_VAR_STRING(var) \
    OSA_INFO(#var " = %s\n", var)
#define OSA_VAR_PTR(var) \
    OSA_INFO(#var " = %p\n", var)

#endif /* _OSA_LOG_H_ */
