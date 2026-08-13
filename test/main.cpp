// ============================================================
// 编解码模块测试程序
// 目标：验证"RequestMsg → 编码 → 字节流 → 解码 → RequestMsg"
//       以及"RespondMsg → 编码 → 字节流 → 解码 → RespondMsg"
// 全链路跑通，亲眼看到多态 + 工厂在工作
// ============================================================
#include <iostream>
#include <string.h>

#include "RequestCodec.h"
#include "RespondCodec.h"
#include "CodecFactory.h"
#include "RequestFactory.h"
#include "RespondFactory.h"

using namespace std;

// 测试1：请求报文的编码/解码往返
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

// 测试2：应答报文的编码/解码往返（结构与测试1对称）
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

int main()
{
    testRequest();
    testRespond();
    return 0;
}
