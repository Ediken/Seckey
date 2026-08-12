#include "RespondFactory.h"

RespondFactory::RespondFactory()
{
    m_flag = false;
    m_respond = NULL;
}

RespondFactory::RespondFactory(RespondMsg* msg)
{
    m_flag = true;
    m_respond = msg;
}

RespondFactory::~RespondFactory()
{
}

// 生产：根据模式决定产哪种 RespondCodec
Codec* RespondFactory::createCodec()
{
    if (m_flag == true)
    {
        return new RespondCodec(m_respond);   // 编码
    }
    else
    {
        return new RespondCodec();            // 解码
    }
}
