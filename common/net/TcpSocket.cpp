#include "TcpSocket.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>          // close
#include <arpa/inet.h>       // htonl ntohl sockaddr_in
#include <sys/socket.h>      // socket connect send recv
#include <netinet/in.h>

// 客户端身份：还没 socket，等 connectToHost 时再创建
TcpSocket::TcpSocket()
{
    m_socket = -1;
}

// 服务端身份：包装 accept 出来的通信 fd
TcpSocket::TcpSocket(int connfd)
{
    m_socket = connfd;
}

TcpSocket::~TcpSocket()
{
    disConnect();   // 析构时自动断开，防止忘关
}

int TcpSocket::connectToHost(char* ip, unsigned short port)
{
    // 1. 创建 socket（AF_INET=IPv4, SOCK_STREAM=TCP, 0=自动选协议）
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket < 0) return -1;

    // 2. 准备服务器地址结构
    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;                    // IPv4
    servAddr.sin_port   = htons(port);                // 端口：主机序→网络序
    // inet_pton：把 "192.168.1.1" 这种点分十进制转成二进制
    if (inet_pton(AF_INET, ip, &servAddr.sin_addr) <= 0)
        return -1;

    // 3. 连接服务器
    if (connect(m_socket, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
        return -1;

    return 0;
}

// 发送数据：先发 4 字节长度头，再发数据体（粘包解决方案）
int TcpSocket::sendMsg(char* sendData, int dataLen)
{
    if (sendData == NULL || dataLen <= 0) return -1;

    // 1. 长度头：把数据长度转成网络字节序(大端)，存进 4 字节
    int netLen = htonl(dataLen);

    // 2. 先发长度头（4字节）
    int ret = writen(&netLen, 4);
    if (ret != 4) return -1;

    // 3. 再发数据体
    ret = writen(sendData, dataLen);
    if (ret != dataLen) return -1;

    return 0;
}

// 接收数据：先收 4 字节长度头，再收数据体
int TcpSocket::recvMsg(char** recvData, int& recvLen)
{
    if (recvData == NULL) return -1;

    // 1. 先收 4 字节长度头
    int netLen = 0;
    int ret = readn(&netLen, 4);
    if (ret != 4) return -1;

    // 2. 网络字节序 → 主机字节序，得到数据长度
    int dataLen = ntohl(netLen);
    if (dataLen <= 0 || dataLen > 1024*1024) return -1;  // 防御：长度异常直接拒

    // 3. 分配内存，收数据体
    char* data = (char*)malloc(dataLen);
    if (data == NULL) return -1;

    ret = readn(data, dataLen);
    if (ret != dataLen) {          // 没收满：释放内存，返回失败
        free(data);
        return -1;
    }

    *recvData = data;              // 数据交给调用者
    recvLen   = dataLen;
    return 0;
}

// 断开连接
void TcpSocket::disConnect()
{
    if (m_socket >= 0)
    {
        close(m_socket);
        m_socket = -1;
    }
}

// 释放 recvMsg 分配的内存
void TcpSocket::freeMemory(char** buf)
{
    if (buf != NULL && *buf != NULL)
    {
        free(*buf);
        *buf = NULL;   // 置空，防野指针
    }
}


// 读满 count 字节（解决半包：read 可能一次读不够，循环读）
int TcpSocket::readn(void* buf, int count)
{
    char* p = (char*)buf;
    int total = 0;                // 已读总数
    while (total < count)
    {
        int n = read(m_socket, p + total, count - total);
        if (n > 0)
        {
            total += n;
        }
        else if (n == 0)
        {
            return -1;            // 对方关闭连接
        }
        else
        {
            return -1;            // 读错误
        }
    }
    return total;
}

// 写满 count 字节（同理，循环写）
int TcpSocket::writen(const void* buf, int count)
{
    const char* p = (const char*)buf;
    int total = 0;
    while (total < count)
    {
        int n = write(m_socket, p + total, count - total);
        if (n > 0)
        {
            total += n;
        }
        else
        {
            return -1;
        }
    }
    return total;
}
