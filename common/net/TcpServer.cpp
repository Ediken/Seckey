#include "TcpServer.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

TcpServer::TcpServer()
{
    m_lfd = -1;
    memset(&m_addrCli, 0, sizeof(m_addrCli));
}

TcpServer::~TcpServer()
{
    closefd();   // 析构时自动关闭监听
}

// 服务端监听准备（第 5 天流程：socket → setsockopt → bind → listen）
int TcpServer::setListen(unsigned short port)
{
    // 1. 创建监听 socket
    m_lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_lfd < 0) return -1;

    // 2. 设置端口复用（关键！解决 TIME_WAIT 导致的重启失败）
    //    服务器断开后端口会处于 TIME_WAIT，不设复用，立刻重启会 bind 失败
    int opt = 1;
    setsockopt(m_lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 3. 绑定地址
    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family      = AF_INET;
    servAddr.sin_port        = htons(port);       // 端口
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY); // 绑定本地任意可用IP
    if (bind(m_lfd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
        return -1;

    // 4. 开始监听（backlog=128：等待连接队列长度）
    if (listen(m_lfd, 128) < 0)
        return -1;

    return 0;
}

// 接受连接：每来一个客户端，返回一个 TcpSocket*（封装通信 fd）
TcpSocket* TcpServer::acceptConn()
{
    socklen_t len = sizeof(m_addrCli);

    // accept 阻塞等待，有客户端连接才返回
    int cfd = accept(m_lfd, (struct sockaddr*)&m_addrCli, &len);
    if (cfd < 0) return NULL;

    // 把通信 fd 包装成 TcpSocket 对象返回（第 5 天改进版设计）
    return new TcpSocket(cfd);
}

// 关闭监听 fd
void TcpServer::closefd()
{
    if (m_lfd >= 0)
    {
        close(m_lfd);
        m_lfd = -1;
    }
}
