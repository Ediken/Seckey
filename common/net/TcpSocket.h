#ifndef TCPSOCKET_H
#define TCPSOCKET_H

// ============================================================
// TcpSocket —— TCP 通信类（客户端、服务端【共用】）
//
// 它解决两个核心问题：
//   1. 封装 socket API（连接、发送、接收、断开）
//   2. 解决 TCP 粘包/半包问题（关键！）
//
// 【粘包问题】TCP 是字节流，没有"消息边界"：
//   发送方 send("AB") send("CD") → 接收方可能一次收到 "ABCD"
//   解决：约定"先发 4 字节长度头，再发数据体"
//     发送：[4字节长度][数据]
//     接收：先读4字节长度 → 再读那么多字节的数据
//
// 【半包问题】一次 send 的数据可能分多次到达：
//   解决：readn() 循环读，直到读满 count 字节
//
// 两个构造函数对应两种身份：
//   TcpSocket()          ：客户端用（自己 socket + connect）
//   TcpSocket(int connfd)：服务端用（包装 accept 出来的 fd）
// ============================================================

class TcpSocket
{
public:
    TcpSocket();                 // 客户端身份
    TcpSocket(int connfd);       // 服务端身份：包装已有 fd
    ~TcpSocket();

    // 连接服务器（客户端用）
    // ip: 服务器IP    port: 服务器端口
    // 返回 0 成功 -1 失败
    int connectToHost(char* ip, unsigned short port);

    // 发送数据：自动加 4 字节长度头
    int sendMsg(char* sendData, int dataLen);

    // 接收数据：自动解析长度头，分配内存装数据
    // recvData: 输出，收到数据的地址(new 的，用完调 freeMemory 释放)
    // recvLen:  输出，数据长度
    int recvMsg(char** recvData, int& recvLen);

    // 断开连接
    void disConnect();

    // 释放 recvMsg 分配的内存
    void freeMemory(char** buf);

private:
    // 读满 count 字节（解决半包：循环读）
    int readn(void* buf, int count);
    // 写满 count 字节（解决半包：循环写）
    int writen(const void* buf, int count);

private:
    int m_socket;   // 通信用的 socket 文件描述符
};

#endif // TCPSOCKET_H
