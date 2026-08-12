#ifndef RESPONDCODEC_H
#define RESPONDCODEC_H

#include "Codec.h"          // 继承 Codec
#include <string.h>         // strcpy 需要

// ============================================================
// RespondMsg —— 服务端发给客户端的【应答报文】结构体
// （第 7 天学的数据结构，现在落地成代码）
// ============================================================
struct RespondMsg
{
    int  rv;            // 返回值：0=成功  -1=失败
    char clientId[12];  // 客户端编号
    char serverId[12];  // 服务端编号
    char r2[64];        // 服务端随机字符串（密钥协商时用）
    int  seckeyid;      // 密钥编号
};

// ============================================================
// RespondCodec —— 应答报文的编解码器（多态的具体实现）
// 与 RequestCodec 结构完全对称，只是字段不同
// ============================================================
class RespondCodec : public Codec
{
public:
    RespondCodec();                  // 解码用
    RespondCodec(RespondMsg* msg);   // 编码用
    ~RespondCodec();

    // 重写父类虚函数（签名必须与 Codec 完全一致）
    int  msgEncode(char** outData, int& len);
    void* msgDecode(char* inData, int inLen);

private:
    RespondMsg m_msg;   // 内部持有这份报文数据
};

#endif // RESPONDCODEC_H
