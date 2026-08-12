#include "SequenceASN1.h"

// 构造函数：三个指针全部置空（好习惯，防止野指针）
SequenceASN1::SequenceASN1()
{
    m_header = NULL;
    m_next   = NULL;
    m_temp   = NULL;
}

// ---------- 写字段 ----------

// 写头节点（整数）：调 BaseASN1 造一个整数节点，作为链表头
int SequenceASN1::writeHeadNode(int iValue)
{
    DER_ItAsn1_WriteInteger(iValue, &m_header);  // 造节点，m_header 指向它
    m_next = m_header;                            // 游标指向头节点
    return 0;
}

// 写头节点（字符串）：同理，用 EncodeChar 造字符串节点
int SequenceASN1::writeHeadNode(char* sValue, int len)
{
    EncodeChar(sValue, len, &m_header);
    m_next = m_header;
    return 0;
}

// 写后继节点（整数）：挂到当前节点后面，游标后移
int SequenceASN1::writeNextNode(int iValue)
{
    DER_ItAsn1_WriteInteger(iValue, &m_next->next);  // 新节点挂到 m_next 后面
    m_next = m_next->next;                           // 游标后移一位
    return 0;
}

// 写后继节点（字符串）
int SequenceASN1::writeNextNode(char* sValue, int len)
{
    EncodeChar(sValue, len, &m_next->next);
    m_next = m_next->next;
    return 0;
}

// ---------- 读字段 ----------

// 读头节点（整数）：从头节点解出整数，游标指向下一个
int SequenceASN1::readHeadNode(int& iValue)
{
    DER_ItAsn1_ReadInteger(m_header, (ITCAST_UINT32*)&iValue);
    m_next = m_header->next;   // 游标移到第二个节点
    return 0;
}

// 读头节点（字符串）：解出字符串拷给调用者，临时节点用完释放
int SequenceASN1::readHeadNode(char* sValue)
{
    DER_ItAsn1_ReadPrintableString(m_header, &m_temp);  // 解出字符串到临时节点
    memcpy(sValue, m_temp->pData, m_temp->dataLen);     // 拷给调用者的缓冲区
    DER_ITCAST_FreeQueue(m_temp);                       // 临时节点释放
    m_temp = NULL;
    m_next = m_header->next;
    return 0;
}

// 读后继节点（整数）
int SequenceASN1::readNextNode(int& iValue)
{
    DER_ItAsn1_ReadInteger(m_next, (ITCAST_UINT32*)&iValue);
    m_next = m_next->next;
    return 0;
}

// 读后继节点（字符串）
int SequenceASN1::readNextNode(char* sValue)
{
    DER_ItAsn1_ReadPrintableString(m_next, &m_temp);
    memcpy(sValue, m_temp->pData, m_temp->dataLen);
    DER_ITCAST_FreeQueue(m_temp);
    m_temp = NULL;
    m_next = m_next->next;
    return 0;
}

// ---------- 打包 / 拆包 ----------

// 打包：链表 -> DER 字节流（编码的最后一步）
int SequenceASN1::packSequence(char** outData, int& outLen)
{
    DER_ItAsn1_WriteSequence(m_header, &m_temp);  // 1. 整条链表序列化成字节流
    *outData = (char*)m_temp->pData;              // 2. 字节流地址交给调用者
    outLen   = m_temp->dataLen;                   //    长度交给调用者
    DER_ITCAST_FreeQueue(m_header);               // 3. 链表用完释放（防泄漏）
    m_header = NULL;
    m_next   = NULL;
    return 0;
}

// 拆包：字节流 -> 链表（解码的第一步）
int SequenceASN1::unpackSequence(char* inData, int inLen)
{
    DER_ITCAST_String_To_AnyBuf(&m_temp, (unsigned char*)inData, inLen);  // 1. char* 转 ANYBUF
    DER_ItAsn1_ReadSequence(m_temp, &m_header);                           // 2. 拆出链表
    DER_ITCAST_FreeQueue(m_temp);                                         // 3. 中转节点释放
    m_temp = NULL;
    m_next = m_header;   // 游标指向头节点，等待 readHeadNode 读取
    return 0;
}

// 释放整条链表
void SequenceASN1::freeSequence()
{
    if (m_header != NULL)
    {
        DER_ITCAST_FreeQueue(m_header);
        m_header = NULL;
        m_next   = NULL;
    }
}
