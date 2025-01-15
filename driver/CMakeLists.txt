# driver/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

# 定义构建目标
add_custom_target(driver ALL
    COMMAND make -C ${CMAKE_CURRENT_SOURCE_DIR} DESTDIR=${CMAKE_INSTALL_PREFIX}
    COMMENT "Building driver using Makefile"
)

# 定义清理目标
add_custom_target(driver_clean
    COMMAND make clean -C ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Cleaning driver using Makefile"
)

# 定义安装目标
add_custom_target(driver_install
    COMMAND make install DESTDIR=${CMAKE_INSTALL_PREFIX} -C ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Installing driver using Makefile"
    DEPENDS driver
)

# 定义卸载目标（如果 Makefile 支持）
add_custom_target(driver_uninstall
    COMMAND make uninstall DESTDIR=${CMAKE_INSTALL_PREFIX} -C ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Uninstalling driver using Makefile"
)
