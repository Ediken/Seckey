#include "RespondCodec.h"
#include <string.h>     // memset、memcpy

// 解码用构造函数：空对象
RespondCodec::RespondCodec() : Codec()
{
    memset(&m_msg, 0, sizeof(RespondMsg));   // 清空
}

// 编码用构造函数：把外部数据整体拷进对象
RespondCodec::RespondCodec(RespondMsg* msg) : Codec()
{
    memcpy(&m_msg, msg, sizeof(RespondMsg));
}

RespondCodec::~RespondCodec()
{
}

// ------------------------------------------------------------
// 编码：RespondMsg(结构体) -> 字节流
// 字段顺序：rv → clientId → serverId → r2 → seckeyid
// ============================================================
int RespondCodec::msgEncode(char** outData, int& len)
{
    // 1. 依次把 5 个字段写进链表
    writeHeadNode(m_msg.rv);                                  // 第1字段：整数
    writeNextNode(m_msg.clientId, strlen(m_msg.clientId) + 1); // 第2字段：字符串
    writeNextNode(m_msg.serverId, strlen(m_msg.serverId) + 1); // 第3字段
    writeNextNode(m_msg.r2,       strlen(m_msg.r2) + 1);       // 第4字段
    writeNextNode(m_msg.seckeyid);                            // 第5字段：整数

    // 2. 打包成字节流
    packSequence(outData, len);

    return 0;
}

// ------------------------------------------------------------
// 解码：字节流 -> RespondMsg(结构体)
// 返回：指向内部 m_msg 的指针
// ============================================================
void* RespondCodec::msgDecode(char* inData, int inLen)
{
    // 1. 字节流拆回链表
    unpackSequence(inData, inLen);

    // 2. 按编码时的顺序读回 5 个字段
    readHeadNode(m_msg.rv);          // 第1字段：整数
    readNextNode(m_msg.clientId);    // 第2字段：字符串
    readNextNode(m_msg.serverId);    // 第3字段
    readNextNode(m_msg.r2);          // 第4字段
    readNextNode(m_msg.seckeyid);    // 第5字段：整数

    // 3. 返回填好的结构体
    return &m_msg;
}
