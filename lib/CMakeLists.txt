# lib/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

# 添加共享库目标，并指定库名为 libpdm.so
add_library(pdm SHARED
    src/lib_pdm_dimmer.c
    src/lib_pdm_nvmem.c
    src/lib_pdm_sensor.c
    src/lib_pdm_switch.c
)

# 设置输出库文件名为 libpdm.so
set_target_properties(pdm PROPERTIES
    OUTPUT_NAME "pdm"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# 包含必要的公共头文件路径
target_include_directories(pdm PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/api
)

# 包含必要的私有头文件路径
target_include_directories(pdm PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/private
)

# 导出库目标，使其在整个项目中可见
install(TARGETS pdm EXPORT pdmTargets
    LIBRARY DESTINATION lib
    INCLUDES DESTINATION include
)

# 创建配置文件
install(EXPORT pdmTargets
    FILE pdmConfig.cmake
    NAMESPACE PDM::
    DESTINATION lib/cmake/pdm
)
