#ifndef REQUESTCODEC_H
#define REQUESTCODEC_H

#include "Codec.h"          // 继承 Codec（抽象基类）
#include <string.h>         // memcpy 需要

// ============================================================
// RequestMsg —— 客户端发给服务端的【请求报文】结构体
// （第 7 天学的数据结构，现在落地成代码）
// ============================================================
struct RequestMsg
{
    int  cmdType;       // 报文类型：1密钥协商 2密钥校验 3密钥注销 4密钥查看
    char clientId[12];  // 客户端编号（唯一）
    char authCode[65];  // 消息认证码（对 r1 做 HMAC 得到的值）
    char serverId[12];  // 服务端编号
    char r1[64];        // 客户端生成的随机字符串
};

// ============================================================
// RequestCodec —— 请求报文的编解码器（多态的具体实现）
//
// 两个构造函数的用途（对应工厂的两种模式）：
//   RequestCodec()           : 解码用（空对象，msgDecode 往里填数据）
//   RequestCodec(RequestMsg*) : 编码用（把外部数据拷进对象，msgEncode 读它）
// ============================================================
class RequestCodec : public Codec
{
public:
    enum CmdType { NewOrUpdate = 1, Check, Revoke, View };  // 报文类型枚举

    RequestCodec();                  // 解码用
    RequestCodec(RequestMsg* msg);   // 编码用
    ~RequestCodec();

    // 重写父类虚函数（签名必须与 Codec 完全一致！）
    int  msgEncode(char** outData, int& len);
    void* msgDecode(char* inData, int inLen);

private:
    RequestMsg m_msg;   // 内部持有这份报文数据
};

#endif // REQUESTCODEC_H
