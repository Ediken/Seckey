#ifndef TCPSERVER_H
#define TCPSERVER_H

#include "TcpSocket.h"     // 返回值类型是 TcpSocket*
#include <netinet/in.h>    // sockaddr_in
class TcpServer
{
public:
    TcpServer();
    ~TcpServer();

    // 设置监听：socket + setsockopt + bind + listen（第 5 天服务端流程前4步）
    int setListen(unsigned short port);

    // 每 accept 一次返回一个新 TcpSocket*（封装了 cfd），用完要 delete
    TcpSocket* acceptConn();

    // 关闭监听 fd
    void closefd();

private:
    int m_lfd;                  // 监听文件描述符
    struct sockaddr_in m_addrCli;  // 记录客户端的地址信息
};

#endif // TCPSERVER_H
