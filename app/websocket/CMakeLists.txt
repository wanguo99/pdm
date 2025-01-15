# app/websocket/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

# 定义 websocket_server 目标
add_executable(websocket_server websocket_server.c)

# 包含必要的头文件路径
target_include_directories(websocket_server PRIVATE
    ${PROJECT_SOURCE_DIR}/driver/include
    ${PROJECT_SOURCE_DIR}/lib/include/api  # 只包含公共 API
)

# 链接必要的库（如果有）
target_link_libraries(websocket_server pdm_lib)
