#pragma once
#ifndef _EP_FONTIMPL_HPP
#define _EP_FONTIMPL_HPP

#include "ep/cpp/component/resource/font.h"
#include "ep/cpp/internal/i/ifont.h"

namespace ep {

SHARED_CLASS(Font);
SHARED_CLASS(FreeType);
SHARED_CLASS(Buffer);

class FontImpl : public BaseImpl<Font, IFont>
{
public:
  FontImpl(Component *pInstance, Variant::VarMap initParams);
  ~FontImpl();

  ArrayBufferRef rasterizeText(String text, int fontSize, Float3 fontColor, Float3 outlineColor) const override final;
  Variant save() const override final { return pInstance->Super::save(); }

private:
  FreeTypeRef spFreeType;
  BufferRef spFontData;
  size_t fontIndex;
  void *pFont;
};

} //namespace ep

#endif // _EP_FONTIMPL_HPP
