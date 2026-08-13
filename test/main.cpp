// ============================================================
// 测试程序：编解码 + 网络通信
// 目标：
//   1. 编解码往返（RequestMsg/RespondMsg 编码→解码）
//   2. 网络收发（客户端 sendMsg / 服务端 recvMsg，长度头防粘包）
// ============================================================
#include <iostream>
#include <string.h>
#include <unistd.h>          // usleep
#include <pthread.h>         // 线程

#include "RequestCodec.h"
#include "RespondCodec.h"
#include "CodecFactory.h"
#include "RequestFactory.h"
#include "RespondFactory.h"
#include "TcpServer.h"
#include "TcpSocket.h"

using namespace std;

// ---------- 网络测试需要的全局变量（线程间传递结果用）----------
static int g_netResult = -1;   // 服务端线程的测试结果：0=通过

// ---------- 服务端线程函数：监听、accept、收数据、回数据 ----------
void* serverThread(void* arg)
{
    // 1. 创建服务端，监听 9898 端口
    TcpServer server;
    if (server.setListen(9898) < 0)
    {
        cout << "[服务端] setListen 失败" << endl;
        g_netResult = -1;
        return NULL;
    }
    cout << "[服务端] 监听 9898 端口..." << endl;

    // 2. accept 等待客户端连接（阻塞）
    TcpSocket* sock = server.acceptConn();
    if (sock == NULL)
    {
        cout << "[服务端] accept 失败" << endl;
        g_netResult = -1;
        return NULL;
    }
    cout << "[服务端] 客户端已连接" << endl;

    // 3. 连续收 2 条消息（验证长度头能正确切分粘包）
    for (int i = 0; i < 2; i++)
    {
        char* recvData = NULL;
        int   recvLen = 0;
        if (sock->recvMsg(&recvData, recvLen) < 0)
        {
            cout << "[服务端] 第" << i+1 << "条 recvMsg 失败" << endl;
            g_netResult = -1;
            sock->disConnect();
            delete sock;
            return NULL;
        }
        cout << "[服务端] 收到第" << i+1 << "条: " << recvData
             << " (长度" << recvLen << ")" << endl;
        sock->freeMemory(&recvData);    // 释放收的数据
    }

    // 4. 回一条消息给客户端
    char reply[] = "hello client, I am server";
    sock->sendMsg(reply, strlen(reply));
    cout << "[服务端] 已回复: " << reply << endl;

    // 5. 收尾
    sock->disConnect();
    delete sock;
    server.closefd();
    g_netResult = 0;     // 全部成功
    return NULL;
}

// ---------- 测试1：请求报文的编码/解码往返 ----------
int testRequest()
{
    cout << "===== 测试1: 请求报文 编码->解码 往返 =====" << endl;

    // 1. 构造一份请求数据（模拟客户端要发的请求）
    RequestMsg msg;
    memset(&msg, 0, sizeof(msg));          // 先清空
    msg.cmdType = RequestCodec::NewOrUpdate;  // =1 密钥协商
    strcpy(msg.clientId, "1111");          // 客户端ID
    strcpy(msg.authCode, "abcdef0123456789"); // 消息认证码(测试用随便写)
    strcpy(msg.serverId, "0001");          // 服务端ID
    strcpy(msg.r1, "hello r1 random");     // 随机串

    // 2. 编码：工厂(带数据=编码模式) -> Codec* -> msgEncode
    char* outData = NULL;
    int   outLen = 0;
    CodecFactory* factory = new RequestFactory(&msg);  // 带数据 → 编码模式
    Codec* codec = factory->createCodec();             // 多态：实际是 RequestCodec
    codec->msgEncode(&outData, outLen);                // 结构体 -> 字节流

    cout << "编码成功: 字节流长度 = " << outLen << " 字节" << endl;
    cout << "字节流内容(十六进制): ";
    for (int i = 0; i < outLen; i++) {
        printf("%02x ", (unsigned char)outData[i]);    // 逐字节打印，看TLV结构
    }
    cout << endl;

    // 3. 解码：工厂(不带数据=解码模式) -> Codec* -> msgDecode
    CodecFactory* factory2 = new RequestFactory();     // 不带数据 → 解码模式
    Codec* codec2 = factory2->createCodec();
    RequestMsg* pMsg = (RequestMsg*)codec2->msgDecode(outData, outLen);  // 字节流 -> 结构体

    // 4. 验证解码结果和原数据是否一致
    cout << "解码结果: " << endl;
    cout << "  cmdType  = " << pMsg->cmdType << endl;
    cout << "  clientId = " << pMsg->clientId << endl;
    cout << "  authCode = " << pMsg->authCode << endl;
    cout << "  serverId = " << pMsg->serverId << endl;
    cout << "  r1       = " << pMsg->r1 << endl;

    // 5. 比对
    if (pMsg->cmdType == msg.cmdType &&
        strcmp(pMsg->clientId, msg.clientId) == 0 &&
        strcmp(pMsg->authCode, msg.authCode) == 0 &&
        strcmp(pMsg->serverId, msg.serverId) == 0 &&
        strcmp(pMsg->r1, msg.r1) == 0) {
        cout << ">>> 测试1通过: 解码数据与编码前完全一致!" << endl;
    } else {
        cout << ">>> 测试1失败: 数据不一致!" << endl;
    }

    delete[] outData;      // 释放编码产生的字节流
    delete codec;
    delete codec2;
    delete factory;
    delete factory2;
    return 0;
}

