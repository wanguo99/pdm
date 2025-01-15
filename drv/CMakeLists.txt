# 修改后的 driver/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

# 定义构建目标
add_custom_target(pdm ALL
    COMMAND make -C ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building pdm"
)

# 为 'clean' 操作添加额外的清理步骤
add_custom_target(pdm_clean
    COMMAND make clean -C ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Cleaning pdm"
)

# 在顶层 CMakeLists.txt 中，你可以建议用户运行 'make clean_driver' 作为清理的一部分
# 或者你可以覆盖默认的 'clean' 行为，但这通常不推荐

# 定义安装目标，并确保其在 'install' 时被调用
add_custom_target(pdm_install
    COMMAND make install DESTDIR=${CMAKE_INSTALL_PREFIX} -C ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Installing pdm"
    DEPENDS pdm
)
if(CMAKE_INSTALL_TARGETS)
    list(APPEND CMAKE_INSTALL_TARGETS pdm_install)
else()
    set(CMAKE_INSTALL_TARGETS pdm_install)
endif()

# 定义卸载目标，并确保其在 'uninstall' 时被调用（如果 Makefile 支持）
add_custom_target(pdm_uninstall
    COMMAND make uninstall DESTDIR=${CMAKE_INSTALL_PREFIX} -C ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Uninstalling pdm"
)

# 创建一个伪目标来模拟 'uninstall' 并添加对 'pdm_uninstall' 的依赖
if(NOT TARGET uninstall)
    add_custom_target(uninstall)
endif()
add_dependencies(uninstall pdm_uninstall) # 确保当调用 uninstall 时也会调用 pdm_uninstall

# 提供一个更友好的方式让用户知道他们应该同时运行 'make clean' 和 'make pdm_clean'
message(STATUS "To perform a full clean, run: make clean && make pdm_clean")
