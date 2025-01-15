# toolchain.cmake

# 检查并设置 CROSS_COMPILE
if(DEFINED CMAKE_CROSSCOMPILING_EMULATOR)
    # 如果使用模拟器进行交叉编译，这里可以处理特殊情况
endif()

# 优先从命令行参数获取 SYSROOT 和 CROSS_COMPILE
if(DEFINED CMAKE_SYSROOT)
    set(SYSROOT ${CMAKE_SYSROOT})
elseif(DEFINED ENV{SYSROOT})
    set(SYSROOT $ENV{SYSROOT})
else()
    message(WARNING "Neither CMAKE_SYSROOT nor environment variable SYSROOT is set.")
endif()

if(DEFINED CROSS_COMPILE)
    set(CROSS_COMPILE ${CROSS_COMPILE})
elseif(DEFINED ENV{CROSS_COMPILE})
    set(CROSS_COMPILE $ENV{CROSS_COMPILE})
else()
    message(WARNING "Neither CROSS_COMPILE nor environment variable CROSS_COMPILE is set.")
endif()

# 设置查找规则为仅在 CMAKE_FIND_ROOT_PATH 中查找
if(DEFINED SYSROOT)
    set(CMAKE_FIND_ROOT_PATH ${SYSROOT})
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

    # 设置 PKG_CONFIG_PATH 以确保 pkg-config 查找正确的路径
    set(ENV{PKG_CONFIG_PATH} "${SYSROOT}/usr/lib/pkgconfig:${SYSROOT}/usr/share/pkgconfig")
endif()

# 设置目标系统名称和处理器架构（根据需要调整）
set(CMAKE_SYSTEM_NAME Linux)  # 或者其他目标系统名称
set(CMAKE_SYSTEM_PROCESSOR arm)  # 或者其他目标处理器架构

# 设置交叉编译工具链
if(DEFINED CROSS_COMPILE)
    set(CMAKE_C_COMPILER   ${CROSS_COMPILE}gcc)
    set(CMAKE_CXX_COMPILER ${CROSS_COMPILE}g++)
    set(CMAKE_AR           ${CROSS_COMPILE}ar CACHE FILEPATH "Archiver")
    set(CMAKE_RANLIB       ${CROSS_COMPILE}ranlib CACHE FILEPATH "Ranlib")
endif()

# 其他配置...
