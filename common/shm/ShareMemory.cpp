#include "ShareMemory.h"
#include <string.h>     // memset

// ---------- 四个构造函数：都是"取得 shmID" ----------

// 打开：按 key，不创建（size=0, flag=0：只获取）
ShareMemory::ShareMemory(int key)
{
    m_shmAddr = NULL;
    m_shmID = getShmID(key, 0, 0);
}

// 创建：按 key，大小 size（IPC_CREAT：不存在则建，存在则获取）
ShareMemory::ShareMemory(int key, int size)
{
    m_shmAddr = NULL;
    m_shmID = getShmID(key, size, IPC_CREAT | 0777);
}

// 打开：按文件名（内部 ftok 生成 key）
ShareMemory::ShareMemory(const char* name)
{
    m_shmAddr = NULL;
    // ftok: 用"文件路径 + 项目ID"生成唯一的 key（第 6 天）
    key_t key = ftok(name, RandX);
    m_shmID = getShmID(key, 0, 0);
}

// 创建：按文件名 + 大小
ShareMemory::ShareMemory(const char* name, int size)
{
    m_shmAddr = NULL;
    key_t key = ftok(name, RandX);
    m_shmID = getShmID(key, size, IPC_CREAT | 0777);
}

ShareMemory::~ShareMemory()
{
    // 析构时自动断开映射（但【不】删共享内存——删除要显式调 delShm）
    if (m_shmAddr != NULL)
    {
        unmapShm();
    }
}

// 内部：封装 shmget
int ShareMemory::getShmID(key_t key, int shmSize, int flag)
{
    // shmget(key, size, flag)
    //   key  : 唯一标识（第 6 天）
    //   size : 大小（打开时传 0）
    //   flag : IPC_CREAT=创建 | 权限
    return shmget(key, shmSize, flag);
}

// 关联：把共享内存映射到进程地址空间，返回地址
void* ShareMemory::mapShm()
{
    if (m_shmAddr != NULL) return m_shmAddr;   // 已映射过，直接返回
    // shmat(shmid, NULL, 0)：NULL=让内核选地址，0=可读可写
    m_shmAddr = shmat(m_shmID, NULL, 0);
    if (m_shmAddr == (void*)-1)                // 失败返回 (void*)-1
    {
        m_shmAddr = NULL;
        return NULL;
    }
    return m_shmAddr;
}

// 断开：解除映射（shmdt）
int ShareMemory::unmapShm()
{
    if (m_shmAddr != NULL)
    {
        int ret = shmdt(m_shmAddr);
        m_shmAddr = NULL;
        return ret;
    }
    return 0;
}

// 删除：标记删除（IPC_RMID）
// 注意：关联计数>0 时不会立即删除，但 key 会失效（第 6 天测试结论）
int ShareMemory::delShm()
{
    return shmctl(m_shmID, IPC_RMID, NULL);
}
