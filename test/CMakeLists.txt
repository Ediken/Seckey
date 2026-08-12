# 生成测试可执行文件 seckey_test
add_executable(seckey_test main.cpp)

# 链接 common 静态库（现在还没源码，先空库链接验证流程）
target_link_libraries(seckey_test common)
