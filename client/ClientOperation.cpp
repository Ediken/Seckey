#include "ClientOperation.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <openssl/hmac.h>     // HMAC 生成 authCode（第 7 天）
#include <openssl/sha.h>      // SHA1 生成密钥

#include "RequestCodec.h"
#include "RespondCodec.h"
#include "RequestFactory.h"
#include "RespondFactory.h"

// 构造函数：保存配置，创建共享内存对象（打开已存在的）
ClientOperation::ClientOperation(ClientInfo* info)
{
    memcpy(&m_info, info, sizeof(ClientInfo));

    // 客户端打开共享内存（key 已知，不创建——服务端已建）
    // 注意：教学版简化，客户端这里【创建】自己的共享内存（maxNode=1）
    // 真实项目里由服务端创建、客户端只打开；我们后续联调时统一约定
    m_shm = new SecKeyShm(m_info.shmKey, 1);
}

ClientOperation::~ClientOperation()
{
    if (m_shm != NULL)
    {
        delete m_shm;
        m_shm = NULL;
    }
}

// ------------------------------------------------------------
// 生成随机字符串（用于 r1）
// 简单实现：用 rand() 生成可见字符
// ------------------------------------------------------------
void ClientOperation::getRandString(int len, char* randBuf)
{
    if (randBuf == NULL) return;

    srand(time(NULL));       // 用时间做随机种子（每次运行不同）
    for (int i = 0; i < len - 1; i++)
    {
        // 生成 33~126 的可见 ASCII 字符
        randBuf[i] = rand() % 94 + 33;
    }
    randBuf[len - 1] = '\0';
}

// ------------------------------------------------------------
// 密钥协商：第 7 天 10 步流程完整落地
// ============================================================
int ClientOperation::secKeyAgree()
{
    // ---------- 第 1 步：构造请求结构体 ----------
    RequestMsg msg;
    memset(&msg, 0, sizeof(msg));
    msg.cmdType = RequestCodec::NewOrUpdate;   // =1 密钥协商
    strcpy(msg.clientId, m_info.clientID);
    strcpy(msg.serverId, m_info.serverID);

    getRandString(64, msg.r1);                 // 生成随机串 r1

    // 生成 authCode = HMAC(r1)
    // HMAC(算法, 密钥, 密钥长度, 数据, 数据长度, 输出, 输出长度)
    // 注意：教学版用固定密钥 "seckey123"，真实项目应使用预共享密钥
    unsigned char mac[EVP_MAX_MD_SIZE];
    unsigned int macLen = 0;
    const char* hmacKey = "seckey123";         // 预共享密钥（教学简化）
    HMAC(EVP_sha256(),
         hmacKey, strlen(hmacKey),
         (const unsigned char*)msg.r1, strlen(msg.r1),
         mac, &macLen);
    // 把二进制 mac 转成十六进制字符串存进 authCode
    for (unsigned int i = 0; i < macLen && i < 64; i++)
    {
        sprintf(msg.authCode + i * 2, "%02x", mac[i]);
    }
    printf("[客户端] r1=%s\n", msg.r1);
    printf("[客户端] authCode=%s\n", msg.authCode);

    // ---------- 第 2 步：编码请求 ----------
    char* outData = NULL;
    int   outLen = 0;
    CodecFactory* factory = new RequestFactory(&msg);   // 编码模式
    Codec* codec = factory->createCodec();
    codec->msgEncode(&outData, outLen);
    printf("[客户端] 编码成功, 报文长度=%d\n", outLen);

    // ---------- 第 3 步：连接服务器 ----------
    if (m_socket.connectToHost(m_info.serverIP, m_info.serverPort) < 0)
    {
        printf("[客户端] 连接服务器失败\n");
        delete codec; delete factory;
        return -1;
    }
    printf("[客户端] 已连接服务器 %s:%d\n", m_info.serverIP, m_info.serverPort);

    // ---------- 第 4 步：发送请求 ----------
    m_socket.sendMsg(outData, outLen);
    printf("[客户端] 请求已发送\n");

    // ---------- 第 5 步：接收应答 ----------
    char* inData = NULL;
    int   inLen = 0;
    if (m_socket.recvMsg(&inData, inLen) < 0)
    {
        printf("[客户端] 接收应答失败\n");
        m_socket.disConnect();
        delete codec; delete factory;
        return -1;
    }
    printf("[客户端] 收到应答, 长度=%d\n", inLen);

    // ---------- 第 6 步：解码应答 ----------
    CodecFactory* factory2 = new RespondFactory();      // 解码模式
    Codec* codec2 = factory2->createCodec();
    RespondMsg* pRsp = (RespondMsg*)codec2->msgDecode(inData, inLen);
    m_socket.freeMemory(&inData);

    printf("[客户端] 应答: rv=%d seckeyid=%d r2=%s\n",
           pRsp->rv, pRsp->seckeyid, pRsp->r2);

    // ---------- 第 7 步：判断 rv ----------
    if (pRsp->rv == -1)
    {
        printf("[客户端] 密钥协商失败!\n");
        m_socket.disConnect();
        delete codec; delete codec2; delete factory; delete factory2;
        return -1;
    }

    // ---------- 第 8 步：生成密钥 seckey = SHA1(r1 + r2) ----------
    char seckey[128];
    memset(seckey, 0, sizeof(seckey));
    {
        // 拼接 r1 + r2
        char r1r2[128];
        memset(r1r2, 0, sizeof(r1r2));
        strcpy(r1r2, msg.r1);
        strcat(r1r2, pRsp->r2);

        // SHA1 哈希
        unsigned char sha[20];    // SHA1 输出 20 字节
        SHA1((const unsigned char*)r1r2, strlen(r1r2), sha);

        // 转十六进制字符串（40 字符）
        for (int i = 0; i < 20; i++)
        {
            sprintf(seckey + i * 2, "%02x", sha[i]);
        }
    }
    printf("[客户端] 生成密钥 seckey=%s\n", seckey);

    // ---------- 第 9 步：写共享内存 ----------
    NodeSHMInfo node;
    memset(&node, 0, sizeof(node));
    node.status   = 1;                    // 1=已使用
    node.seckeyID = pRsp->seckeyid;       // 服务端分配的编号
    strcpy(node.clientID, m_info.clientID);
    strcpy(node.serverID, m_info.serverID);
    strcpy(node.seckey, seckey);          // 协商出的密钥

    if (m_shm->shmWrite(&node) == 0)
    {
        printf("[客户端] 密钥已写入共享内存!\n");
    }
    else
    {
        printf("[客户端] 写共享内存失败!\n");
    }
    m_shm->printShm();    // 打印查看

    // ---------- 第 10 步：断开连接 ----------
    m_socket.disConnect();
    printf("[客户端] 已断开连接\n");

    delete codec; delete codec2;
    delete factory; delete factory2;
    return 0;
}
