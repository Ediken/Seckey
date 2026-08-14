#ifndef CLIENTINFO_H
#define CLIENTINFO_H

// ============================================================
// ClientInfo —— 客户端的配置信息
// 相当于"客户端的档案"，ClientOperation 用它知道：
//   我是谁(clientID)、我要连谁(serverID/IP/端口)、密钥写哪(shmKey)
// ============================================================
struct ClientInfo
{
    char clientID[12];          // 客户端编号（本机唯一标识）
    char serverID[12];          // 服务端编号（我要跟谁协商）
    char serverIP[32];          // 服务端 IP 地址
    unsigned short serverPort;  // 服务端端口
    int  shmKey;                // 共享内存 key（协商完密钥写这块内存）
};

#endif // CLIENTINFO_H
