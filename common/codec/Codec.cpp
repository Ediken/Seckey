#include "Codec.h"

// 构造函数（空的即可，成员由子类管理）
Codec::Codec()
{
}

// 析构函数（虚析构，见头文件注释）
Codec::~Codec()
{
}

// 占位实现：真正的编码逻辑在子类 RequestCodec/RespondCodec 里
int Codec::msgEncode(char** outData, int& len)
{
    return 0;          // 返回 0 只是"占位"，子类会重写这个函数
}

// 占位实现：真正的解码逻辑在子类里
void* Codec::msgDecode(char* inData, int inLen)
{
    return NULL;       // 占位
}
