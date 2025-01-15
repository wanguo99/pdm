# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(PDMProject)

# 设置 C 标准
set(CMAKE_C_STANDARD 99)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 设置默认安装路径为 _install，并支持 DESTDIR
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/_install" CACHE PATH "Default install path" FORCE)
endif()

# 添加子目录
add_subdirectory(app/test)
# add_subdirectory(app/websocket)
add_subdirectory(driver)  # 调用 driver 的 Makefile
add_subdirectory(lib)

# 确保库目标在整个项目中可见
if (NOT TARGET pdm)
    find_package(PDM REQUIRED CONFIG)
endif()
