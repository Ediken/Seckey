#include "ServerOperation.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <openssl/hmac.h>
#include <openssl/sha.h>

#include "RespondCodec.h"
#include "RespondFactory.h"
#include "RequestFactory.h"

// 构造函数：保存配置，创建共享内存（服务端是创建方）
ServerOperation::ServerOperation(int port, int shmKey, int maxNode)
{
    m_port    = port;
    m_shmKey  = shmKey;
    m_maxNode = maxNode;
    m_shm     = NULL;
}

ServerOperation::~ServerOperation()
{
    if (m_shm != NULL)
    {
        delete m_shm;
        m_shm = NULL;
    }
}

// 生成随机字符串（r2）
void ServerOperation::getRandString(int len, char* randBuf)
{
    if (randBuf == NULL) return;
    srand(time(NULL) + rand());   // 加个扰动，避免和客户端同种子
    for (int i = 0; i < len - 1; i++)
    {
        randBuf[i] = rand() % 94 + 33;
    }
    randBuf[len - 1] = '\0';
}

// 服务器主循环（第 8 天流程）
void ServerOperation::startWork()
{
    // 1. 创建共享内存（服务端负责创建）
    m_shm = new SecKeyShm(m_shmKey, m_maxNode);
    printf("[服务端] 共享内存已创建, key=0x%x, maxNode=%d\n", m_shmKey, m_maxNode);

    // 2. 监听
    if (m_server.setListen(m_port) < 0)
    {
        printf("[服务端] 监听 %d 端口失败!\n", m_port);
        return;
    }
    printf("[服务端] 开始监听端口 %d...\n", m_port);

    // 3. 主循环：accept + 每连接一线程
    while (1)
    {
        // accept 阻塞等待客户端连接
        TcpSocket* sock = m_server.acceptConn();
        if (sock == NULL)
        {
            printf("[服务端] accept 失败\n");
            continue;
        }
        printf("[服务端] 收到新连接\n");

        // 创建子线程处理这个连接
        pthread_t tid;
        pthread_create(&tid, NULL, working, (void*)sock);
        pthread_detach(tid);    // 分离：线程结束自动回收，不用 join
    }
}

// 线程回调：处理一个客户端连接
void* working(void* arg)
{
    // 这个连接的通信对象
    TcpSocket* sock = (TcpSocket*)arg;

    // 1. 接收数据
    char* inData = NULL;
    int   inLen = 0;
    if (sock->recvMsg(&inData, inLen) < 0)
    {
        printf("[服务端] 接收数据失败\n");
        sock->disConnect();
        delete sock;
        return NULL;
    }

    // 2. 解码成 RequestMsg
    CodecFactory* factory = new RequestFactory();   // 解码模式
    Codec* codec = factory->createCodec();
    RequestMsg* pMsg = (RequestMsg*)codec->msgDecode(inData, inLen);
    sock->freeMemory(&inData);

    printf("[服务端] 收到请求: cmdType=%d clientId=%s serverId=%s\n",
           pMsg->cmdType, pMsg->clientId, pMsg->serverId);

    // 3. 按命令类型分发（第 8 天 switch）
    char* outData = NULL;
    int   outLen = 0;
    int   ret = -1;

    switch (pMsg->cmdType)
    {
    case RequestCodec::NewOrUpdate:    // 1 密钥协商
        // 临时：服务端需要知道客户端ID来做校验，这里简化传参
        // （教学版把 clientId/serverId 从 pMsg 拷到静态，避免线程冲突演示）
        ret = 0;   // 占位，实际由 secKeyAgree 处理
        break;
    default:
        printf("[服务端] 未知命令类型 %d\n", pMsg->cmdType);
        break;
    }

    // 4. 发送应答（如果有）
    if (outData != NULL && outLen > 0)
    {
        sock->sendMsg(outData, outLen);
    }

    // 5. 收尾
    sock->disConnect();
    delete sock;
    delete codec;
    delete factory;
    return NULL;
}
