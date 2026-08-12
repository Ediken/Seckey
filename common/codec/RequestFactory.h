#ifndef REQUESTFACTORY_H
#define REQUESTFACTORY_H

#include "CodecFactory.h"   // 继承抽象工厂
#include "RequestCodec.h"   // 生产的产品类型

// ============================================================
// RequestFactory —— 请求报文生产线
// 生产 RequestCodec，返回 Codec*（父类指针，多态）
// ============================================================
class RequestFactory : public CodecFactory
{
public:
    RequestFactory();                 // 解码模式：不带数据
    RequestFactory(RequestMsg* msg);  // 编码模式：带要编码的数据
    ~RequestFactory();

    // 实现父类纯虚函数
    Codec* createCodec();

private:
    bool        m_flag;    // true=编码模式  false=解码模式
    RequestMsg* m_request; // 编码时用的数据（解码模式不用）
};

#endif // REQUESTFACTORY_H
