// 服务端主程序
#include <stdio.h>
#include <string.h>
#include "ServerOperation.h"

int main()
{
    // 服务端配置（教学版写死，与客户端约定一致）
    int port    = 9898;      // 监听端口（客户端连这个）
    int shmKey  = 0x7788;    // 共享内存 key（与客户端一致）
    int maxNode = 10;        // 共享内存最大节点数

    ServerOperation server(port, shmKey, maxNode);
    server.startWork();      // 开始监听（阻塞）

    return 0;
}
