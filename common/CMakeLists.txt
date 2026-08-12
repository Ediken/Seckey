# ============================================================
# common 子目录 —— 编译成一个静态库 libcommon.a
# 客户端、服务端都链接它，公共代码只写一份
# ============================================================

# 收集本目录所有 .cpp 源文件，放进变量 COMMON_SRC
# aux_source_directory 是"偷懒写法"：目录里新增 .cpp 不用改这里
aux_source_directory(codec  COMMON_SRC)
aux_source_directory(net   COMMON_SRC)
aux_source_directory(shm   COMMON_SRC)
aux_source_directory(log   COMMON_SRC)
aux_source_directory(crypto COMMON_SRC)

# 生成静态库 libcommon.a
# add_library(名字 STATIC 源文件...)
add_library(common STATIC ${COMMON_SRC})

# 告诉 CMake：这个库的头文件在哪
# 别的目录 #include "xxx.h" 时，能在这里找到
target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
