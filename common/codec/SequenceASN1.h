#ifndef SEQUENCEASN1_H
#define SEQUENCEASN1_H

#include "BaseASN1.h"      // 用 BaseASN1 的 DER 工具造节点
#include <string.h>        // memcpy 需要

class SequenceASN1 : public BaseASN1
{
public:
    SequenceASN1();

    // ---------- 写入字段（编码方向） ----------

    // 写【头】节点（报文第一个字段）：整数版 / 字符串版
    int writeHeadNode(int iValue);
    int writeHeadNode(char* sValue, int len);
    // 写【后继】节点（追加到链表尾部）：整数版 / 字符串版
    int writeNextNode(int iValue);
    int writeNextNode(char* sValue, int len);

    // ---------- 读取字段（解码方向） ----------

    // 读【头】节点：整数版 / 字符串版
    int readHeadNode(int& iValue);
    int readHeadNode(char* sValue);
    // 读【后继】节点：整数版 / 字符串版
    int readNextNode(int& iValue);
    int readNextNode(char* sValue);

    // ---------- 打包 / 拆包 ----------

    // 打包：把整条链表编码成一段字节流（编码的最后一步）
    int packSequence(char** outData, int& outLen);
    // 拆包：把字节流解析回链表（解码的第一步）
    int unpackSequence(char* inData, int inLen);

    // 释放整条链表（防止内存泄漏）
    void freeSequence();

private:
    ITCAST_ANYBUF* m_header;   // 链表头节点
    ITCAST_ANYBUF* m_next;     // 游标：当前操作到的节点
    ITCAST_ANYBUF* m_temp;     // 临时节点（中转用）
};

#endif // SEQUENCEASN1_H
