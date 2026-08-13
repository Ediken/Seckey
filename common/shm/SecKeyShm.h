#ifndef SECKEYSHM_H
#define SECKEYSHM_H

#include "ShareMemory.h"    // 继承基础类

// ============================================================
// NodeSHMInfo —— 共享内存里"一条密钥记录"的节点（第 7 天）
// ============================================================
struct NodeSHMInfo
{
    int  status;        // 状态：0=可用 1=注销
    int  seckeyID;      // 密钥编号
    char clientID[12];  // 客户端ID
    char serverID[12];  // 服务端ID
    char seckey[128];   // 密钥本体（hash(r1+r2) 的结果）
};

class SecKeyShm : public ShareMemory
{
public:
    SecKeyShm(int key);                  // 打开
    SecKeyShm(int key, int maxNode);     // 创建（指定最大节点数）
    SecKeyShm(const char* pathName);     // 打开
    SecKeyShm(const char* pathName, int maxNode);  // 创建
    ~SecKeyShm();

    // 写一条密钥到共享内存（找空位写，自动更新头部计数）
    int shmWrite(NodeSHMInfo* pNodeInfo);
    // 按 客户端ID+服务端ID 查密钥记录
    int shmRead(const char* clientID, const char* serverID, NodeSHMInfo* pNodeInfo);

    // 调试用：打印共享内存全部节点
    void printShm();

private:
    int m_maxNode;   // 最大节点数（头部 + 节点数组的布局依据）
};

#endif // SECKEYSHM_H
