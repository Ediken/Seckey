#include "ServerOperation.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <openssl/hmac.h>     // HMAC 校验 authCode
#include <openssl/sha.h>      // SHA1 生成密钥

#include "RespondCodec.h"
#include "RespondFactory.h"
#include "RequestFactory.h"

// 构造函数：保存配置，共享内存对象先置空（startWork 里再创建）
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
    srand(time(NULL) + rand());   // 加扰动，避免和客户端同种子
    for (int i = 0; i < len - 1; i++)
    {
        randBuf[i] = rand() % 94 + 33;   // 可见 ASCII 字符
    }
    randBuf[len - 1] = '\0';
}

// ------------------------------------------------------------
// 密钥协商处理（第 8 天流程）
// 输入：客户端请求 reqmsg
// 输出：应答字节流 outData/outLen
// 返回：0=成功 -1=失败
// ============================================================
int ServerOperation::secKeyAgree(RequestMsg* reqmsg, char** outData, int& outLen)
{
    // (a) 校验客户端是否合法
    //     教学版简化：固定白名单（clientId="1111" 且 serverId="0001" 合法）
    //     真实项目：查数据库 t_client 表（第 10 天 MysqlOP）
    if (strcmp(reqmsg->clientId, "1111") != 0 ||
        strcmp(reqmsg->serverId, "0001") != 0)
    {
        printf("[服务端] 客户端不合法, 拒绝服务!\n");

        // 组一个 rv=-1 的应答
        RespondMsg rsp;
        memset(&rsp, 0, sizeof(rsp));
        rsp.rv = -1;
        strcpy(rsp.clientId, reqmsg->clientId);
        strcpy(rsp.serverId, reqmsg->serverId);

        CodecFactory* f = new RespondFactory(&rsp);
        Codec* c = f->createCodec();
        c->msgEncode(outData, outLen);
        delete c; delete f;
        return -1;
    }
    printf("[服务端] 客户端合法\n");

    // (b) 验证 authCode（防篡改）
    //     用相同密钥和算法重算 HMAC(r1)，与客户端发来的比对
    unsigned char mac[EVP_MAX_MD_SIZE];
    unsigned int macLen = 0;
    const char* hmacKey = "seckey123";    // 与客户端一致的预共享密钥
    HMAC(EVP_sha256(),
         hmacKey, strlen(hmacKey),
         (const unsigned char*)reqmsg->r1, strlen(reqmsg->r1),
         mac, &macLen);

    // 把二进制 mac 转成十六进制字符串
    char calcAuthCode[65];
    memset(calcAuthCode, 0, sizeof(calcAuthCode));
    for (unsigned int i = 0; i < macLen && i < 32; i++)
    {
        sprintf(calcAuthCode + i * 2, "%02x", mac[i]);
    }

    if (strcmp(calcAuthCode, reqmsg->authCode) != 0)
    {
        printf("[服务端] authCode 校验失败, 拒绝服务!\n");
        printf("  计算值: %s\n  收到值: %s\n", calcAuthCode, reqmsg->authCode);

        RespondMsg rsp;
        memset(&rsp, 0, sizeof(rsp));
        rsp.rv = -1;
        strcpy(rsp.clientId, reqmsg->clientId);
        strcpy(rsp.serverId, reqmsg->serverId);

        CodecFactory* f = new RespondFactory(&rsp);
        Codec* c = f->createCodec();
        c->msgEncode(outData, outLen);
        delete c; delete f;
        return -1;
    }
    printf("[服务端] authCode 校验通过\n");

    // (c) 生成随机字符串 r2
    char r2[64];
    getRandString(64, r2);
    printf("[服务端] 生成 r2=%s\n", r2);

    // (d) 生成密钥 seckey = SHA1(r1 + r2)
    char seckey[128];
    memset(seckey, 0, sizeof(seckey));
    {
        char r1r2[128];
        memset(r1r2, 0, sizeof(r1r2));
        strcpy(r1r2, reqmsg->r1);
        strcat(r1r2, r2);

        unsigned char sha[20];    // SHA1 输出 20 字节
        SHA1((const unsigned char*)r1r2, strlen(r1r2), sha);
        for (int i = 0; i < 20; i++)
        {
            sprintf(seckey + i * 2, "%02x", sha[i]);
        }
    }
    printf("[服务端] 生成密钥 seckey=%s\n", seckey);

    // (e) 分配 seckeyid（教学版：用时间戳取末位，简单生成）
    int seckeyid = (int)(time(NULL) % 100000);

    // (f) 写共享内存
    NodeSHMInfo node;
    memset(&node, 0, sizeof(node));
    node.status   = 1;                    // 1=已使用
    node.seckeyID = seckeyid;
    strcpy(node.clientID, reqmsg->clientId);
    strcpy(node.serverID, reqmsg->serverId);
    strcpy(node.seckey, seckey);
    m_shm->shmWrite(&node);
    printf("[服务端] 密钥已写入共享内存, seckeyid=%d\n", seckeyid);
    m_shm->printShm();

    // (g) 组应答（rv=0 成功, r2 发回客户端, seckeyid 一起发）
    RespondMsg rsp;
    memset(&rsp, 0, sizeof(rsp));
    rsp.rv = 0;
    strcpy(rsp.clientId, reqmsg->clientId);
    strcpy(rsp.serverId, reqmsg->serverId);
    strcpy(rsp.r2, r2);
    rsp.seckeyid = seckeyid;

    CodecFactory* f = new RespondFactory(&rsp);
    Codec* c = f->createCodec();
    c->msgEncode(outData, outLen);       // 编码应答字节流
    delete c; delete f;
    return 0;
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

        // 打包参数，创建子线程处理这个连接
        pthread_t tid;
        ThreadArg* arg = new ThreadArg;   // 堆上分配（线程里用完释放）
        arg->sock   = sock;
        arg->server = this;               // 传 this 指针，线程里能调 secKeyAgree
        pthread_create(&tid, NULL, working, (void*)arg);
        pthread_detach(tid);              // 分离：线程结束自动回收
    }
}

