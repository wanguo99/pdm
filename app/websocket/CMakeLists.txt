# app/websocket/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

# 查找 websockets 库
find_package(websockets REQUIRED)

# 查找 cJSON 库
find_package(cJSON REQUIRED)

# 查找 mosquitto 库
find_package(PkgConfig REQUIRED)
pkg_check_modules(MOSQUITTO REQUIRED libmosquitto)

# 定义 websocket_server 目标
add_executable(websocket_server websocket_server.c)

# 包含必要的头文件路径
target_include_directories(websocket_server PRIVATE
    ${PROJECT_SOURCE_DIR}/driver/include
    ${PROJECT_SOURCE_DIR}/lib/include/api  # 只包含公共 API
    ${WEB SOCKETS_INCLUDE_DIRS}
    ${CJSON_INCLUDE_DIRS}
    ${MOSQUITTO_INCLUDE_DIRS}
)

# 链接到 pdm 共享库和其他依赖库
target_link_libraries(websocket_server PRIVATE
    pdm
    ${WEB_SOCKETS_LIBRARIES}
    ${CJSON_LIBRARIES}
    ${MOSQUITTO_LIBRARIES}
)
