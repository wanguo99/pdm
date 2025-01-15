# app/test/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

# 定义测试程序目标
add_executable(pdm_test
    src/pdm_test.c
    src/pdm_dimmer_test.c
    src/pdm_nvmem_test.c
    src/pdm_sensor_test.c
    src/pdm_switch_test.c
)

# 包含必要的公共头文件路径
target_include_directories(pdm_test PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/driver/include/uapi
    ${PROJECT_SOURCE_DIR}/lib/include/api  # 只包含公共 API
)

# 链接到 libpdm 库
target_link_libraries(pdm_test PRIVATE libpdm)

# 设置 pdm_test 的 RPATH
set_target_properties(pdm_test PROPERTIES
    INSTALL_RPATH "$ORIGIN/../lib"  # 使用相对路径
)

# 安装规则
install(TARGETS pdm_test
    RUNTIME DESTINATION bin
)
