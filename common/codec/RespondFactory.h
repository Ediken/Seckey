#ifndef RESPONDFACTORY_H
#define RESPONDFACTORY_H

#include "CodecFactory.h"
#include "RespondCodec.h"

// ============================================================
// RespondFactory —— 应答报文生产线
// 生产 RespondCodec，返回 Codec*（父类指针，多态）
// ============================================================
class RespondFactory : public CodecFactory
{
public:
    RespondFactory();                   // 解码模式
    RespondFactory(RespondMsg* msg);    // 编码模式
    ~RespondFactory();

    // 实现父类纯虚函数
    Codec* createCodec();

private:
    bool        m_flag;     // true=编码模式  false=解码模式
    RespondMsg* m_respond;  // 编码时用的数据
};

#endif // RESPONDFACTORY_H
