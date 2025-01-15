# toolchain.cmake

# 设置目标系统名称和处理器架构（根据需要调整）
set(CMAKE_SYSTEM_NAME Linux)  # 或者其他目标系统名称
set(CMAKE_SYSTEM_PROCESSOR arm)  # 或者其他目标处理器架构

# 检查并设置 SYSROOT
if(DEFINED CMAKE_SYSROOT AND NOT CMAKE_SYSROOT STREQUAL "")
    set(SYSROOT ${CMAKE_SYSROOT})
elseif(DEFINED ENV{SYSROOT} AND NOT "$ENV{SYSROOT}" STREQUAL "")
    set(SYSROOT $ENV{SYSROOT})
else()
    message(FATAL_ERROR "Neither CMAKE_SYSROOT nor environment variable SYSROOT is set.")
endif()

# 检查并设置 CROSS_COMPILE
if(DEFINED CROSS_COMPILE AND NOT CROSS_COMPILE STREQUAL "")
    set(CROSS_COMPILE ${CROSS_COMPILE})
elseif(DEFINED ENV{CROSS_COMPILE} AND NOT "$ENV{CROSS_COMPILE}" STREQUAL "")
    set(CROSS_COMPILE $ENV{CROSS_COMPILE})
else()
    message(FATAL_ERROR "Neither CROSS_COMPILE nor environment variable CROSS_COMPILE is set.")
endif()

# 设置交叉编译工具链
set(CMAKE_C_COMPILER   ${CROSS_COMPILE}gcc)
set(CMAKE_CXX_COMPILER ${CROSS_COMPILE}g++)
set(CMAKE_AR           ${CROSS_COMPILE}ar CACHE FILEPATH "Archiver")
set(CMAKE_RANLIB       ${CROSS_COMPILE}ranlib CACHE FILEPATH "Ranlib")
