# ============================================================
# client 子目录 —— 密钥协商客户端可执行程序
# ============================================================

# 收集本目录所有 .cpp 源文件
aux_source_directory(. CLIENT_SRC)

# 生成可执行文件 seckey_client
add_executable(seckey_client ${CLIENT_SRC})

# 链接 common 静态库
target_link_libraries(seckey_client common)

# 链接 openssl 的 crypto 库（HMAC/哈希函数需要）
#   头文件在系统默认路径 /usr/include/openssl，不用额外配 include
target_link_libraries(seckey_client crypto)

# 头文件搜索路径：本目录（能找到 ClientInfo.h/ClientOperation.h）
target_include_directories(seckey_client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
