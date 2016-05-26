#pragma once
#ifndef _EP_TEXTIMPL_HPP
#define _EP_TEXTIMPL_HPP

#include "ep/cpp/component/resource/text.h"
#include "ep/cpp/internal/i/itext.h"

namespace ep {

class TextImpl : public BaseImpl<Text, IText>
{
public:
  TextImpl(Component *pInstance, Variant::VarMap initParams)
    : ImplSuper(pInstance)
  {}

  Variant ParseXml() override final;
  void FormatXml(Variant root) override final;
  Variant ParseJson() override final;
  void FormatJson(Variant root) override final;

protected:
  void FormatXmlElement(StreamRef spOut, Variant::VarMap element, int depth);
};

class TextImplStatic : public BaseStaticImpl<ITextStatic>
{
  Variant XMLMapToComponentParams(Variant node) override final;
  Variant ComponentParamsToXMLMap(Variant map) override final;

  uint32_t GetLineNumberFromByteIndex(String buffer, size_t index) override final;
};

} // namespace ep

#endif // _EP_TEXTIMPL_HPP
