# lib/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

# 添加库目标
add_library(pdm_lib STATIC
    src/lib_pdm_dimmer.c
    src/lib_pdm_nvmem.c
    src/lib_pdm_sensor.c
    src/lib_pdm_switch.c
)

# 包含必要的公共头文件路径
target_include_directories(pdm_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/api
)

# 包含必要的私有头文件路径
target_include_directories(pdm_lib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/private
)

# 如果库依赖于其他库，链接它们
# target_link_libraries(pdm_lib ...)
