#include <stdio.h>
#include <string.h>
#include "ServerOperation.h"

int main()
{
    // 服务端配置
    int port    = 9898;
    int shmKey  = 0x7788;
    int maxNode = 10;

    // 数据库配置（与你本机 MySQL 一致）
    const char* dbHost   = "127.0.0.1";
    const char* dbUser   = "root";
    const char* dbPasswd = "123456";   
    const char* dbName   = "seckeydb";

    ServerOperation server(port, shmKey, maxNode,
                           dbHost, dbUser, dbPasswd, dbName);
    server.startWork();
    return 0;
}
