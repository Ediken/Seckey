#ifndef CODECFACTORY_H
#define CODECFACTORY_H

#include "Codec.h"      // 工厂生产的"产品"是 Codec

// ============================================================
// CodecFactory —— 抽象工厂（工厂模式的父类）
//
// 作用：规定"凡是工厂，必须能生产 Codec"。
// 本类不生产任何具体对象（createCodec 返回 NULL 是占位），
// 具体生产逻辑在子类 RequestFactory / RespondFactory 里。
//
// 为什么要工厂：
//   调用方只需持有 CodecFactory*（父类指针），调 createCodec()
//   就能拿到编解码器，不用自己 new 具体类 —— 面向接口编程。
// ============================================================
class CodecFactory
{
public:
    CodecFactory();
    virtual ~CodecFactory();          // 虚析构

    // 生产编解码器（纯虚函数 = 抽象类，子类必须实现）
    virtual Codec* createCodec() = 0;
};

#endif // CODECFACTORY_H
