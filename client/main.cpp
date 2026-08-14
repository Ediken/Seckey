// 客户端主程序：菜单循环（第 8 天）
#include <stdio.h>
#include <string.h>

#include "ClientInfo.h"
#include "ClientOperation.h"
#include "RequestCodec.h"   // 用枚举 CmdType

// 打印菜单，返回用户选择
int usage()
{
    int sel = -1;
    printf("\n/*****************************/\n");
    printf("  1. 密钥协商\n");
    printf("  2. 密钥校验\n");
    printf("  3. 密钥注销\n");
    printf("  4. 密钥查看\n");
    printf("  0. 退出\n");
    printf("/*****************************/\n");
    printf("请选择: ");
    scanf("%d", &sel);
    while (getchar() != '\n');   // 清掉输入缓冲里的换行
    return sel;
}

int main()
{
    // 1. 配置客户端信息（教学版写死，真实项目从配置文件读）
    ClientInfo info;
    memset(&info, 0, sizeof(info));
    strcpy(info.clientID, "1111");        // 客户端ID
    strcpy(info.serverID, "0001");        // 服务端ID
    strcpy(info.serverIP, "127.0.0.1");   // 服务端IP（本机测试）
    info.serverPort = 9898;               // 服务端端口
    info.shmKey     = 0x7788;             // 共享内存key（与服务端约定一致）

    // 2. 创建客户端业务对象
    ClientOperation client(&info);

    // 3. 菜单循环
    while (1)
    {
        int sel = usage();
        switch (sel)
        {
        case RequestCodec::NewOrUpdate:    // 1 密钥协商
            client.secKeyAgree();
            break;
        case RequestCodec::Check:          // 2 密钥校验（未实现）
            client.secKeyCheck();
            break;
        case RequestCodec::Revoke:         // 3 密钥注销（未实现）
            client.secKeyRevoke();
            break;
        case RequestCodec::View:           // 4 密钥查看（未实现）
            client.secKeyView();
            break;
        case 0:
            printf("再见!\n");
            return 0;
        default:
            printf("无效选择\n");
            break;
        }
    }
    return 0;
}
