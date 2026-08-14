#ifndef CLIENTOPERATION_H
#define CLIENTOPERATION_H

#include "ClientInfo.h"
#include "TcpSocket.h"
#include "SecKeyShm.h"

// ============================================================
// ClientOperation —— 客户端业务操作类（第 7、8 天）
//
// 核心：密钥协商 secKeyAgree() —— 第 7 天 10 步流程
//  ① 构造 RequestMsg（r1 + authCode）
//  ② 编码 ③ 连接 ④ 发送 ⑤ 接收
//  ⑥ 解码 ⑦ 判断rv ⑧ 生成密钥 hash(r1+r2)
//  ⑨ 写共享内存 ⑩ 断开
//
// 其余三个命令（校验/注销/查看）后面再补，先做协商
// ============================================================
class ClientOperation
{
public:
    ClientOperation(ClientInfo* info);   // 保存配置信息
    ~ClientOperation();

    // 密钥协商（cmdType=1）—— 今天实现
    int secKeyAgree();
    // 密钥校验 / 注销 / 查看 —— 先占位，后续实现
    int secKeyCheck();
    int secKeyRevoke();
    int secKeyView();

private:
    // 生成随机字符串（填入 r1 用）
    void getRandString(int len, char* randBuf);

private:
    ClientInfo m_info;      // 配置信息
    TcpSocket  m_socket;    // 网络通信（第 5 天）
    SecKeyShm* m_shm;       // 共享内存写密钥（第 7 天）
};

#endif // CLIENTOPERATION_H
