#include "SecKeyShm.h"
#include <string.h>     // memset memcpy strcmp

#include <cstdio>
// ---------- 构造函数 ----------

// 打开：只获取已存在的（不带 maxNode，布局未知，读取时用头部信息）
SecKeyShm::SecKeyShm(int key) : ShareMemory(key)
{
    m_maxNode = 0;
}

// 创建：按 key + 最大节点数
SecKeyShm::SecKeyShm(int key, int maxNode) : ShareMemory(key, maxNode * sizeof(NodeSHMInfo) + sizeof(int)*2)
{
    // 共享内存总大小 = 头部(2个int) + 节点数组(maxNode个)
    m_maxNode = maxNode;

    // 映射并初始化头部（只在"创建"时做，保证首次使用有干净头部）
    void* addr = mapShm();
    if (addr != NULL)
    {
        int* pInt = (int*)addr;       // 头部就是两个 int
        if (pInt[0] == 0)             // 首次创建：total=0 表示还没初始化
        {
            pInt[0] = maxNode;        // total = 最大节点数
            pInt[1] = 0;              // useNum = 已用节点数
        }
    }
}

// 打开（按文件名）
SecKeyShm::SecKeyShm(const char* pathName) : ShareMemory(pathName)
{
    m_maxNode = 0;
}

// 创建（按文件名 + 节点数）
SecKeyShm::SecKeyShm(const char* pathName, int maxNode) : ShareMemory(pathName, maxNode * sizeof(NodeSHMInfo) + sizeof(int)*2)
{
    m_maxNode = maxNode;
    void* addr = mapShm();
    if (addr != NULL)
    {
        int* pInt = (int*)addr;
        if (pInt[0] == 0)
        {
            pInt[0] = maxNode;
            pInt[1] = 0;
        }
    }
}

SecKeyShm::~SecKeyShm()
{
}

// 写一条密钥：找空位（status==0 的节点写，或末尾追加）
int SecKeyShm::shmWrite(NodeSHMInfo* pNodeInfo)
{
    if (pNodeInfo == NULL) return -1;

    void* addr = mapShm();
    if (addr == NULL) return -1;

    // 1. 读头部
    int* pInt = (int*)addr;
    int total = pInt[0];        // 最大节点数
    int useNum = pInt[1];       // 已用数

    // 2. 节点数组的起始位置 = 头部之后
    NodeSHMInfo* pNodes = (NodeSHMInfo*)(pInt + 2);

    // 3. 找空位：遍历找 status==0 的节点
    //    （找"可用且未占用"的位置，复用被注销的节点）
    for (int i = 0; i < total; i++)
    {
        if (pNodes[i].status == 0)    // 空位：status=0 表示未用
        {
            memcpy(&pNodes[i], pNodeInfo, sizeof(NodeSHMInfo));
            pInt[1]++;                // useNum +1
            return 0;
        }
    }
    return -1;   // 满了
}

// 按 客户端ID+服务端ID 查密钥
int SecKeyShm::shmRead(const char* clientID, const char* serverID, NodeSHMInfo* pNodeInfo)
{
    if (clientID == NULL || serverID == NULL || pNodeInfo == NULL) return -1;

    void* addr = mapShm();
    if (addr == NULL) return -1;

    int* pInt = (int*)addr;
    int total = pInt[0];
    NodeSHMInfo* pNodes = (NodeSHMInfo*)(pInt + 2);

    for (int i = 0; i < total; i++)
    {
        if (strcmp(pNodes[i].clientID, clientID) == 0 &&
            strcmp(pNodes[i].serverID, serverID) == 0 &&
            pNodes[i].status != 0)      // 找到匹配且可用的
        {
            memcpy(pNodeInfo, &pNodes[i], sizeof(NodeSHMInfo));
            return 0;
        }
    }
    return -1;   // 没找到
}

// 调试：打印所有节点
void SecKeyShm::printShm()
{
    void* addr = mapShm();
    if (addr == NULL) return;

    int* pInt = (int*)addr;
    int total = pInt[0];
    int useNum = pInt[1];
    printf("total=%d useNum=%d\n", total, useNum);

    NodeSHMInfo* pNodes = (NodeSHMInfo*)(pInt + 2);
    for (int i = 0; i < total; i++)
    {
        printf("node[%d]: status=%d seckeyID=%d clientID=%s serverID=%s seckey=%s\n",
               i, pNodes[i].status, pNodes[i].seckeyID,
               pNodes[i].clientID, pNodes[i].serverID, pNodes[i].seckey);
    }
}
