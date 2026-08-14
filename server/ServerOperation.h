#ifndef SERVEROPERATION_H
#define SERVEROPERATION_H

#include "TcpServer.h"
#include "TcpSocket.h"
#include "SecKeyShm.h"
#include "RequestCodec.h"
#include "MysqlOP.h"    
#include <pthread.h>
class ServerOperation
{
public:

    // 服务器开始工作（主循环：accept + 开线程）
    void startWork();
    void createDaemon();
    static void catchSignal(int num);
    // 密钥协商处理（子线程 working 里调用，所以是 public）
    // 输入：客户端请求 reqmsg
    // 输出：应答字节流 outData/outLen
    int secKeyAgree(RequestMsg* reqmsg, char** outData, int& outLen);

        // 密钥校验 / 注销 / 查看
    int secKeyCheck(RequestMsg* reqmsg, char** outData, int& outLen);
    int secKeyRevoke(RequestMsg* reqmsg, char** outData, int& outLen);
    int secKeyView(RequestMsg* reqmsg, char** outData, int& outLen);

    ServerOperation(int port, int shmKey, int maxNode,
                    const char* dbHost, const char* dbUser,
                    const char* dbPasswd, const char* dbName);   // 新增重载
    ~ServerOperation();
private:
    // 生成随机字符串（r2）
    void getRandString(int len, char* randBuf);

private:
    int       m_port;     // 监听端口
    int       m_shmKey;   // 共享内存 key
    int       m_maxNode;  // 共享内存最大节点数
    TcpServer m_server;   // 监听器
    SecKeyShm* m_shm;     // 共享内存（写密钥）
    MysqlOP m_db;        //db
    bool m_stop;    // 停止标志
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
