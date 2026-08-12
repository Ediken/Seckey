#include "RequestCodec.h"
#include <string.h>     // strlen、memcpy

// 解码用构造函数：空对象（数据等 msgDecode 往里填）
RequestCodec::RequestCodec() : Codec()
{
    memset(&m_msg, 0, sizeof(RequestMsg));   // 清空，防止垃圾数据
}

// 编码用构造函数：把外部数据拷进对象内部
RequestCodec::RequestCodec(RequestMsg* msg) : Codec()
{
    memcpy(&m_msg, msg, sizeof(RequestMsg)); // 结构体整体拷贝
}

RequestCodec::~RequestCodec()
{
}

// ------------------------------------------------------------
// 编码：RequestMsg(结构体) -> 字节流
// 关键：字段顺序必须固定，解码端按同样的顺序读
// ============================================================
int RequestCodec::msgEncode(char** outData, int& len)
{
    // 1. 依次把 5 个字段写进链表（顺序即协议：cmdType→clientId→authCode→serverId→r1）
    writeHeadNode(m_msg.cmdType);                             // 第1字段：整数
    writeNextNode(m_msg.clientId, strlen(m_msg.clientId) + 1); // 第2字段：字符串(含\0)
    writeNextNode(m_msg.authCode, strlen(m_msg.authCode) + 1); // 第3字段
    writeNextNode(m_msg.serverId, strlen(m_msg.serverId) + 1); // 第4字段
    writeNextNode(m_msg.r1,       strlen(m_msg.r1) + 1);       // 第5字段

    // 2. 打包成一段连续字节流（outData/len 由函数填充）
    packSequence(outData, len);

    return 0;
}

// ------------------------------------------------------------
// 解码：字节流 -> RequestMsg(结构体)
// 返回：指向内部 m_msg 的指针（调用者转成 RequestMsg* 使用）
// ============================================================
void* RequestCodec::msgDecode(char* inData, int inLen)
{
    // 1. 先把字节流拆回链表
    unpackSequence(inData, inLen);

    // 2. 按编码时的顺序，依次读出 5 个字段
    readHeadNode(m_msg.cmdType);        // 第1字段：整数
    readNextNode(m_msg.clientId);       // 第2字段：字符串
    readNextNode(m_msg.authCode);       // 第3字段
    readNextNode(m_msg.serverId);       // 第4字段
    readNextNode(m_msg.r1);             // 第5字段

    // 3. 返回填好的结构体
    return &m_msg;
}
