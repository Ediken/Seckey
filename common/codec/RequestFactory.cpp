#include "RequestFactory.h"

// 解码模式：不带数据
RequestFactory::RequestFactory()
{
    m_flag = false;
    m_request = NULL;
}

// 编码模式：记住要编码的数据
RequestFactory::RequestFactory(RequestMsg* msg)
{
    m_flag = true;
    m_request = msg;
}

RequestFactory::~RequestFactory()
{
}

// 生产：根据模式决定产哪种 RequestCodec
Codec* RequestFactory::createCodec()
{
    if (m_flag == true)
    {
        return new RequestCodec(m_request);   // 编码：把数据带进去
    }
    else
    {
        return new RequestCodec();            // 解码：空对象
    }
}
