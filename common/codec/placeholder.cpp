// 占位源文件：保证 common 静态库至少有一个源文件
// 作用：让 CMake 的 add_library(common STATIC ...) 能正常生成
// 后续 codec 模块写真实代码时，本文件删除即可
void seckey_common_placeholder() {}   // 空函数，防止编译器警告"文件无内容"
