#ifndef SERVEROPERATION_H
#define SERVEROPERATION_H

#include "TcpServer.h"
#include "TcpSocket.h"
#include "SecKeyShm.h"
#include "RequestCodec.h"

#include <pthread.h>

// ============================================================
// ServerOperation —— 服务端业务类（第 8 天）
//
// 结构：
//   main: startWork() 主循环
//     ├─ TcpServer 监听，accept 每个客户端
//     └─ 每来一个连接，开一个子线程 working() 处理
//           ├─ 收数据 → 解码 → 按 cmdType 分发
//           └─ secKeyAgree()：校验 → r2 → 算密钥 → 写共享内存 → 应答
// ============================================================
class ServerOperation
{
public:
    ServerOperation(int port, int shmKey, int maxNode);
    ~ServerOperation();

    // 服务器开始工作（主循环：accept + 开线程）
    void startWork();

    // 密钥协商处理（子线程里调用）
    int secKeyAgree(RequestMsg* reqmsg, char** outData, int& outLen);

private:
    // 生成随机字符串（r2）
    void getRandString(int len, char* randBuf);

private:
    int      m_port;      // 监听端口
    int      m_shmKey;    // 共享内存 key
    int      m_maxNode;   // 共享内存最大节点数
    TcpServer m_server;   // 监听器
    SecKeyShm* m_shm;     // 共享内存（写密钥）
};

// 线程回调函数：必须是全局函数或静态成员（第 8 天）
// 处理一个客户端连接
void* working(void* arg);

#endif // SERVEROPERATION_H
