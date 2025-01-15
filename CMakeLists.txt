# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(PDM)

# 设置 C 标准
set(CMAKE_C_STANDARD 99)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 添加子目录
add_subdirectory(lib)

add_custom_target(distclean
    COMMAND /bin/sh -c "rm -rf ${CMAKE_BINARY_DIR}/*"
    COMMENT "Removing all files in the build directory using shell command"
)