// 线程回调：处理一个客户端连接
void* working(void* arg)
{
    // 1. 解包参数
    ThreadArg* targ = (ThreadArg*)arg;
    TcpSocket* sock = targ->sock;
    ServerOperation* server = targ->server;
    delete targ;                          // 参数用完释放（防泄漏）

    // 2. 接收数据
    char* inData = NULL;
    int   inLen = 0;
    if (sock->recvMsg(&inData, inLen) < 0)
    {
        printf("[服务端] 接收数据失败\n");
        sock->disConnect();
        delete sock;
        return NULL;
    }

    // 3. 解码成 RequestMsg
    CodecFactory* factory = new RequestFactory();   // 解码模式
    Codec* codec = factory->createCodec();
    RequestMsg* pMsg = (RequestMsg*)codec->msgDecode(inData, inLen);
    sock->freeMemory(&inData);

    printf("[服务端] 收到请求: cmdType=%d clientId=%s serverId=%s\n",
           pMsg->cmdType, pMsg->clientId, pMsg->serverId);

    // 4. 按命令类型分发
    char* outData = NULL;
    int   outLen = 0;

    switch (pMsg->cmdType)
    {
    case RequestCodec::NewOrUpdate:    // 1 密钥协商
        server->secKeyAgree(pMsg, &outData, outLen);
        break;
    default:
        printf("[服务端] 未知命令类型 %d\n", pMsg->cmdType);
        break;
    }

    // 5. 发送应答（如果有）
    if (outData != NULL && outLen > 0)
    {
        sock->sendMsg(outData, outLen);
        printf("[服务端] 应答已发送, 长度=%d\n", outLen);
        delete[] outData;     // 释放应答字节流（msgEncode new 的）
    }

    // 6. 收尾
    sock->disConnect();
    delete sock;
    delete codec;
    delete factory;
    return NULL;
}
