#ifndef SERVEROPERATION_H
#define SERVEROPERATION_H

#include "TcpServer.h"
#include "TcpSocket.h"
#include "SecKeyShm.h"
#include "RequestCodec.h"

#include <pthread.h>
class ServerOperation
{
public:
    ServerOperation(int port, int shmKey, int maxNode);
    ~ServerOperation();

    // 服务器开始工作（主循环：accept + 开线程）
    void startWork();

    // 密钥协商处理（子线程 working 里调用，所以是 public）
    // 输入：客户端请求 reqmsg
    // 输出：应答字节流 outData/outLen
    int secKeyAgree(RequestMsg* reqmsg, char** outData, int& outLen);

private:
    // 生成随机字符串（r2）
    void getRandString(int len, char* randBuf);

private:
    int       m_port;     // 监听端口
    int       m_shmKey;   // 共享内存 key
    int       m_maxNode;  // 共享内存最大节点数
    TcpServer m_server;   // 监听器
    SecKeyShm* m_shm;     // 共享内存（写密钥）
};

// ============================================================
struct ThreadArg
{
    TcpSocket* sock;          // 连接通信对象
    ServerOperation* server;  // 服务端业务对象
};

// 处理一个客户端连接
void* working(void* arg);

#endif // SERVEROPERATION_H
