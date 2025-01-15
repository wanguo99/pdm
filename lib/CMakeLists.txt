# lib/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

# 添加共享库目标，并指定库名为 pdm
add_library(pdm SHARED
    src/lib_pdm_dimmer.c
    src/lib_pdm_nvmem.c
    src/lib_pdm_sensor.c
    src/lib_pdm_switch.c
)

# 设置输出库文件名为 pdm 并指定输出目录
set_target_properties(pdm PROPERTIES
    OUTPUT_NAME "pdm"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# 包含必要的公共头文件路径（使用相对路径）
target_include_directories(pdm PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/api>
    $<INSTALL_INTERFACE:include>
)

# 包含必要的私有头文件路径（使用相对路径）
target_include_directories(pdm PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/private
)

# 安装规则
install(TARGETS pdm
    LIBRARY DESTINATION lib
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/api/
    DESTINATION include
)
