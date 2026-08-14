# ============================================================
# 顶层 CMakeLists.txt —— 整个工程的构建入口
# CMake 从这里开始，一层层把子目录的规则拼装成完整工程
# ============================================================

# 1. 最低 CMake 版本要求
cmake_minimum_required(VERSION 3.10)

# 2. 工程名 + 语言（本项目只用 C++）
project(SeckeyProject LANGUAGES CXX)

# 3. 统一使用 C++11 标准（智能指针等特性需要）
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 4. 所有可执行文件统一输出到 build/bin/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 5. 依次加入子目录（顺序：先 common，因为它被 client/server 依赖）
add_subdirectory(common)
add_subdirectory(client)
add_subdirectory(test)
# add_subdirectory(server)   ← server 后面再放开
