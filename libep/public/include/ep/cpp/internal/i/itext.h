#pragma once
#ifndef _EP_ITEXT_HPP
#define _EP_ITEXT_HPP

#include "ep/cpp/variant.h"

namespace ep {

SHARED_CLASS(Text);

class IText
{
public:
  virtual Variant ParseXml() = 0;
  virtual void FormatXml(Variant root) = 0;
  virtual Variant ParseJson() = 0;
  virtual void FormatJson(Variant root) = 0;
};

class ITextStatic
{
public:
  virtual Variant XMLMapToComponentParams(Variant node) = 0;
  virtual Variant ComponentParamsToXMLMap(Variant map) = 0;

  virtual uint32_t GetLineNumberFromByteIndex(String buffer, size_t index) = 0;
};

} // namespace ep

#endif // _EP_ITEXT_HPP
