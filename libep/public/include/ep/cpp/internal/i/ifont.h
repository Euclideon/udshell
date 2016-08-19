#pragma once
#if !defined(_EP_IFONT_HPP)
#define _EP_IFONT_HPP

#include "ep/cpp/component/component.h"

namespace ep {

SHARED_CLASS(ArrayBuffer);

class IFont
{
public:
  virtual ArrayBufferRef rasterizeText(String text, int fontSize, Float3 fontColor, Float3 outlineColor) const = 0;

  virtual Variant save() const = 0;
};

} //namespace ep

#endif // _EP_IFONT_HPP
