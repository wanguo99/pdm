cmake_minimum_required(VERSION 3.10)
project(PDMProject)

# 设置 C++ 标准（如果适用）
set(CMAKE_C_STANDARD 99)

# 定义全局包含路径，只包含公共头文件路径
include_directories(
    ${PROJECT_SOURCE_DIR}/driver/include
    ${PROJECT_SOURCE_DIR}/lib/include/api  # 只包含公共 API
)

# 添加子目录
add_subdirectory(app/test)
add_subdirectory(app/websocket)
add_subdirectory(driver)  # 调用 driver 的 Makefile
add_subdirectory(lib)

# 如果有其他需要添加的子目录，可以继续添加
