# toolchain.cmake

# 检查并设置 SYSROOT
if(DEFINED CMAKE_SYSROOT)
    # 使用命令行传递的 CMAKE_SYSROOT
elseif(DEFINED ENV{SYSROOT})
    set(CMAKE_SYSROOT $ENV{SYSROOT})
else()
    # 使用系统默认的 sysroot（通常为空）
    set(CMAKE_SYSROOT "")
endif()

# 检查并设置 CROSS_COMPILE
if(DEFINED CROSS_COMPILE)
    # 使用命令行传递的 CROSS_COMPILE
    set(CMAKE_C_COMPILER   ${CROSS_COMPILE}gcc)
    set(CMAKE_CXX_COMPILER ${CROSS_COMPILE}g++)
elseif(DEFINED ENV{CROSS_COMPILE})
    set(CMAKE_C_COMPILER   $ENV{CROSS_COMPILE}gcc)
    set(CMAKE_CXX_COMPILER $ENV{CROSS_COMPILE}g++)
else()
    # 使用系统默认的 gcc 和 g++
    find_program(CMAKE_C_COMPILER gcc)
    find_program(CMAKE_CXX_COMPILER g++)
endif()

# 设置目标系统名称和处理器架构（根据需要调整）
set(CMAKE_SYSTEM_NAME Linux)  # 或者其他目标系统名称
set(CMAKE_SYSTEM_PROCESSOR arm)  # 或者其他目标处理器架构

# 设置查找规则
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# 其他配置...
