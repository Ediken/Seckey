# ============================================================
# server 子目录 —— 密钥协商服务端可执行程序
# ============================================================

aux_source_directory(. SERVER_SRC)

# 生成可执行文件 seckey_server
add_executable(seckey_server ${SERVER_SRC})

# 链接 common 静态库 + openssl crypto 库
target_link_libraries(seckey_server common)
target_link_libraries(seckey_server mysqlclient)
target_include_directories(seckey_server PRIVATE /usr/include/mysql)
target_link_libraries(seckey_server crypto)

# 头文件搜索路径：本目录
target_include_directories(seckey_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