// ---------- 测试2：应答报文的编码/解码往返 ----------
int testRespond()
{
    cout << endl << "===== 测试2: 应答报文 编码->解码 往返 =====" << endl;

    RespondMsg msg;
    memset(&msg, 0, sizeof(msg));
    msg.rv = 0;                    // 成功
    strcpy(msg.clientId, "1111");
    strcpy(msg.serverId, "0001");
    strcpy(msg.r2, "world r2 random");
    msg.seckeyid = 88;             // 密钥编号

    // 编码
    char* outData = NULL;
    int   outLen = 0;
    CodecFactory* factory = new RespondFactory(&msg);  // 带数据 → 编码模式
    Codec* codec = factory->createCodec();
    codec->msgEncode(&outData, outLen);

    cout << "编码成功: 字节流长度 = " << outLen << " 字节" << endl;
    cout << "字节流内容(十六进制): ";
    for (int i = 0; i < outLen; i++) {
        printf("%02x ", (unsigned char)outData[i]);
    }
    cout << endl;

    // 解码
    CodecFactory* factory2 = new RespondFactory();     // 不带数据 → 解码模式
    Codec* codec2 = factory2->createCodec();
    RespondMsg* pMsg = (RespondMsg*)codec2->msgDecode(outData, outLen);

    cout << "解码结果: " << endl;
    cout << "  rv       = " << pMsg->rv << endl;
    cout << "  clientId = " << pMsg->clientId << endl;
    cout << "  serverId = " << pMsg->serverId << endl;
    cout << "  r2       = " << pMsg->r2 << endl;
    cout << "  seckeyid = " << pMsg->seckeyid << endl;

    if (pMsg->rv == msg.rv &&
        pMsg->seckeyid == msg.seckeyid &&
        strcmp(pMsg->clientId, msg.clientId) == 0 &&
        strcmp(pMsg->serverId, msg.serverId) == 0 &&
        strcmp(pMsg->r2, msg.r2) == 0) {
        cout << ">>> 测试2通过: 解码数据与编码前完全一致!" << endl;
    } else {
        cout << ">>> 测试2失败: 数据不一致!" << endl;
    }

    delete[] outData;
    delete codec;
    delete codec2;
    delete factory;
    delete factory2;
    return 0;
}

// ---------- 网络测试主函数：起服务端线程 + 客户端 ----------
int testNet()
{
    cout << endl << "===== 测试3: 网络收发(服务端线程+客户端) =====" << endl;

    // 1. 启动服务端线程
    pthread_t tid;
    pthread_create(&tid, NULL, serverThread, NULL);

    // 2. 主线程当客户端：等服务端先监听（睡一会儿，避免连不上）
    usleep(500 * 1000);    // 500ms

    // 3. 客户端连接
    TcpSocket client;
    if (client.connectToHost("127.0.0.1", 9898) < 0)
    {
        cout << "[客户端] 连接失败" << endl;
        return -1;
    }
    cout << "[客户端] 已连接服务器" << endl;

    // 4. 连续发 2 条消息（故意两条不同长度，验证长度头切分）
    char msg1[] = "hello server, first msg";
    char msg2[] = "second";
    client.sendMsg(msg1, strlen(msg1));
    client.sendMsg(msg2, strlen(msg2));
    cout << "[客户端] 已发送2条消息" << endl;

    // 5. 收服务端回复
    char* recvData = NULL;
    int   recvLen = 0;
    if (client.recvMsg(&recvData, recvLen) < 0)
    {
        cout << "[客户端] 收回复失败" << endl;
        return -1;
    }
    cout << "[客户端] 收到回复: " << recvData << " (长度" << recvLen << ")" << endl;
    client.freeMemory(&recvData);

    // 6. 断开
    client.disConnect();

    // 7. 等服务端线程结束，检查结果
    pthread_join(tid, NULL);
    if (g_netResult == 0)
        cout << ">>> 测试3通过: 网络收发成功, 长度头切分正确!" << endl;
    else
        cout << ">>> 测试3失败" << endl;
    return 0;
}

// ---------- 主函数 ----------
int main()
{
    testRequest();     // 测试1：请求报文编解码往返
    testRespond();     // 测试2：应答报文编解码往返
    testNet();         // 测试3：网络收发
    return 0;
}
