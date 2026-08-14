#include <stdio.h>
#include <string.h>
#include "ServerOperation.h"

int main()
{
    int port    = 9898;
    int shmKey  = 0x7788;
    int maxNode = 10;

    const char* dbHost   = "127.0.0.1";
    const char* dbUser   = "root";
    const char* dbPasswd = "123456";   
    const char* dbName   = "seckeydb";

    ServerOperation server(port, shmKey, maxNode,
                           dbHost, dbUser, dbPasswd, dbName);

    // 变成守护进程（后台运行）。如果启动后没有输出，说明成功进入后台
    server.createDaemon();

    // 守护进程化后 stdout 被重定向到 /dev/null，看不到打印了
    server.startWork();

    return 0;
}
