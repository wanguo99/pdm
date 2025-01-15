cmake_minimum_required(VERSION 3.10)
project(PDMProject)

# 设置 C++ 标准（如果适用）
set(CMAKE_C_STANDARD 99)

# 定义全局包含路径，只包含公共头文件路径
include_directories(
    ${PROJECT_SOURCE_DIR}/driver/include
    ${PROJECT_SOURCE_DIR}/lib/include/api  # 只包含公共 API
)

# 设置共享库的安装路径（可选）
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 添加子目录
add_subdirectory(app/test)
add_subdirectory(app/websocket)
add_subdirectory(driver)  # 调用 driver 的 Makefile
add_subdirectory(lib)

# 查找外部依赖库
find_package(websockets REQUIRED)
find_package(cJSON REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(MOSQUITTO REQUIRED libmosquitto)

# 确保库目标在整个项目中可见
if (NOT TARGET pdm)
    find_package(PDM REQUIRED CONFIG)
endif()

# 如果有其他需要添加的子目录，可以继续添加
