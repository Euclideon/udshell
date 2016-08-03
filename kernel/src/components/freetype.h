#pragma once
#ifndef EP_FREETYPE_H
#define EP_FREETYPE_H

#include "ep/cpp/component/component.h"
#include "ep/cpp/component/resource/arraybuffer.h"

#if defined CreateFont
# undef CreateFont
#endif

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H

namespace ep {

SHARED_CLASS(FreeType);

class FreeType : public Component
{
  EP_DECLARE_COMPONENT(ep, FreeType, Component, EPKERNEL_PLUGINVERSION, "FreeType integration", 0)
public:

  void *createFont(Slice<const void> buffer, size_t faceIndex);
  void destroyFont(void *pFont);
  size_t getNumFaces(void *pFont) const;
  Variant::VarMap::MapType getMetadata(void *pFont) const;
  ArrayBufferRef rasterizeText(void *pFont, String text, int fontSize, Float3 fontColor, Float3 outlineColor) const;

protected:
  FreeType(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
  ~FreeType();

private:
  FT_Library m_library;
};

} //namespace ep

#endif // EP_FREETYPE_H
