# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(PDM)

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

# 设置 C 标准
set(CMAKE_C_STANDARD 99)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 设置安装时的 RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# 禁止安装过程中清除 RPATH
set(CMAKE_SKIP_INSTALL_RPATH FALSE)
set(CMAKE_SKIP_RPATH FALSE)

# 添加子目录
add_subdirectory(lib)
add_subdirectory(app/pdm_test)
add_subdirectory(app/websocket_server)

# 自定义清理目标
add_custom_target(distclean
    COMMAND /bin/sh -c "rm -rf ${CMAKE_BINARY_DIR}/*"
    COMMENT "Removing all files in the build directory using shell command"
)
