# app/test/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

# 定义测试目标
add_executable(pdm_test
    pdm_dimmer_test.c
    pdm_nvmem_test.c
    pdm_sensor_test.c
    pdm_switch_test.c
    pdm_test.c
)

# 包含必要的头文件路径
target_include_directories(pdm_test PRIVATE
    ${PROJECT_SOURCE_DIR}/driver/include/uapi
    ${PROJECT_SOURCE_DIR}/lib/include/api  # 只包含公共 API
)

# 链接到 pdm 共享库
target_link_libraries(pdm_test PRIVATE pdm)
