#ifndef SHAREMEMORY_H
#define SHAREMEMORY_H

#include <sys/ipc.h>      // ftok
#include <sys/shm.h>      // shmget shmat shmdt shmctl
#include <stdlib.h>
// ftok 的 proj_id 参数：用文件名生成 key 时的固定项目ID
const char RandX = 'x';
class ShareMemory
{
public:
    ShareMemory(int key);             // 打开：按 key 获取已存在的共享内存
    ShareMemory(int key, int size);   // 创建：按 key 创建，大小 size
    ShareMemory(const char* name);    // 打开：按文件名(内部ftok生成key)
    ShareMemory(const char* name, int size);  // 创建：按文件名

    virtual ~ShareMemory();

    // 关联共享内存，返回映射地址（第 6 天：shmat）
    void* mapShm();
    // 断开关联（第 6 天：shmdt）
    int unmapShm();
    // 删除共享内存（第 6 天：shmctl IPC_RMID）
    int delShm();

private:
    // 内部：统一封装 shmget（flag 决定创建还是打开）
    int getShmID(key_t key, int shmSize, int flag);

private:
    int  m_shmID;     // 共享内存 ID（shmget 返回值）
    void* m_shmAddr;  // 映射后的地址（shmat 返回值）
};

#endif // SHAREMEMORY_H
