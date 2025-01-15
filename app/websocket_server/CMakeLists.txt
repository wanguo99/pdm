# app/test/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

# 定义测试程序目标
add_executable(websocket_server websocket_server.c)

# 包含必要的公共头文件路径
target_include_directories(websocket_server PRIVATE
    ${PROJECT_SOURCE_DIR}/lib/include/api  # 只包含公共 API
)

# 链接到 libpdm 库
target_link_libraries(websocket_server PRIVATE libpdm
	cjson
	websockets
	mosquitto)

# 设置 pdm_test 的 RPATH
set_target_properties(websocket_server PROPERTIES
    INSTALL_RPATH "$ORIGIN/../lib"  # 使用相对路径
)

# 安装规则
install(TARGETS websocket_server
    RUNTIME DESTINATION bin
)
