# driver/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

# 调用 Makefile 进行构建
add_custom_target(driver ALL
    COMMAND make -C ${CMAKE_CURRENT_SOURCE_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building driver using Makefile"
)
